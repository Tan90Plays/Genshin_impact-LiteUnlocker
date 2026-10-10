/*
Copyright (c) FufuLauncher Dev Team. All rights reserved.
Licensed under the AGPL-3.0 License.
*/
#pragma once

namespace Patterns {
    inline constexpr const char* CookHandler = "41 56 56 57 55 53 48 83 EC 20 48 89 D3 49 89 CE 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 48 8B 05 ? ? ? ? 0F 85 ? ? ? ? 48 8B 90 ? ? ? ? 48 85 D2 0F 84";
    inline constexpr const char* CookPathB = "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 89 C3 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 89 C6";
    inline constexpr const char* CookFireWrite = "89 86 ? ? 00 00 89 8E ? ? 00 00 4C 89";
    inline constexpr const char* CookEntityVal = "48 85 DB 0F 84";
    inline constexpr const char* CookBplSkip = "40 84 ED 75";
    inline constexpr const char* CookNullChk = "48 85 C0 0F 84";
    inline constexpr const char* CookNullTgt1 = "48 8B 86 ? ? 00 00";
    inline constexpr const char* CookNullTgt2 = "48 85 DB";
}
