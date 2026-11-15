/*
Copyright (c) FufuLauncher Dev Team. All rights reserved.
Licensed under the AGPL-3.0 License.
Adapted from FufuLauncher.UnlockerIsland/DomainMap/DomainMap.cpp (7.2).
*/
#include "DomainMap.h"
#include "Scanner.h"
#include "MinHook.h"
#include <Windows.h>
#include <wincrypt.h>
#include <intrin.h>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#pragma comment(lib, "Advapi32.lib")

namespace {
// Upstream 7.2 CN signatures and object layout. Do not apply to another EXE.
constexpr wchar_t kCn72Sha256[] =
    L"8B014F64B5DC12FBC476E0C362B053ABFDD7A1378CD282E4AD81057621DBD6B5";
constexpr char kSourceEntry[] =
    "41 56 56 57 55 53 48 83 EC 20 41 89 D6 48 89 CF "
    "80 3D ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? 00 0F 85 ?? ?? ?? ?? "
    "48 89 F9 E8 ?? ?? ?? ?? 84 C0 0F 85 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? "
    "48 8B 88 00 A4 00 00 48 85 C9 0F 84 ?? ?? ?? ?? 44 89 F2 E8 ?? ?? ?? ??";
constexpr char kSourceGuard[] =
    "56 48 83 EC 30 48 89 CE 80 3D ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? "
    "80 3D ?? ?? ?? ?? 00 48 8B 05 ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? "
    "48 8B 88 80 9E 00 00 48 85 C9 0F 84 ?? ?? ?? ?? E8 ?? ?? ?? ?? 84 C0 75 26";
constexpr char kDungeonTypeCheck[] =
    "56 57 53 48 83 EC 20 89 CE 80 3D ?? ?? ?? ?? 00 48 8B 05 ?? ?? ?? ?? "
    "75 61 48 83 B8 80 9E 00 00 00 74 78 48 8B 88 90 BE 00 00 "
    "48 85 C9 74 71 E8 ?? ?? ?? ?? 48 85 C0 74 6C 8B 78 28";
constexpr char kDailyFightCall[] = "B9 03 00 00 00 E8 ?? ?? ?? ?? 84 C0";

using SourceJump = void (*)(void*, uint32_t, void*);
using TypeCheck = bool (*)(int, void*);
std::atomic<bool> g_enabled{false};
std::atomic<bool> g_faulted{false};
std::atomic<const char*> g_note{"disabled by config; hooks not created"};
SourceJump g_originalSource = nullptr;
TypeCheck g_originalType = nullptr;
const unsigned char* const* g_modules = nullptr;
void* g_dailyCallers[2]{};
void* g_source = nullptr;
void* g_type = nullptr;
bool g_created = false;
bool g_sourceEnabled = false;
bool g_typeEnabled = false;
thread_local bool g_sourceMapJump = false;

bool Sha256File(const wchar_t* path, std::wstring& output) {
    output.clear();
    HCRYPTPROV provider = 0;
    HCRYPTHASH hash = 0;
    FILE* file = nullptr;
    bool okay = false;
    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) goto done;
    if (!CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash)) goto done;
    if (_wfopen_s(&file, path, L"rb") || !file) goto done;
    {
        BYTE buffer[64 * 1024]{};
        for (;;) {
            const size_t count = std::fread(buffer, 1, sizeof(buffer), file);
            if (count && !CryptHashData(hash, buffer, static_cast<DWORD>(count), 0)) goto done;
            if (count < sizeof(buffer)) {
                if (std::ferror(file)) goto done;
                break;
            }
        }
    }
    {
        BYTE bytes[32]{};
        DWORD size = sizeof(bytes);
        if (!CryptGetHashParam(hash, HP_HASHVAL, bytes, &size, 0) || size != sizeof(bytes)) goto done;
        constexpr wchar_t digits[] = L"0123456789ABCDEF";
        for (BYTE byte : bytes) {
            output.push_back(digits[byte >> 4]);
            output.push_back(digits[byte & 15]);
        }
    }
    okay = true;
