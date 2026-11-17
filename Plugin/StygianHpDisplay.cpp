#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "StygianHpDisplay.h"
#include "MinHook.h"
#include "../ThirdParty/DisplayCD/src/Native.h"
#include <Windows.h>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>

// Adapted from the locally validated StygianHpDisplay v141 probe. This feature
// intentionally reads only the tested first-room runtime entity (0x1000000F).
namespace {
constexpr uintptr_t kGetValidEntityRva = 0x117AB3A0;
constexpr uintptr_t kCombatBarUpdateRva = 0x13EF5C90;
constexpr uintptr_t kPropManagerGlobalRva = 0x586B600;
constexpr uintptr_t kPropStoreLookupRva = 0xE9B1040;
constexpr uintptr_t kPropStoreGetRva = 0x12F80950;
constexpr uintptr_t kComponentTransformRva = 0x1A230450;
constexpr uintptr_t kComponentGameObjectRva = 0x1A230470;
constexpr uintptr_t kActiveInHierarchyRva = 0x1A20F460;
constexpr uintptr_t kRectTransformGetRectRva = 0x1A2271F0;
constexpr uintptr_t kTransformPointRva = 0x1A22BFE0;
constexpr uint32_t kTestedRuntimeId = 0x1000000F;
constexpr uint64_t kPollMs = 100;
constexpr uint64_t kSampleTimeoutMs = 750;
constexpr uint64_t kUiHandoffMs = 750;
constexpr float kLabelWidth = 420.0f;
constexpr float kLabelHeight = 40.0f;
constexpr float kLabelGap = 16.0f;

constexpr Patterns::Method kUiTextSetText{0x1A30DD90,
    "56 57 48 83 EC 28 48 89 CE 48 85 D2 0F 84 C2 00 00 00 48 89 D7 8B 42 10"};
constexpr Patterns::Method kUiTextSetColor{0x1A309AA0,
    "56 48 83 EC 50 0F 29 74 24 40 48 89 CE 0F 10 02 F3 0F 10 49 24 0F 2E C8"};
constexpr Patterns::Method kUiTextSetAlignment{0x1A30A280,
    "56 57 48 83 EC 28 48 8B 81 D8 00 00 00 48 85 C0 0F 84 8B 00 00 00 39 50"};
constexpr Patterns::Method kUiTextSetFontSize{0x1A30E360,
    "56 57 48 83 EC 28 48 8B 81 D8 00 00 00 48 85 C0 0F 84 8B 00 00 00 39 50 18"};
constexpr Patterns::Method kUiTextSetBestFit{0x1A30E060,
    "56 57 48 83 EC 28 48 8B 81 D8 00 00 00 48 85 C0 0F 84 94 00 00 00 48 89 CE 80 78 20"};

constexpr BYTE kEntityPrefix[] = {
    0x56,0x57,0x55,0x53,0x48,0x83,0xEC,0x28,0x44,0x89,0xC5,0x89,0xD6,0x48,0x89,0xCF};
constexpr BYTE kBarPrefix[] = {
    0x56,0x57,0x48,0x83,0xEC,0x28,0x48,0x89,0xCE,0x80,0x3D,0x84,0x71,0x8D,0xF1,0x00};
constexpr BYTE kStoreLookupPrefix[] = {
    0x56,0x57,0x48,0x83,0xEC,0x28,0x89,0xD6,0x48,0x89,0xCF,0x80,0x3D,0x50,0xD4,0xE4};
constexpr BYTE kStoreGetPrefix[] = {
    0x56,0x57,0x48,0x83,0xEC,0x78,0x0F,0x29,0x74,0x24,0x60,0x89,0xD6,0x48,0x89,0xCF};
constexpr BYTE kComponentTransformPrefix[] = {
    0xE9,0x9B,0x48,0x23,0xE7,0x66,0x66,0x2E,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00};
constexpr BYTE kComponentGameObjectPrefix[] = {
    0xE9,0xCB,0x48,0x23,0xE7,0x66,0x66,0x2E,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00};
constexpr BYTE kActivePrefix[] = {
    0xE9,0x2B,0x4B,0x25,0xE7,0x66,0x66,0x2E,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00};
constexpr BYTE kRectPrefix[] = {
    0x56,0x48,0x83,0xEC,0x20,0x48,0x89,0xCE,0x0F,0x57,0xC0,0x0F,0x11,0x01,0x48,0x89};
constexpr BYTE kTransformPointPrefix[] = {
    0x56,0x48,0x83,0xEC,0x20,0x48,0x89,0xCE,0xC7,0x41,0x08,0x00,0x00,0x00,0x00,0x48};

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Rect { float x, y, width, height; };
struct alignas(16) Color { float r, g, b, a; };
using GetValidEntityFn = void* (__fastcall*)(void*, uint32_t, uint64_t);
using CombatBarUpdateFn = void (__fastcall*)(void*, void*);
using StoreLookupFn = void* (__fastcall*)(void*, uint32_t, void*);
using StoreGetFn = float (__fastcall*)(void*, int32_t, void*);
using ComponentFn = void* (__fastcall*)(void*, void*);
using ActiveFn = bool (__fastcall*)(void*, void*);
using GetRectFn = Rect* (__fastcall*)(Rect*, void*, void*);
using TransformPointFn = Vec3* (__fastcall*)(Vec3*, void*, const Vec3*, void*);
using CloneTextFn = void* (__fastcall*)(void*, void*, bool, void*);
using DestroyObjectFn = void (__fastcall*)(void*, void*);
using StringNewFn = void* (__fastcall*)(const char*);
using SetBoolFn = void (__fastcall*)(void*, bool, void*);
using SetIntFn = void (__fastcall*)(void*, int, void*);
using SetTextFn = void (__fastcall*)(void*, void*, void*);
using SetVec2Fn = void (__fastcall*)(void*, Vec2, void*);
using SetVec3Fn = void (__fastcall*)(void*, const Vec3*, void*);
using SetColorFn = void (__fastcall*)(void*, const Color*, void*);

uintptr_t g_image = 0;
GetValidEntityFn g_originalEntity = nullptr;
CombatBarUpdateFn g_originalBarUpdate = nullptr;
StoreLookupFn g_storeLookup = nullptr;
StoreGetFn g_storeGet = nullptr;
ComponentFn g_getTransform = nullptr, g_getGameObject = nullptr;
ActiveFn g_activeInHierarchy = nullptr;
GetRectFn g_getRect = nullptr;
TransformPointFn g_transformPoint = nullptr;
CloneTextFn g_cloneText = nullptr;
DestroyObjectFn g_destroyObject = nullptr;
StringNewFn g_stringNew = nullptr;
SetBoolFn g_setActive = nullptr, g_setRaycast = nullptr, g_setEnabled = nullptr, g_setBestFit = nullptr;
SetIntFn g_setAlignment = nullptr, g_setFontSize = nullptr;
SetTextFn g_setText = nullptr;
SetVec2Fn g_setSize = nullptr, g_setAnchorMin = nullptr, g_setAnchorMax = nullptr, g_setPivot = nullptr;
SetVec3Fn g_setPosition = nullptr, g_setScale = nullptr;
SetColorFn g_setColor = nullptr;
std::atomic<bool> g_enabled{false}, g_showMaximum{true};
std::atomic<const char*> g_note{"disabled by config"};
std::atomic<uint32_t> g_currentBits{0}, g_maximumBits{0};
std::atomic<uint64_t> g_lastSample{0}, g_maxChanged{0}, g_lastStorePoll{0};
bool g_created = false, g_hooksEnabled = false;
SRWLOCK g_uiLock = SRWLOCK_INIT;
DWORD g_uiThread = 0;
uint64_t g_lastUiUpdate = 0;

struct LabelState {
    uintptr_t bar = 0;
    void* text = nullptr;
    uint64_t lastRefresh = 0;
    char last[64]{};
    Vec3 position{};
    bool positioned = false;
    bool shown = false;
};
LabelState g_label{}; // guarded by g_uiLock

void ResetSamples() {
    g_currentBits.store(0);
    g_maximumBits.store(0);
    g_lastSample.store(0);
    g_maxChanged.store(0);
}

void RecordRuntimeFault(const char* note) {
    ResetSamples();
    g_note.store(note);
}

bool ReadExact(uintptr_t address, void* out, size_t bytes) {
    SIZE_T read = 0;
    return address >= 0x10000 && bytes < 0x100000 &&
        address < 0x00007FFFFFFFFFFFULL - bytes &&
        ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(address),
                          out, bytes, &read) && read == bytes;
}

