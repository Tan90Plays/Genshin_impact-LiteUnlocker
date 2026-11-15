#pragma once
#include <cstdint>
#include <cstddef>

namespace Patterns
{
    struct Method { uintptr_t rva; const char* bytes; };
    struct SkillCooldownLayout
    {
        size_t TeamRows;
        size_t SkillStates;
        size_t AvatarGuid;
        uintptr_t AvatarEntityType;
        size_t VTable;
    };
    struct SkillCooldownProfile
    {
        const char* name;
        const wchar_t* executableName;
        uint32_t Timestamp;
        uint32_t ImageSize;
        uintptr_t ChargeNamePrefix;
        uintptr_t ChargeImageType;
        SkillCooldownLayout layout;
        Method StringNew;
        Method SetupCharges;
        Method GameObjectTransform;
        Method FindTransform;
        Method ComponentByType;
        Method ComponentByName;
        Method SetMaskFill;
        Method ConcatStrings;
        Method SetColor;
        Method BindTeam;
        Method UpdateTeam;
        Method SetupRow;
        Method UpdateRow;
        Method DisableRow;
        Method DestroyRow;
        Method ResolveSkill;
        Method DecodeFloat;
        Method DecodeInt;
        Method CloneText;
        Method DestroyObject;
        Method GetTransform;
        Method GetGameObject;
        Method GetEnabled;
        Method GetActiveInHierarchy;
        Method SetEnabled;
        Method GetRect;
        Method SetPosition;
        Method TransformPoint;
        Method InverseTransformPoint;
        Method SetScale;
        Method SetSize;
        Method SetAnchorMin;
        Method SetAnchorMax;
        Method SetPivot;
        Method SetText;
        Method SetAlignment;
        Method SetRaycast;
        Method SetActive;
    };

