// Copyright (c) FufuLauncher Dev Team. AGPL-3.0.
#pragma once
#include <filesystem>
#include <string>

namespace Config
{
    struct Settings
    {
        bool elementalSkill = true;
        bool elementalBurst = true;
        bool chargeIndicators = true;
        bool enabled = false; // LiteUnlocker master switch; no HUD hooks until enabled.
    };
    // Complete snapshots are published atomically; HUD callbacks never read files.
    Settings Get();
    Settings Parse(std::string text);
    void Reload(const std::filesystem::path& path);
}
