// Copyright (c) FufuLauncher Dev Team. AGPL-3.0.
#include "Support.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <mutex>

namespace Support
{
    std::filesystem::path Directory()
    {
        HMODULE module = nullptr;
        wchar_t path[32768]{};
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&Directory), &module) || !GetModuleFileNameW(module, path, 32768)) return {};
        return std::filesystem::path(path).parent_path();
    }
    void Log(const char* message)
    {
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        const auto path = Directory() / L"DisplayCD.log";
        std::error_code ec;
        const auto size = std::filesystem::file_size(path, ec);
        std::ofstream output(path, !ec && size > 1024 * 1024 ? std::ios::trunc : std::ios::app);
        SYSTEMTIME time{};
        GetLocalTime(&time);
        char stamp[64]{};
        sprintf_s(stamp, "[%04u-%02u-%02u %02u:%02u:%02u] ", time.wYear, time.wMonth, time.wDay,
            time.wHour, time.wMinute, time.wSecond);
        output << stamp << message << '\n';
    }
    bool Match(const void* address, const char* bytes)
    {
        __try
        {
            const auto data = static_cast<const unsigned char*>(address);
            for (size_t i = 0; *bytes; ++i)
            {
                unsigned expected = 0;
                if (sscanf_s(bytes, "%2x", &expected) != 1 || data[i] != expected) return false;
                bytes += 2;
                if (*bytes == ' ') ++bytes;
            }
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
    static bool Broker(void* address, const char* bytes, void** target)
    {
        __try
        {
            const auto code = static_cast<unsigned char*>(address);
            // MinHook x64 replaces exactly five bytes and jumps to an absolute
            // relay. Do not treat an arbitrary changed function as compatible.
            if (code[0] != 0xE9 || !Match(code + 5, bytes + 15)) return false;
            const auto relay = code + 5 + *reinterpret_cast<const int32_t*>(code + 1);
            const unsigned char prefix[] = {0xFF, 0x25, 0, 0, 0, 0};
            if (std::memcmp(relay, prefix, sizeof(prefix)) != 0) return false;
            std::memcpy(target, relay + 6, sizeof(*target));
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
    bool SharedSetActive(void* address, const char* bytes)
    {
        if (Match(address, bytes)) return true;
        void* target = nullptr;
        if (!Broker(address, bytes, &target)) return false;
        HMODULE owner = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCWSTR>(target), &owner)) return false;
        wchar_t path[32768]{};
        if (!GetModuleFileNameW(owner, path, 32768)) return false;
        // GetModuleFileNameW preserves the loader's spelling, including DOS 8.3
        // aliases such as MAINUI~1.DLL. Resolve that spelling before comparing
        // module identities; accepting an alias string itself would be ambiguous.
        std::wstring longPath(32768, L'\0');
        const auto length = GetLongPathNameW(path, longPath.data(), static_cast<DWORD>(longPath.size()));
        if (!length || length >= longPath.size()) return false;
        longPath.resize(length);
        const auto name = std::filesystem::path(longPath).filename().wstring();
        if (_wcsicmp(name.c_str(), L"FufuLauncher.UnlockerIsland.dll") != 0 &&
            _wcsicmp(name.c_str(), L"DetachedRuntime.dll") != 0 &&
            _wcsicmp(name.c_str(), L"MainUICustomizer.dll") != 0)
        {
            char moduleName[512]{};
            WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, moduleName, sizeof(moduleName), nullptr, nullptr);
            char message[600]{};
            sprintf_s(message, "SetActive rejected: unrecognized detour module %s.", moduleName);
            Log(message);
            return false;
        }
        MEMORY_BASIC_INFORMATION info{};
        if (!VirtualQuery(target, &info, sizeof(info)) || info.State != MEM_COMMIT ||
            (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)) ||
            !(info.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) return false;
        // MainUICustomizer observes activation and forwards the original request.
        // Keep the public entry so every broker/observer still receives our calls.
        return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
            static_cast<LPCWSTR>(target), &owner) != FALSE;
    }
}
