/*
Copyright (c) FufuLauncher Dev Team. All rights reserved.
Licensed under the AGPL-3.0 License.

This file exposes the cooking portion of FufuLauncher.UnlockerIsland's
Automation module for LiteUnlocker.
*/
#pragma once

#include <Windows.h>
#include <cstdint>

namespace HelperField {
    constexpr uint32_t CookCtxV35 = 0x20;
    constexpr uint32_t CookCtxV2 = 0x10;
    constexpr uint32_t CookFireStateDef = 0x248;
    constexpr uint32_t CookFireParamDef = 0x250;
    constexpr uint32_t CookHookMagic1 = 0x3F800000;
}

namespace HelperAddr {
    extern uintptr_t CookHandler;
    extern uintptr_t CookShowPage;
    extern uintptr_t CookPatchEntity;
    extern uintptr_t CookPatchPathB;
    extern uintptr_t CookPatchBplSkip;
    extern uintptr_t CookPatchNullChk1;
    extern uintptr_t CookPatchNullChk2;
    extern uintptr_t CookPatchNullTgt1;
    extern uintptr_t CookPatchNullTgt2;
    extern uintptr_t CookPatchFireWr;
}

using Fn_CookShowPage = void(__fastcall*)(__int64);
using Fn_CookHandler = __int64(__fastcall*)(__int64, __int64);

extern Fn_CookShowPage g_oCookShowPage;
extern BYTE g_CookHandlerPrologue[8];
extern bool g_CookReady;

namespace Cooking {
    void Init();
    void OpenCookingPage();
    bool IsReady();
}
