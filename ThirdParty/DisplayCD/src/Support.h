// Copyright (c) FufuLauncher Dev Team. AGPL-3.0.
#pragma once
#include <Windows.h>
#include <filesystem>
#include <cstdint>
namespace Support
{
    std::filesystem::path Directory();
    void Log(const char* message);
    bool Match(const void* address, const char* bytes);
    // SetActive is shared with the main plugin/runtime. Accept its known
    // MinHook broker/UI observer only after checking the unmodified suffix and owner.
    bool SharedSetActive(void* address, const char* bytes);
}
