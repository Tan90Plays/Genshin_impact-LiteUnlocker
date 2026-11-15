/*
Copyright (c) FufuLauncher Dev Team. All rights reserved.
Licensed under the AGPL-3.0 License.
*/
#pragma once

namespace SkillCooldown
{
    bool IsSupportedExecutable(const wchar_t* executableName);
    bool Init(const wchar_t* executableName);
}