done:
    if (file) std::fclose(file);
    if (hash) CryptDestroyHash(hash);
    if (provider) CryptReleaseContext(provider, 0);
    return okay;
}

size_t ImageSize() {
    const auto* base = reinterpret_cast<const unsigned char*>(GetModuleHandleW(nullptr));
    if (!base) return 0;
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) return 0;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
        return 0;
    return nt->OptionalHeader.SizeOfImage;
}

bool InImage(const void* address, size_t size, size_t imageSize) {
    const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    const auto value = reinterpret_cast<uintptr_t>(address);
    return value >= base && value - base <= imageSize && size <= imageSize - (value - base);
}

bool PatternAt(const unsigned char* address, const char* signature, size_t imageSize) {
    if (!address || !InImage(address, 1, imageSize)) return false;
    std::istringstream words(signature);
    std::string word;
    size_t offset = 0;
    while (words >> word) {
        if (!InImage(address, offset + 1, imageSize)) return false;
        if (word != "?" && word != "??") {
            char* end = nullptr;
            const unsigned long byte = std::strtoul(word.c_str(), &end, 16);
            if (!end || *end || byte > 255 || address[offset] != byte) return false;
        }
        ++offset;
    }
    return offset != 0;
}

template <typename T>
T Read(const void* object, size_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const unsigned char*>(object) + offset);
}

void* Object(const void* object, size_t offset) {
    return object ? Read<void*>(object, offset) : nullptr;
}

bool StableDomain() {
    const auto modules = *g_modules;
    if (!modules) return false;
    const auto scene = Object(Object(modules, 0xB330), 0x60);
    const auto script = Object(scene, 0x28);
    const auto transition = Object(Object(modules, 0xBE90), 0x30);
    const auto dungeon = Object(modules, 0xA400);
    if (!scene || !script || !transition || !dungeon) return false;
    const auto dungeonId = Read<uint32_t>(dungeon, 0x2CC);
    return Read<int>(script, 0x70) == 1 && Read<int>(scene, 0x64) == 4 &&
        Read<int>(transition, 0x38) == 10 && dungeonId != 0 &&
        dungeonId == Read<uint32_t>(transition, 0x28);
}

void HookSourceJump(void* instance, uint32_t sourceId, void* methodInfo) {
    const bool previous = g_sourceMapJump;
    g_sourceMapJump = g_enabled.load() && !g_faulted.load() && sourceId != 0;
    __try { g_originalSource(instance, sourceId, methodInfo); }
    __finally { g_sourceMapJump = previous; }
}

bool HookTypeCheck(int type, void* methodInfo) {
    const auto caller = _ReturnAddress();
    const bool native = g_originalType(type, methodInfo);
    if (!native || type != 3 || !g_sourceMapJump || !g_enabled.load() || g_faulted.load() ||
        (caller != g_dailyCallers[0] && caller != g_dailyCallers[1]))
        return native;
    __try {
        if (StableDomain() && g_enabled.load() && !g_faulted.load()) return false;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        g_faulted.store(true);
        g_note.store("domain state unavailable; native restriction retained");
    }
    return native;
}