template <size_t N> bool MatchPrefix(uintptr_t rva, const BYTE (&bytes)[N]) {
    BYTE actual[N]{};
    return rva + N <= 0x1B65F000 && ReadExact(g_image + rva, actual, N) &&
           std::memcmp(actual, bytes, N) == 0;
}

int HexDigit(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

bool MatchMethod(const Patterns::Method& method) {
    if (!method.rva || !method.bytes || method.rva >= 0x1B65F000) return false;
    size_t offset = 0;
    const char* pattern = method.bytes;
    while (*pattern) {
        while (*pattern == ' ') ++pattern;
        if (!*pattern) break;
        if (!pattern[1] || method.rva + offset >= 0x1B65F000) return false;
        const int hi = HexDigit(pattern[0]), lo = HexDigit(pattern[1]);
        BYTE actual = 0;
        if (hi < 0 || lo < 0 || !ReadExact(g_image + method.rva + offset, &actual, 1) ||
            actual != static_cast<BYTE>(hi * 16 + lo)) return false;
        pattern += 2;
        ++offset;
    }
    return offset >= 8;
}

bool ValidateTargets() {
    g_image = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    if (!g_image) return false;
    IMAGE_DOS_HEADER dos{};
    if (!ReadExact(g_image, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE ||
        dos.e_lfanew <= 0 || dos.e_lfanew > 0x1000) return false;
    IMAGE_NT_HEADERS64 nt{};
    if (!ReadExact(g_image + dos.e_lfanew, &nt, sizeof(nt)) ||
        nt.Signature != IMAGE_NT_SIGNATURE ||
        nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
        nt.FileHeader.TimeDateStamp != 0x6AD886FB ||
        nt.OptionalHeader.SizeOfImage != 0x1B65F000 ||
        nt.OptionalHeader.CheckSum != 0x1AE0CBAC) return false;
    if (!MatchPrefix(kGetValidEntityRva, kEntityPrefix) ||
        !MatchPrefix(kCombatBarUpdateRva, kBarPrefix) ||
        !MatchPrefix(kPropStoreLookupRva, kStoreLookupPrefix) ||
        !MatchPrefix(kPropStoreGetRva, kStoreGetPrefix) ||
        !MatchPrefix(kComponentTransformRva, kComponentTransformPrefix) ||
        !MatchPrefix(kComponentGameObjectRva, kComponentGameObjectPrefix) ||
        !MatchPrefix(kActiveInHierarchyRva, kActivePrefix) ||
        !MatchPrefix(kRectTransformGetRectRva, kRectPrefix) ||
        !MatchPrefix(kTransformPointRva, kTransformPointPrefix)) return false;
    const auto& p = Patterns::SkillCooldownCN72;
    const Patterns::Method* methods[] = {
        &p.StringNew, &p.CloneText, &p.DestroyObject, &p.GetTransform, &p.GetGameObject,
        &p.SetEnabled, &p.SetActive, &p.SetRaycast, &p.SetScale, &p.SetSize,
        &p.SetPosition, &p.SetAnchorMin, &p.SetAnchorMax, &p.SetPivot,
        &kUiTextSetText, &kUiTextSetColor, &kUiTextSetAlignment,
        &kUiTextSetFontSize, &kUiTextSetBestFit
    };
    for (const auto* method : methods)
        if (!MatchMethod(*method)) return false;
    g_storeLookup = reinterpret_cast<StoreLookupFn>(g_image + kPropStoreLookupRva);
    g_storeGet = reinterpret_cast<StoreGetFn>(g_image + kPropStoreGetRva);
    g_getTransform = reinterpret_cast<ComponentFn>(g_image + kComponentTransformRva);
    g_getGameObject = reinterpret_cast<ComponentFn>(g_image + kComponentGameObjectRva);
    g_activeInHierarchy = reinterpret_cast<ActiveFn>(g_image + kActiveInHierarchyRva);
    g_getRect = reinterpret_cast<GetRectFn>(g_image + kRectTransformGetRectRva);
    g_transformPoint = reinterpret_cast<TransformPointFn>(g_image + kTransformPointRva);
    g_cloneText = reinterpret_cast<CloneTextFn>(g_image + p.CloneText.rva);
    g_destroyObject = reinterpret_cast<DestroyObjectFn>(g_image + p.DestroyObject.rva);
    g_stringNew = reinterpret_cast<StringNewFn>(g_image + p.StringNew.rva);
    g_setActive = reinterpret_cast<SetBoolFn>(g_image + p.SetActive.rva);
    g_setRaycast = reinterpret_cast<SetBoolFn>(g_image + p.SetRaycast.rva);
    g_setEnabled = reinterpret_cast<SetBoolFn>(g_image + p.SetEnabled.rva);
    g_setBestFit = reinterpret_cast<SetBoolFn>(g_image + kUiTextSetBestFit.rva);
    g_setAlignment = reinterpret_cast<SetIntFn>(g_image + kUiTextSetAlignment.rva);
    g_setFontSize = reinterpret_cast<SetIntFn>(g_image + kUiTextSetFontSize.rva);
    g_setText = reinterpret_cast<SetTextFn>(g_image + kUiTextSetText.rva);
    g_setColor = reinterpret_cast<SetColorFn>(g_image + kUiTextSetColor.rva);
    g_setSize = reinterpret_cast<SetVec2Fn>(g_image + p.SetSize.rva);
    g_setAnchorMin = reinterpret_cast<SetVec2Fn>(g_image + p.SetAnchorMin.rva);
    g_setAnchorMax = reinterpret_cast<SetVec2Fn>(g_image + p.SetAnchorMax.rva);
    g_setPivot = reinterpret_cast<SetVec2Fn>(g_image + p.SetPivot.rva);
    g_setPosition = reinterpret_cast<SetVec3Fn>(g_image + p.SetPosition.rva);
    g_setScale = reinterpret_cast<SetVec3Fn>(g_image + p.SetScale.rva);
    return true;
}

bool Alive(void* object) {
    uintptr_t native = 0;
    return object && ReadExact(reinterpret_cast<uintptr_t>(object) + 0x10,
                               &native, sizeof(native)) && native != 0;
}

void HideLabel() {
    if (g_label.shown && Alive(g_label.text)) {
        void* gameObject = g_getGameObject(g_label.text, nullptr);
        if (Alive(gameObject)) g_setActive(gameObject, false, nullptr);
    }
    g_label.shown = false;
}

void RetireLabel() {
    if (Alive(g_label.text)) {
        void* object = g_getGameObject(g_label.text, nullptr);
        if (Alive(object)) {
            g_setActive(object, false, nullptr);
            g_destroyObject(object, nullptr);
        }
    }
    g_label = {};
}

// Only call while holding g_uiLock. A reset retires the old cloned label and
// lets the next valid sample select the current UI callback thread again.
void ResetUiState() {
    RetireLabel();
    g_uiThread = 0;
    g_lastUiUpdate = 0;
}

void* FindTextSource(uintptr_t bar) {
    uintptr_t source = 0, sourceClass = 0;
    if (!ReadExact(bar + 0xA8, &source, sizeof(source)) ||
        !Alive(reinterpret_cast<void*>(source)) ||
        !ReadExact(source, &sourceClass, sizeof(sourceClass)) || !sourceClass)
        return nullptr;
    for (uintptr_t offset : {0x70ULL, 0x78ULL, 0x80ULL}) {
        uintptr_t peer = 0, peerClass = 0;
        if (ReadExact(bar + offset, &peer, sizeof(peer)) && peer != source &&
            Alive(reinterpret_cast<void*>(peer)) &&
            ReadExact(peer, &peerClass, sizeof(peerClass)) && peerClass == sourceClass)
            return reinterpret_cast<void*>(source);
    }
    return nullptr;
}

bool CreateLabel(uintptr_t bar, uintptr_t parent) {
    void* source = FindTextSource(bar);
    if (!source || !Alive(reinterpret_cast<void*>(parent))) return false;
    void* label = g_cloneText(source, reinterpret_cast<void*>(parent), false, nullptr);
    if (!Alive(label)) return false;
    g_label.text = label;
    g_label.last[0] = '\0';
    void* labelObject = g_getGameObject(label, nullptr);
    if (!Alive(labelObject)) return false;
    g_setActive(labelObject, false, nullptr);
    g_setRaycast(label, false, nullptr);
    g_setAlignment(label, 3, nullptr); // TextAnchor.MiddleLeft
    g_setEnabled(label, true, nullptr);
    const Color white{1, 1, 1, 1};
    g_setColor(label, &white, nullptr);
    void* transform = g_getTransform(label, nullptr);
    if (!Alive(transform)) return false;
    const Vec3 scale{1, 1, 1};
    const Vec2 centre{0.5f, 0.5f};
    g_setScale(transform, &scale, nullptr);
    g_setAnchorMin(transform, centre, nullptr);
    g_setAnchorMax(transform, centre, nullptr);
    g_setPivot(transform, centre, nullptr);
    g_setSize(transform, Vec2{kLabelWidth, kLabelHeight}, nullptr);
    g_setBestFit(label, false, nullptr);
    g_setFontSize(label, 24, nullptr);
    g_label.positioned = false;
    g_label.shown = false;
    return true;
}

void FormatHp(char (&output)[64], float value) {
    char raw[32]{};
    sprintf_s(raw, "%llu", static_cast<unsigned long long>(std::llround(value)));
    const size_t length = std::strlen(raw);
    size_t used = 0;
    for (size_t i = 0; i < length && used + 2 < sizeof(output); ++i) {
        output[used++] = raw[i];
        if ((length - i - 1) % 3 == 0 && i + 1 < length) output[used++] = ',';
    }
    output[used] = '\0';
}

void UpdateLabel(uintptr_t bar, uintptr_t parent, const Rect& rect, bool active) {
    const uint64_t now = GetTickCount64();
    const DWORD thread = GetCurrentThreadId();
    if (g_uiThread && thread != g_uiThread) {
        if (now - g_lastUiUpdate <= kUiHandoffMs) return;
        ResetUiState();
    }
    if (!active ||
        rect.width < 600 || rect.width > 760 ||
        rect.height < 10 || rect.height > 25) {
        if (g_uiThread == thread) HideLabel();
        return;
    }
    const uint64_t sampled = g_lastSample.load();
    const uint64_t maxChanged = g_maxChanged.load();
    if (!sampled || now - sampled > kSampleTimeoutMs ||
        !maxChanged || now - maxChanged < 500) {
        if (sampled && now - sampled > kSampleTimeoutMs) {
            ResetUiState();
            ResetSamples();
        } else if (g_uiThread == thread) {
            HideLabel();
        }
        return;
    }
    if (!g_uiThread) g_uiThread = thread;
    if (bar != g_label.bar) {
        RetireLabel();
        g_label.bar = bar;
    }
    if (now - g_label.lastRefresh < kPollMs) {
        if (g_label.shown) g_lastUiUpdate = now;
        return;
    }
    g_label.lastRefresh = now;
    float current = 0, maximum = 0;
    const uint32_t currentBits = g_currentBits.load(), maximumBits = g_maximumBits.load();
    std::memcpy(&current, &currentBits, sizeof(current));
    std::memcpy(&maximum, &maximumBits, sizeof(maximum));
    if (!std::isfinite(current) || !std::isfinite(maximum) ||
        maximum <= 0 || current < 0 || current > maximum * 1.05f) {
        HideLabel();
        return;
    }
    if (!Alive(g_label.text) && !CreateLabel(bar, parent)) return;
    void* transform = g_getTransform(g_label.text, nullptr);
    if (!Alive(transform)) return;
    const Vec3 local{rect.x + rect.width + kLabelGap + kLabelWidth * 0.5f,
                     rect.y + rect.height * 0.5f, 0};
    Vec3 world{};
    g_transformPoint(&world, reinterpret_cast<void*>(parent), &local, nullptr);
    if (!std::isfinite(world.x) || !std::isfinite(world.y) || !std::isfinite(world.z)) return;
    if (!g_label.positioned ||
        std::fabs(world.x - g_label.position.x) > 0.0001f ||
        std::fabs(world.y - g_label.position.y) > 0.0001f ||
        std::fabs(world.z - g_label.position.z) > 0.0001f) {
        g_setPosition(transform, &world, nullptr);
        g_label.position = world;
        g_label.positioned = true;
    }
    char currentText[64]{}, maximumText[64]{}, value[64]{};
    FormatHp(currentText, current);
    if (g_showMaximum.load()) {
        FormatHp(maximumText, maximum);
        sprintf_s(value, "%s / %s", currentText, maximumText);
    } else {
        strcpy_s(value, currentText);
    }
    if (std::strcmp(value, g_label.last) != 0) {
        void* string = g_stringNew(value);
        if (!string) return;
        g_setText(g_label.text, string, nullptr);
        strcpy_s(g_label.last, value);
    }
    if (!g_label.shown) {
        void* object = g_getGameObject(g_label.text, nullptr);
        if (Alive(object)) {
            g_setActive(object, true, nullptr);
            g_label.shown = true;
        }
    }
    if (g_label.shown) {
        g_lastUiUpdate = now;
        g_note.store("CN 7.2 first-room single boss HP display active");
    }
}

void SampleStore() {
    const uint64_t now = GetTickCount64();
    uint64_t previous = g_lastStorePoll.load();
    if (now - previous < kPollMs ||
        !g_lastStorePoll.compare_exchange_strong(previous, now)) return;
    uintptr_t root = 0, manager = 0;
    if (!ReadExact(g_image + kPropManagerGlobalRva, &root, sizeof(root)) ||
        !ReadExact(root + 0xA650, &manager, sizeof(manager)) || !manager) return;
    void* store = g_storeLookup(reinterpret_cast<void*>(manager), kTestedRuntimeId, nullptr);
    if (!store) return;
    const float current = g_storeGet(store, 1010, nullptr);
    const float maximum = g_storeGet(store, 2000, nullptr);
    if (!std::isfinite(current) || !std::isfinite(maximum) ||
        maximum <= 0 || current < 0 || current > maximum * 1.05f) return;
    uint32_t currentBits = 0, maximumBits = 0;
    std::memcpy(&currentBits, &current, sizeof(currentBits));
    std::memcpy(&maximumBits, &maximum, sizeof(maximumBits));
    if (g_maximumBits.exchange(maximumBits) != maximumBits) g_maxChanged.store(now);
    g_currentBits.store(currentBits);
    g_lastSample.store(now);
}

void* __fastcall HookGetValidEntity(void* manager, uint32_t runtimeId, uint64_t checkActive) {
    void* entity = g_originalEntity(manager, runtimeId, checkActive);
    if (entity && runtimeId == kTestedRuntimeId && g_enabled.load()) {
        __try { SampleStore(); }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            RecordRuntimeFault("HP read fault; waiting for next entity query");
        }
    }
    return entity;
}

void ProcessCombatBarUpdateUnsafe(void* object) {
    if (!g_enabled.load()) {
        if (g_label.text || g_uiThread) ResetUiState();
        return;
    }
    const uint64_t sampled = g_lastSample.load();
    if (sampled && GetTickCount64() - sampled > kSampleTimeoutMs) {
        ResetUiState();
        ResetSamples();
        return;
    }
    uintptr_t hpBar = 0;
    if (!ReadExact(reinterpret_cast<uintptr_t>(object) + 0x18, &hpBar, sizeof(hpBar)) ||
        !Alive(reinterpret_cast<void*>(hpBar))) return;
    void* transform = g_getTransform(reinterpret_cast<void*>(hpBar), nullptr);
    void* gameObject = g_getGameObject(reinterpret_cast<void*>(hpBar), nullptr);
    if (!Alive(transform) || !Alive(gameObject)) return;
    Rect rect{};
    g_getRect(&rect, transform, nullptr);
    if (!std::isfinite(rect.x) || !std::isfinite(rect.y) ||
        !std::isfinite(rect.width) || !std::isfinite(rect.height)) return;
    UpdateLabel(reinterpret_cast<uintptr_t>(object), reinterpret_cast<uintptr_t>(transform),
                rect, g_activeInHierarchy(gameObject, nullptr));
}

void ProcessCombatBarUpdate(void* object) {
    if (!TryAcquireSRWLockExclusive(&g_uiLock)) return;
    __try { ProcessCombatBarUpdateUnsafe(object); }
    __finally { ReleaseSRWLockExclusive(&g_uiLock); }
}

void ResetUiAfterFaultUnsafe() {
    __try { RetireLabel(); }
    __except (EXCEPTION_EXECUTE_HANDLER) { /* The old native UI object may be gone. */ }
    g_label = {};
    g_uiThread = 0;
    g_lastUiUpdate = 0;
}

void ResetUiAfterFault() {
    if (!TryAcquireSRWLockExclusive(&g_uiLock)) return;
    __try { ResetUiAfterFaultUnsafe(); }
    __finally { ReleaseSRWLockExclusive(&g_uiLock); }
}

void __fastcall HookCombatBarUpdate(void* object, void* methodInfo) {
    g_originalBarUpdate(object, methodInfo);
    if (!object) return;
    __try { ProcessCombatBarUpdate(object); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        RecordRuntimeFault("native UI fault; waiting for next entity query");
        ResetUiAfterFault();
    }
}

bool CreateHooks() {
    if (!ValidateTargets()) {
        g_note.store("unsupported game build or native method mismatch");
        return false;
    }
    void* entityTarget = reinterpret_cast<void*>(g_image + kGetValidEntityRva);
    void* barTarget = reinterpret_cast<void*>(g_image + kCombatBarUpdateRva);
    if (MH_CreateHook(entityTarget, &HookGetValidEntity,
                      reinterpret_cast<void**>(&g_originalEntity)) != MH_OK) {
        g_note.store("entity hook creation failed");
        return false;
    }
    if (MH_CreateHook(barTarget, &HookCombatBarUpdate,
                      reinterpret_cast<void**>(&g_originalBarUpdate)) != MH_OK) {
        MH_RemoveHook(entityTarget);
        g_originalEntity = nullptr;
        g_note.store("bar hook creation failed");
        return false;
    }
    g_created = true;
    return true;
}
} // namespace

namespace StygianHpDisplay {
const char* DiagnosticNote() { return g_note.load(); }

bool SetEnabled(bool enabled, bool showMaximum) {
    g_showMaximum.store(showMaximum);
    if (!enabled) {
        g_enabled.store(false);
        ResetSamples();
        g_note.store("disabled by config");
        return true;
    }
    if (!g_created && !CreateHooks()) return false;
    if (!g_hooksEnabled) {
        void* entityTarget = reinterpret_cast<void*>(g_image + kGetValidEntityRva);
        void* barTarget = reinterpret_cast<void*>(g_image + kCombatBarUpdateRva);
        if (MH_EnableHook(entityTarget) != MH_OK) {
            g_note.store("entity hook enable failed");
            return false;
        }
        if (MH_EnableHook(barTarget) != MH_OK) {
            MH_DisableHook(entityTarget);
            g_note.store("bar hook enable failed");
            return false;
        }
        g_hooksEnabled = true;
    }
    if (!g_enabled.exchange(true)) {
        ResetSamples();
        g_note.store("CN 7.2 first-room single boss HP display active");
    }
    return true;
}
} // namespace StygianHpDisplay
