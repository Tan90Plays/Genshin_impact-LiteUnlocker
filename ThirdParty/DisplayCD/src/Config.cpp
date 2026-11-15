// Copyright (c) FufuLauncher Dev Team. AGPL-3.0.
#include "Config.h"
#include <atomic>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iterator>

namespace Config
{
    namespace
    {
        std::atomic<unsigned> current{7}; // E/Q/charges on, master off.
        std::string Trim(std::string value)
        {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return {};
            return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }
        std::string Lower(std::string value)
        {
            for (auto& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return value;
        }
        bool Boolean(std::string value, bool fallback)
        {
            value = Lower(Trim(value.substr(0, value.find_first_of(";#"))));
            if (value == "1" || value == "true") return true;
            if (value == "0" || value == "false") return false;
            return fallback;
        }
    }
    Settings Parse(std::string text)
    {
        if (text.compare(0, 3, "\xEF\xBB\xBF") == 0) text.erase(0, 3);
        else if (text.compare(0, 2, "\xFF\xFE") == 0)
        {
            // All section/key/value tokens are ASCII. Keep non-ASCII metadata
            // opaque rather than rewriting the user's file or its encoding.
            std::string ascii;
            for (size_t i = 2; i + 1 < text.size(); i += 2)
                ascii += text[i + 1] == 0 ? text[i] : '?';
            text = std::move(ascii);
        }
        Settings result;
        std::istringstream stream(text);
        std::string line, section;
        while (std::getline(stream, line))
        {
            line = Trim(line);
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            if (line.front() == '[' && line.back() == ']')
            { section = Lower(Trim(line.substr(1, line.size() - 2))); continue; }
            const auto equal = line.find('=');
            if (equal == std::string::npos || Lower(Trim(line.substr(0, equal))) != "value") continue;
            const auto value = line.substr(equal + 1);
            if (section == "showelementalskillcd") result.elementalSkill = Boolean(value, result.elementalSkill);
            else if (section == "showelementalburstcd") result.elementalBurst = Boolean(value, result.elementalBurst);
            else if (section == "showchargeindicators") result.chargeIndicators = Boolean(value, result.chargeIndicators);
            else if (section == "enableskillcd") result.enabled = Boolean(value, result.enabled);
        }
        return result;
    }
    Settings Get()
    {
        const auto bits = current.load(std::memory_order_relaxed);
        const bool enabled = bool(bits & 8);
        return {enabled && bool(bits & 1), enabled && bool(bits & 2),
                enabled && bool(bits & 4), enabled};
    }
    void Reload(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input) return; // A missing/transiently locked file keeps the last snapshot.
        std::string text((std::istreambuf_iterator<char>(input)), {});
        if (input.bad()) return;
        const auto settings = Parse(std::move(text));
        current.store(unsigned(settings.elementalSkill) | (unsigned(settings.elementalBurst) << 1) |
            (unsigned(settings.chargeIndicators) << 2) | (unsigned(settings.enabled) << 3),
            std::memory_order_relaxed);
    }
}
