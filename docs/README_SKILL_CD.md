# 全队技能 CD 显示

本功能基于 [ChiyukiKazama/Skill-CD-Display-Plugin-for-FufuLauncher](https://github.com/ChiyukiKazama/Skill-CD-Display-Plugin-for-FufuLauncher) 的 `e8f1a456` 源码（DisplayCD 1.0.3）。它是独立的 `DisplayCD.dll`，由 LiteUnlocker 启动器随 `SimpleUnlocker.dll` 注入，不依赖 FufuLauncher 主插件。

在 GUI 的“体验优化”中启用“全队技能 CD 显示”，并按需选择元素战技 E、元素爆发 Q、战技充能次数。总开关默认关闭，三个显示选项默认打开。GUI 把设置保存到运行目录的 `Plugins/SkillCdDisplayPlugin/config.ini`；插件约每 250 ms 重读一次。首次启用时安装 HUD Hook，关闭总开关后清除附加显示，但 Hook 保持到游戏退出，不热卸载 DLL。

插件只支持其源码验证过的国服和国际服 7.2.0 构建。它在安装 Hook 前校验进程名、PE 时间戳、映像大小和全部 38 个函数入口；不匹配时停止初始化，详见同目录的 `DisplayCD.log`。本机国服 EXE 的 PE 时间戳为 `0x6AD886FB`、映像大小为 `0x1B65F000`，与源码内的国服配置一致。用户已确认该构建中的技能 CD 显示可用；长期运行和其他构建仍需按实际环境验证。

本项目将上游工程改为本机可用的 MSVC v143 构建，增加默认关闭的 `[EnableSkillCD]` 总开关及 GUI 配置读写，并让两个注入模块的 Hook 安装使用同名进程内互斥量。上游源码与许可证保留在 `ThirdParty/DisplayCD/`，归属说明见[第三方声明](../THIRD_PARTY_NOTICES.md)。
