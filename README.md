# LiteUnlocker

> 本项目基于原始 [wannabeabetterman/LiteUnlocker](https://github.com/wannabeabetterman/LiteUnlocker) 二次开发。随身烹饪及部分注入 / Hook 实现进一步衍生自 [FufuLauncher.UnlockerIsland](https://github.com/FufuLauncher/FufuLauncher.UnlockerIsland)；完整许可与第三方声明见 [LICENSE](LICENSE) 和 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

这是一个以本地体验调整为目标的原 LiteUnlocker 衍生版本，提供：

- **FPS 解锁**：解除游戏帧率上限，目标值可调。
- **FOV 修改**：修改摄像机视场角，支持平滑过渡和特殊镜头保护。
- **移除队伍切换动画**：跳过角色展示，直接打开队伍页面。
- **移除大招动画**：跳过 EventCamera 事件镜头移动。
- **隐藏 UID 水印**：仅隐藏本地画面中的 UID 水印。
- **关闭场景雾效 / 角色半透明**：本地视觉效果调整。
- **随身合成台**：可设置独立热键，并可在原合成台入口处直接打开界面。
- **随身烹饪**：可设置独立热键打开烹饪界面；依赖游戏特征码及临时内存补丁，风险较高。
- **UnlockerGUI**：深色 WinForms 启动器，自动保存配置并内嵌原生 DLL。

与原 LiteUnlocker / FufuLauncher 的完整功能集相比，本项目**未包含** FreeCam、派蒙、隐藏 UI、伤害数字及原 FufuLauncher 的联网授权、遥测等功能；但**保留并扩展了**随身合成台（默认热键可设为 F12）与新增的随身烹饪功能。

---

## ⚠️ 风险声明

1. 修改或注入游戏客户端可能违反游戏服务条款，并存在**封号**或**其他风险**。
2. 特征码、Hook 和临时内存补丁会随游戏更新失效，也可能导致崩溃或异常行为。
3. 本项目仅供学习、研究和本地测试；请自行承担使用风险，**不要**用于正式账号或使用某些功能于公开视频、直播等。

---

## 当前目录结构

```text
LiteUnlocker_src/
├── LICENSE                         # AGPL-3.0 完整文本
├── THIRD_PARTY_NOTICES.md          # 第三方组件与来源说明
├── README.md
├── SimpleUnlocker.sln              # Visual Studio 解决方案
├── Launcher/                       # 启动与 DLL 注入器
│   ├── Launcher.cpp / Launcher.h
│   └── Launcher.vcxproj
├── Plugin/                         # 注入到游戏进程的功能 DLL
│   ├── Config.cpp / Config.h       # config.ini 读取与热重载配置
│   ├── Hooks.cpp / Hooks.h         # FPS、FOV、UI、合成等 Hook
│   ├── Cooking.cpp / Cooking.h     # 随身烹饪实现（Fufu 衍生）
│   ├── CookingPatterns.h           # 烹饪相关特征码
│   ├── Scanner.cpp / Scanner.h     # 特征码扫描器
│   ├── dllmain.cpp                 # DLL 入口与热键监听
│   ├── config.ini                  # 默认配置示例
│   ├── SimpleUnlocker.vcxproj
│   └── MinHook/                    # MinHook 头文件与 x64 静态库
├── UnlockerGUI/                    # C# WinForms 启动器
│   ├── Program.cs
│   ├── UnlockerGUI.csproj
│   └── app.manifest
└── exec/Plugins/config.ini          # 传统运行目录的配置示例
```

构建输出 `build/`、`UnlockerGUI/bin/`、`UnlockerGUI/obj/` 均为可再生成文件，已被 Git 忽略，不属于源码。

## 编译

### 环境要求

- Visual Studio 2022，安装“使用 C++ 的桌面开发”（MSVC v143 + Windows 10/11 SDK）。
- .NET 8 SDK（编译 UnlockerGUI）。

### 一次性构建全部组件

在“x64 Native Tools Command Prompt for VS 2022”中、从仓库根目录执行：

```cmd
msbuild SimpleUnlocker.sln /restore /p:Configuration=Release /p:Platform=x64
```

产物为：

- `build/Launcher.dll`
- `build/SimpleUnlocker.dll`
- `UnlockerGUI/bin/x64/Release/net8.0-windows/win-x64/UnlockerGUI.exe`

### 发布 GUI

`UnlockerGUI` 会在构建时内嵌 `build/Launcher.dll` 和 `build/SimpleUnlocker.dll`。首次运行 GUI 时，会自动将它们释放到：

```text
%LocalAppData%\LiteUnlocker\runtime\
```

因此终端用户只需运行 `UnlockerGUI.exe`，不需要手动复制 `Plugins` 目录或 DLL。

- 轻量发布版：需要 .NET 8 Windows Desktop Runtime x64。
- 自包含发布版：不依赖目标机的 .NET 运行时，但文件约 163 MB。

当前公开的两种版本均见 [GitHub Releases](https://github.com/Tan90Plays/Genshin_impact-LiteUnlocker/releases)。

## 使用方式

1. 启动 `UnlockerGUI.exe`，选择游戏可执行文件。
2. 按需启用功能并点击启动；GUI 会将设置写入运行时目录内的 `Plugins/config.ini`。
3. 随身合成与随身烹饪仅在游戏窗口前台时监听热键，热键请求会转发到游戏主线程执行，并使用 500ms 防抖。
4. 首次使用随身烹饪后，可使用 GUI 的特征码诊断确认 `AutoCook` 是否处于可用状态。

## 配置说明

| Section | 含义 | 示例值 |
|---|---|---:|
| `[FpsUnlock]` | 是否解锁帧率 | 1 |
| `[TargetFps]` | 目标帧率（GUI 可设至 2000） | 120 |
| `[VSync]` | 关闭垂直同步 | 1 |
| `[FovUnlock]` | 是否修改 FOV | 1 |
| `[FovValue]` | 目标 FOV | 60.0 |
| `[FovTransitionSpeed]` | FOV 平滑过渡速度 | 0.05 |
| `[FovLimitCheck]` | 仅在 FOV > 30 时覆盖 | 1 |
| `[RemoveTeamAnim]` | 移除队伍切换动画 | 1 |
| `[DisableUltimateAnim]` | 移除大招 / 事件镜头动画 | 0 |
| `[HideUID]` | 隐藏本地 UID 水印 | 1 |
| `[DisableFog]` | 关闭场景雾效 | 0 |
| `[DisableCharFade]` | 关闭角色穿模半透明 | 0 |
| `[RedirectCraft]` | 启用随身合成台 | 0 |
| `[CraftKey]` | 随身合成台热键（123 = F12） | 123 |
| `[AutoCook]` | 启用随身烹饪 | 0 |
| `[AutoCookKey]` | 随身烹饪热键（121 = F10） | 121 |
| `[DebugConsole]` | 显示调试控制台 | 0 |

修改运行时 `config.ini` 后，注入插件会自动热重载；不过特征码扫描结果需要重新启动游戏进程后才会重新生成。

## 技术概览

1. `Launcher.dll` 以挂起态启动游戏，并通过 `CreateRemoteThread + LoadLibraryW` 注入插件。
2. `Hooks.cpp` 在游戏主循环内处理 FPS、FOV、视觉和页面请求，以避免从热键监听线程直接操作游戏 UI。
3. 随身合成台使用独立请求标志与 `CraftPartner` / `CraftEntry` 路径打开合成界面。
4. 随身烹饪使用来自 FufuLauncher 的页面 Hook、特征码与临时补丁；游戏版本更新后可能需要同步更新 `CookingPatterns.h`。

## 已知限制

- 所有特征码都与特定游戏版本相关；更新后功能可能不可用，诊断窗口会显示扫描结果。
- 随身合成台与随身烹饪均涉及游戏交互，风险显著高于纯 FPS/FOV 配置。
- GUI 不会保证绕过反作弊检测，也不保证功能在所有系统或游戏版本中稳定工作。

## 许可证与第三方声明

本项目修改自原始 [wannabeabetterman/LiteUnlocker](https://github.com/wannabeabetterman/LiteUnlocker)，并包含基于 [FufuLauncher.UnlockerIsland](https://github.com/FufuLauncher/FufuLauncher.UnlockerIsland) 的衍生实现。因此，本项目及其组合衍生工作按 **GNU Affero General Public License v3.0（AGPL-3.0）** 发布；完整文本见 [LICENSE](LICENSE)。

- `Plugin/Cooking.cpp`、`Plugin/Cooking.h` 与 `Plugin/CookingPatterns.h` 基于 FufuLauncher 的烹饪自动化实现修改，并保留原始版权声明。
- `Plugin/Hooks.cpp`、`Plugin/dllmain.cpp` 与部分特征码 / 注入逻辑，修改自原 LiteUnlocker 和 FufuLauncher 的相应实现。
- `Plugin/MinHook` 使用 [MinHook](https://github.com/TsudaKageyu/minhook)，该第三方组件继续适用其 BSD 2-Clause License。

发布、分发或修改本项目时，请保留上述版权与许可信息，并依 AGPL-3.0 提供对应完整源码。

## 致谢

- 原始基础项目：[wannabeabetterman/LiteUnlocker](https://github.com/wannabeabetterman/LiteUnlocker)。本仓库在其基础上进行功能精简、GUI、配置、诊断、随身合成与随身烹饪等修改。
- [FufuLauncher.UnlockerIsland](https://github.com/FufuLauncher/FufuLauncher.UnlockerIsland)：随身烹饪及部分 Hook / 注入实现的上游来源。
- [Snap.Hutao.Remastered.UnlockerIsland](https://github.com/SnapHutaoRemasteringProject/Snap.Hutao.Remastered.UnlockerIsland)：相关解锁实现的参考来源。