    // Match the executable name and PE fingerprint, then verify all 38
    // entries before installing any hooks or reading managed fields.
    // dump72.cs plus the local 7.2.0 YuanShen.exe; native icalls and global
    // references were resolved from that binary, not copied from 7.1 wrappers.
    inline constexpr SkillCooldownProfile SkillCooldownCN72{
        "CN 7.2", L"YuanShen.exe", 0x6AD886FB, 0x1B65F000, 0x5B26080, 0x5887350,
        {0x420, 0x240, 0x560, 0x5898D98, 0xD0},
        {0x19C21C90, "56 48 83 EC 20 48 85 C9 74 17 48 89 CE E8 3E 4B 09 E8 48 89 F1 89 C2 48 83 C4 20 5E E9 3F DF 90"}, // StringNew
        {0x1197E680, "41 57 41 56 56 57 55 53 48 83 EC 28 41 89 D7 48 89 CF 80 3D 8B 07 ED F3 00 0F 85 19 02 00 00 48"}, // SetupCharges
        {0x1463E80, "40 53 48 83 EC 20 48 8B D9 48 85 C9 0F 84 85 00 00 00 E8 C9 7F FF FF 48 85 C0 74 7B 48 8B 40 40"}, // GameObjectTransform
        {0x1448CE0, "48 89 5C 24 18 57 48 83 EC 50 48 8B D9 48 89 54 24 60 48 8D 54 24 20 48 8D 4C 24 60 E8 AF A3 08"}, // FindTransform
        {0x1464E20, "40 53 48 83 EC 20 48 8B DA E8 32 70 FF FF 48 85 C0 74 1F 48 8B 50 40 48 85 D2 74 16 4C 8B C3 48"}, // ComponentByType
        {0x1463870, "40 53 48 83 EC 50 48 8B D9 48 89 54 24 60 48 8D 54 24 20 48 8D 4C 24 60 E8 23 F8 06 00 48 85 DB"}, // ComponentByName
        {0x1A335B40, "56 48 83 EC 20 48 89 CE 48 83 C1 50 45 31 C0 45 31 C9 E8 49 62 5B FA 84 C0 74 07 C6 86 30 01 00"}, // SetMaskFill
        {0x19A8D050, "55 41 57 41 56 41 54 56 57 53 48 83 EC 30 48 8D 6C 24 30 48 C7 45 F8 FE FF FF FF 48 89 D6 48 85"}, // ConcatStrings
        {0x1A310410, "56 57 48 83 EC 48 48 89 CE 0F 10 02 F3 0F 10 49 24 0F 2E C8 75 38 7A 36 F3 0F 10 4E 28 F3 0F 16"}, // SetColor
        {0x8A2DB60, "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 38 4D 89 CC 44 89 C5 48 89 D6 49 89 CD 0F B6 9C 24"}, // BindTeam
        {0x8761C80, "56 57 48 83 EC 28 48 89 CE 80 3D 6C DB 0A FD 00 0F 85 B7 02 00 00 80 3D 06 3A 07 FD 00 0F 85 D6"}, // UpdateTeam
        {0xE76F2B0, "56 57 53 48 83 EC 20 49 89 D0 48 89 CE 80 3D DB 1C 08 F7 00 0F 85 A2 00 00 00 4C 89 86 78 01 00"}, // SetupRow
        {0xE772240, "56 48 83 EC 20 48 89 CE 80 3D 85 ED 07 F7 00 75 3E 48 89 F1 E8 B7 FC FF FF 48 89 F1 E8 BF D3 FF"}, // UpdateRow
        {0xE76FC30, "56 57 48 83 EC 28 48 89 CE 80 3D 62 13 08 F7 00 0F 85 BF 00 00 00 80 3D 4D 13 08 F7 00 0F 85 DE"}, // DisableRow
        {0xE771230, "56 57 48 83 EC 28 48 89 CE 80 3D 79 FD 07 F7 00 75 6F 48 8B 46 48 48 85 C0 74 47 48 83 78 10 00"}, // DestroyRow
        {0x13451A30, "56 57 48 83 EC 28 89 D6 48 89 CF 80 3D 87 10 75 F2 00 74 47 80 3D FC 1D 3D F2 00 75 58 48 8B BF"}, // ResolveSkill
        {0x12E90A60, "56 53 48 83 EC 38 49 89 C8 48 8B 19 48 85 DB 75 0B 49 83 78 08 00 0F 84 20 03 00 00 C7 44 24 28"}, // DecodeFloat
        {0x8304740, "56 53 48 83 EC 38 49 89 C8 48 8B 19 48 85 DB 75 0B 49 83 78 08 00 0F 84 10 03 00 00 C7 44 24 28"}, // DecodeInt
        {0x1A227020, "56 57 48 83 EC 28 48 85 D2 74 17 48 83 7A 10 00 74 10 48 85 C9 74 17 48 83 C4 28 5F 5E E9 7E 24"}, // CloneText
        {0x1A226970, "0F 57 C9 E9 08 2D 22 E7 0F 1F 84 00 00 00 00 00 E9 2B 2D 22 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // DestroyObject
        {0x1A230450, "E9 9B 48 23 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 8B 48 23 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetTransform
        {0x1A230470, "E9 CB 48 23 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 2B 49 23 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetGameObject
        {0x1A2008B0, "E9 5B DA EE E6 66 66 2E 0F 1F 84 00 00 00 00 00 E9 8B DA EE E6 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetEnabled
        {0x1A20F460, "E9 2B 4B 25 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 8B 4B 25 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetActiveInHierarchy
        {0x1A2008C0, "E9 8B DA EE E6 66 66 2E 0F 1F 84 00 00 00 00 00 E9 6B 58 26 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetEnabled
        {0x1A2271F0, "56 48 83 EC 20 48 89 CE 0F 57 C0 0F 11 01 48 89 D1 48 89 F2 E8 87 DA 22 E7 48 89 F0 48 83 C4 20"}, // GetRect
        {0x1A229EB0, "E9 CB DC 21 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 BB DC 21 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetPosition
        {0x1A22BFE0, "56 48 83 EC 20 48 89 CE C7 41 08 00 00 00 00 48 C7 01 00 00 00 00 48 89 D1 4C 89 C2 49 89 F0 E8"}, // TransformPoint
        {0x1A22C0C0, "56 48 83 EC 20 48 89 CE C7 41 08 00 00 00 00 48 C7 01 00 00 00 00 48 89 D1 4C 89 C2 49 89 F0 E8"}, // InverseTransformPoint
        {0x1A22AAC0, "E9 7B D5 21 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 6B D5 21 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetScale
        {0x1A2274E0, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 2D DB 22 E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetSize
        {0x1A227270, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 AD DA 22 E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetAnchorMin
        {0x1A2272E0, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 2D DB 22 E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetAnchorMax
        {0x1A227550, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 AD DB 22 E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetPivot
        {0x11CC2FD0, "56 57 53 48 83 EC 20 48 89 D7 48 89 CE 48 8D 99 18 01 00 00 48 85 D2 74 14 83 7F 10 00 48 8B 8E"}, // SetText
        {0x11CC3270, "39 91 F0 00 00 00 75 01 C3 89 91 F0 00 00 00 E9 1C DA FF FF 66 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetAlignment
        {0x1A310570, "88 51 34 C3 66 66 66 2E 0F 1F 84 00 00 00 00 00 0F B6 41 50 C3 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetRaycast
        {0x1463FC0, "48 89 5C 24 08 57 48 83 EC 20 0F B6 FA 48 8B D9 48 85 C9 74 22 E8 86 7E FF FF 48 85 C0 74 18 40"}, // SetActive
    };

    // International 7.2.0: verified against the local os72.exe sample.
    inline constexpr SkillCooldownProfile SkillCooldownOS72{
        "OS 7.2", L"GenshinImpact.exe", 0x6AD8992C, 0x1B690000, 0x5B26A40, 0x58872F8,
        {0x348, 0x280, 0x558, 0x5898D30, 0xD0},
        {0x19C54AD0, "56 48 83 EC 20 48 85 C9 74 17 48 89 CE E8 FE 2C 06 E8 48 89 F1 89 C2 48 83 C4 20 5E E9 7F BF 8D"}, // StringNew
        {0x11988CB0, "41 57 41 56 56 57 55 53 48 83 EC 28 41 89 D7 48 89 CF 80 3D 71 ED E8 F3 00 0F 85 1A 02 00 00 48"}, // SetupCharges
        {0x1464E80, "40 53 48 83 EC 20 48 8B D9 48 85 C9 0F 84 85 00 00 00 E8 C9 7F FF FF 48 85 C0 74 7B 48 8B 40 40"}, // GameObjectTransform
        {0x1449CE0, "48 89 5C 24 18 57 48 83 EC 50 48 8B D9 48 89 54 24 60 48 8D 54 24 20 48 8D 4C 24 60 E8 AF A3 08"}, // FindTransform
        {0x1465E20, "40 53 48 83 EC 20 48 8B DA E8 32 70 FF FF 48 85 C0 74 1F 48 8B 50 40 48 85 D2 74 16 4C 8B C3 48"}, // ComponentByType
        {0x1464870, "40 53 48 83 EC 50 48 8B D9 48 89 54 24 60 48 8D 54 24 20 48 8D 4C 24 60 E8 23 F8 06 00 48 85 DB"}, // ComponentByName
        {0x1A367C80, "56 48 83 EC 20 48 89 CE 48 83 C1 50 45 31 C0 45 31 C9 E8 89 3A 59 FA 84 C0 74 07 C6 86 30 01 00"}, // SetMaskFill
        {0x19AC0000, "55 41 57 41 56 41 54 56 57 53 48 83 EC 30 48 8D 6C 24 30 48 C7 45 F8 FE FF FF FF 48 89 D6 48 85"}, // ConcatStrings
        {0x1A3424D0, "56 57 48 83 EC 48 48 89 CE 0F 10 02 F3 0F 10 49 24 0F 2E C8 75 38 7A 36 F3 0F 10 4E 28 F3 0F 16"}, // SetColor
        {0x8761900, "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 38 4D 89 CC 44 89 C5 48 89 D6 49 89 CD 0F B6 9C 24"}, // BindTeam
        {0x8A5C300, "56 57 48 83 EC 28 48 89 CE 80 3D 96 99 DC FC 00 0F 85 B7 02 00 00 80 3D D7 A6 D9 FC 00 0F 85 D6"}, // UpdateTeam
        {0xE7781E0, "56 57 53 48 83 EC 20 49 89 D0 48 89 CE 80 3D F5 CE 09 F7 00 0F 85 A2 00 00 00 4C 89 86 78 01 00"}, // SetupRow
        {0xE775230, "56 48 83 EC 20 48 89 CE 80 3D 9A FE 09 F7 00 75 3E 48 89 F1 E8 67 00 00 00 48 89 F1 E8 4F 01 00"}, // UpdateRow
        {0xE7791F0, "56 57 48 83 EC 28 48 89 CE 80 3D F4 BE 09 F7 00 0F 85 BF 00 00 00 80 3D F7 BE 09 F7 00 0F 85 DE"}, // DisableRow
        {0xE77A6C0, "56 57 48 83 EC 28 48 89 CE 80 3D 3B AA 09 F7 00 75 6F 48 8B 46 48 48 85 C0 74 47 48 83 78 10 00"}, // DestroyRow
        {0x1345F680, "56 57 48 83 EC 28 89 D6 48 89 CF 80 3D 39 28 74 F2 00 74 47 80 3D A0 21 3B F2 00 75 58 48 8B BF"}, // ResolveSkill
        {0x12E9DE40, "56 53 48 83 EC 38 49 89 C8 48 8B 19 48 85 DB 75 0B 49 83 78 08 00 0F 84 20 03 00 00 C7 44 24 28"}, // DecodeFloat
        {0x8307300, "56 53 48 83 EC 38 49 89 C8 48 8B 19 48 85 DB 75 0B 49 83 78 08 00 0F 84 10 03 00 00 C7 44 24 28"}, // DecodeInt
        {0x1A2592C0, "56 57 48 83 EC 28 48 85 D2 74 17 48 83 7A 10 00 74 10 48 85 C9 74 17 48 83 C4 28 5F 5E E9 DE 11"}, // CloneText
        {0x1A258C10, "0F 57 C9 E9 68 1A 1F E7 0F 1F 84 00 00 00 00 00 E9 8B 1A 1F E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // DestroyObject
        {0x1A262690, "E9 5B 36 20 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 4B 36 20 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetTransform
        {0x1A2626B0, "E9 8B 36 20 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 EB 36 20 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetGameObject
        {0x1A232B90, "E9 7B C7 EB E6 66 66 2E 0F 1F 84 00 00 00 00 00 E9 AB C7 EB E6 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetEnabled
        {0x1A241740, "E9 4B 38 22 E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 DB 38 22 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // GetActiveInHierarchy
        {0x1A232BA0, "E9 AB C7 EB E6 66 66 2E 0F 1F 84 00 00 00 00 00 E9 8B 45 23 E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetEnabled
        {0x1A259490, "56 48 83 EC 20 48 89 CE 0F 57 C0 0F 11 01 48 89 D1 48 89 F2 E8 E7 C7 1F E7 48 89 F0 48 83 C4 20"}, // GetRect
        {0x1A25C150, "E9 2B CA 1E E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 1B CA 1E E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetPosition
        {0x1A25E280, "56 48 83 EC 20 48 89 CE C7 41 08 00 00 00 00 48 C7 01 00 00 00 00 48 89 D1 4C 89 C2 49 89 F0 E8"}, // TransformPoint
        {0x1A25E360, "56 48 83 EC 20 48 89 CE C7 41 08 00 00 00 00 48 C7 01 00 00 00 00 48 89 D1 4C 89 C2 49 89 F0 E8"}, // InverseTransformPoint
        {0x1A25CD60, "E9 DB C2 1E E7 66 66 2E 0F 1F 84 00 00 00 00 00 E9 CB C2 1E E7 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetScale
        {0x1A259780, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 8D C8 1F E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetSize
        {0x1A259510, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 0D C8 1F E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetAnchorMin
        {0x1A259580, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 8D C8 1F E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetAnchorMax
        {0x1A2597F0, "48 83 EC 28 48 89 54 24 20 48 8D 54 24 20 E8 0D C9 1F E7 90 48 83 C4 28 C3 0F 1F 80 00 00 00 00"}, // SetPivot
        {0x11CCCDA0, "56 57 53 48 83 EC 20 48 89 D7 48 89 CE 48 8D 99 18 01 00 00 48 85 D2 74 14 83 7F 10 00 48 8B 8E"}, // SetText
        {0x11CCECF0, "39 91 F0 00 00 00 75 01 C3 89 91 F0 00 00 00 E9 2C E2 FF FF 66 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetAlignment
        {0x1A342630, "88 51 34 C3 66 66 66 2E 0F 1F 84 00 00 00 00 00 0F B6 41 50 C3 66 66 2E 0F 1F 84 00 00 00 00 00"}, // SetRaycast
        {0x1464FC0, "48 89 5C 24 08 57 48 83 EC 20 0F B6 FA 48 8B D9 48 85 C9 74 22 E8 86 7E FF FF 48 85 C0 74 18 40"}, // SetActive
    };

    inline constexpr const SkillCooldownProfile* SkillCooldownProfiles[]{
        &SkillCooldownCN72, &SkillCooldownOS72
    };
}