bool CreateHooks() {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring digest;
    if (!length || length >= MAX_PATH || !Sha256File(path, digest)) {
        g_note.store("game executable SHA256 unavailable; domain map disabled");
        return false;
    }
    if (digest != kCn72Sha256) {
        g_note.store("unsupported CN executable SHA256; domain map disabled");
        return false;
    }
    const size_t imageSize = ImageSize();
    if (!imageSize) {
        g_note.store("invalid game image; domain map disabled");
        return false;
    }
    auto* source = static_cast<unsigned char*>(Scanner::ScanMainMod(kSourceEntry));
    auto* type = static_cast<unsigned char*>(Scanner::ScanMainMod(kDungeonTypeCheck));
    if (!source || !type || !PatternAt(source, kSourceEntry, imageSize) ||
        !PatternAt(type, kDungeonTypeCheck, imageSize) ||
        !PatternAt(source + 0x2D, "E8 ?? ?? ?? ??", imageSize)) {
        g_note.store("CN 7.2 domain map entry mismatch; disabled");
        return false;
    }
    auto* guard = static_cast<unsigned char*>(Scanner::ResolveRelative(source + 0x2D, 1, 5));
    if (!PatternAt(guard, kSourceGuard, imageSize)) {
        g_note.store("CN 7.2 domain map guard mismatch; disabled");
        return false;
    }
    for (unsigned i = 0; i < 2; ++i) {
        auto* call = guard + (i == 0 ? 0x1BD : 0x262);
        if (!PatternAt(call, kDailyFightCall, imageSize) ||
            Scanner::ResolveRelative(call + 5, 1, 5) != type) {
            g_note.store("CN 7.2 domain map caller mismatch; disabled");
            return false;
        }
        g_dailyCallers[i] = call + 0xA;
    }
    if (!InImage(type + 0x10, 7, imageSize) || !InImage(guard + 0x1C, 7, imageSize)) {
        g_note.store("CN 7.2 domain map module reference mismatch; disabled");
        return false;
    }
    auto* modules = static_cast<unsigned char*>(Scanner::ResolveRelative(type + 0x10, 3, 7));
    if (modules != Scanner::ResolveRelative(guard + 0x1C, 3, 7) ||
        !InImage(modules, sizeof(void*), imageSize)) {
        g_note.store("CN 7.2 domain map module reference mismatch; disabled");
        return false;
    }
    g_modules = reinterpret_cast<const unsigned char* const*>(modules);
    g_source = source;
    g_type = type;
    if (MH_CreateHook(g_source, &HookSourceJump,
                      reinterpret_cast<void**>(&g_originalSource)) != MH_OK) {
        g_note.store("domain map source hook creation failed");
        return false;
    }
    if (MH_CreateHook(g_type, &HookTypeCheck,
                      reinterpret_cast<void**>(&g_originalType)) != MH_OK) {
        MH_RemoveHook(g_source);
        g_originalSource = nullptr;
        g_note.store("domain map type hook creation failed");
        return false;
    }
    g_created = true;
    return true;
}
} // namespace

namespace DomainMap {
const char* DiagnosticNote() { return g_note.load(); }

bool SetHookEnabled(bool enabled) {
    if (!enabled) {
        g_enabled.store(false);
        bool okay = true;
        if (g_typeEnabled) {
            if (MH_DisableHook(g_type) == MH_OK) g_typeEnabled = false;
            else okay = false;
        }
        if (g_sourceEnabled) {
            if (MH_DisableHook(g_source) == MH_OK) g_sourceEnabled = false;
            else okay = false;
        }
        g_note.store(okay ? (g_created ? "disabled by config; targets restored" :
                                      "disabled by config; hooks not created") :
                            "domain map hook disable failed");
        return okay;
    }
    if (g_faulted.load()) {
        g_note.store("domain state faulted; native restriction retained");
        return false;
    }
    if (!g_created && !CreateHooks()) return false;
    if (!g_sourceEnabled) {
        if (MH_EnableHook(g_source) != MH_OK) {
            g_note.store("domain map source hook enable failed");
            return false;
        }
        g_sourceEnabled = true;
    }
    if (!g_typeEnabled) {
        if (MH_EnableHook(g_type) != MH_OK) {
            if (MH_DisableHook(g_source) == MH_OK) g_sourceEnabled = false;
            g_note.store("domain map type hook enable failed");
            return false;
        }
        g_typeEnabled = true;
    }
    g_enabled.store(true);
    g_note.store("CN 7.2 domain item-source navigation active");
    return true;
}
} // namespace DomainMap
