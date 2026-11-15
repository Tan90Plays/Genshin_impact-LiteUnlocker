#include "FpsHotkey.h"

#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace {
constexpr unsigned kCtrl = 1, kAlt = 2, kShift = 4, kWin = 8;
struct Binding {
    std::string source;
    int key = 0;
    unsigned modifiers = 0;
    bool wasDown = false;
};
Binding g_binding;

std::string TrimLower(std::string s) {
    const auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return {};
    const auto end = s.find_last_not_of(" \t\r\n");
    s = s.substr(begin, end - begin + 1);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

int KeyCode(const std::string& token) {
    if (token.size() == 1) {
        const char c = token[0];
        if (c >= 'a' && c <= 'z') return 'A' + c - 'a';
        if (c >= '0' && c <= '9') return '0' + c - '0';
    }
    if (token.size() >= 2 && token[0] == 'f') {
        char* end = nullptr;
        const long n = std::strtol(token.c_str() + 1, &end, 10);
        if (*end == '\0' && n >= 1 && n <= 24) return VK_F1 + static_cast<int>(n) - 1;
    }
    if (token.rfind("vk:0x", 0) == 0) {
        char* end = nullptr;
        const long n = std::strtol(token.c_str() + 5, &end, 16);
        if (*end == '\0' && n >= 1 && n <= 254 &&
            n != VK_CONTROL && n != VK_MENU && n != VK_SHIFT &&
            n != VK_LWIN && n != VK_RWIN &&
            n != VK_LCONTROL && n != VK_RCONTROL &&
            n != VK_LMENU && n != VK_RMENU &&
            n != VK_LSHIFT && n != VK_RSHIFT) return static_cast<int>(n);
    }
    constexpr std::pair<const char*, int> named[] = {
        {"space", VK_SPACE}, {"enter", VK_RETURN}, {"return", VK_RETURN},
        {"tab", VK_TAB}, {"esc", VK_ESCAPE}, {"escape", VK_ESCAPE},
        {"backspace", VK_BACK}, {"delete", VK_DELETE}, {"del", VK_DELETE},
        {"insert", VK_INSERT}, {"ins", VK_INSERT}, {"home", VK_HOME},
        {"end", VK_END}, {"page_up", VK_PRIOR}, {"pgup", VK_PRIOR},
        {"page_down", VK_NEXT}, {"pgdn", VK_NEXT}, {"up", VK_UP},
        {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
        {"caps_lock", VK_CAPITAL}, {"pause", VK_PAUSE},
        {"print_screen", VK_SNAPSHOT}, {"scroll_lock", VK_SCROLL},
        {"mouse_left", VK_LBUTTON}, {"mouse_right", VK_RBUTTON},
        {"mouse_middle", VK_MBUTTON}, {"mouse_x1", VK_XBUTTON1},
        {"mouse_x2", VK_XBUTTON2},
        {"numpad_add", VK_ADD}, {"numpad_subtract", VK_SUBTRACT},
        {"numpad_multiply", VK_MULTIPLY}, {"numpad_divide", VK_DIVIDE},
        {"numpad_decimal", VK_DECIMAL}
    };
    for (const auto& item : named) if (token == item.first) return item.second;
    if (token.rfind("numpad", 0) == 0 && token.size() == 7 &&
        token[6] >= '0' && token[6] <= '9') return VK_NUMPAD0 + token[6] - '0';
    return 0;
}

Binding Parse(std::string source) {
    Binding result;
    result.source = std::move(source);
    size_t start = 0;
    for (;;) {
        const size_t plus = result.source.find('+', start);
        const std::string token = TrimLower(result.source.substr(start, plus - start));
        if (token.empty()) { result.key = 0; return result; }
        unsigned modifier = 0;
        if (token == "ctrl" || token == "control") modifier = kCtrl;
        else if (token == "alt") modifier = kAlt;
        else if (token == "shift") modifier = kShift;
        else if (token == "win" || token == "cmd" || token == "windows") modifier = kWin;
        if (modifier) {
            if ((result.modifiers & modifier) || result.key) { result.key = 0; return result; }
            result.modifiers |= modifier;
        } else {
            if (result.key || plus != std::string::npos) { result.key = 0; return result; }
            result.key = KeyCode(token);
            if (!result.key) return result;
        }
        if (plus == std::string::npos) break;
        start = plus + 1;
    }
    return result;
}

bool Down(int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; }
unsigned CurrentModifiers() {
    return (Down(VK_CONTROL) ? kCtrl : 0) |
           (Down(VK_MENU) ? kAlt : 0) |
           (Down(VK_SHIFT) ? kShift : 0) |
           ((Down(VK_LWIN) || Down(VK_RWIN)) ? kWin : 0);
}
} // namespace

namespace FpsHotkey {
void Configure(const std::string& value) {
    if (g_binding.source != value) g_binding = Parse(value);
}

bool Poll(bool gameFocused) {
    const bool down = g_binding.key && Down(g_binding.key) &&
        g_binding.modifiers == CurrentModifiers();
    const bool pressed = gameFocused && down && !g_binding.wasDown;
    g_binding.wasDown = down;
    return pressed;
}
} // namespace FpsHotkey
