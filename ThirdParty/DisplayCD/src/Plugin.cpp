// Copyright (c) FufuLauncher Dev Team. AGPL-3.0.
#include "Config.h"
#include "Support.h"
#include "HookSetupGuard.h"
#include "SkillCooldown/SkillCooldown.h"
#include <cwchar>
#include <exception>

namespace
{
    DWORD WINAPI Worker(void*)
    {
        // The injector owns DLL selection. Keep callbacks resident until exit.
        HMODULE pinned = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&Worker), &pinned)) return 0;
        wchar_t executable[32768]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
        if (length == 0 || length >= 32768) return 0;
        const auto* separator = std::wcsrchr(executable, L'\\');
        const auto* executableName = separator ? separator + 1 : executable;
        if (!SkillCooldown::IsSupportedExecutable(executableName))
        { Support::Log("Inactive: expected YuanShen.exe or GenshinImpact.exe."); return 0; }
        try
        {
            const auto config = Support::Directory() / L"config.ini";
            Config::Reload(config);
            bool initialized = false;
            for (;;)
            {
                if (Config::Get().enabled && !initialized)
                {
                    Detached::HookSetupGuard guard;
                    if (!guard) { Support::Log("Inactive: timed out waiting for shared hook setup."); return 0; }
                    if (!SkillCooldown::Init(executableName)) return 0;
                    initialized = true;
                    Support::Log("DisplayCD " DISPLAYCD_VERSION " initialized; configuration reload interval: 250 ms.");
                }
                Sleep(250);
                Config::Reload(config);
            }
        }
        catch (const std::exception& error) { Support::Log(error.what()); }
        return 0;
    }
}
BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        // Keep CRT thread notifications: this DLL statically links the CRT.
        HANDLE thread = CreateThread(nullptr, 0, &Worker, nullptr, 0, nullptr);
        if (thread) CloseHandle(thread);
    }
    return TRUE;
}
