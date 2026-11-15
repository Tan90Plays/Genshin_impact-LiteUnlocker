# LiteUnlocker 1.1.1

## 新增功能

- 在“体验优化”中加入秘境内通过奖励物品来源跳转地图。默认关闭；仅在指定国服 7.2 游戏程序的 SHA-256 和函数入口匹配时安装 Hook。用户已确认该构建可用。
- 集成独立的 DisplayCD 1.0.3 插件，在队伍栏显示 E、Q 剩余冷却和战技充能次数。总开关默认关闭，三项显示可分别设置。用户已确认本机国服 7.2 构建可用。
- 帧率设置增加主挡位、可留空的备选挡位和游戏前台切换快捷键。`TargetFps` 保存当前目标值，切换时不改主、备选挡位。

## 适用范围

秘境地图仅针对 SHA-256 为 `8B014F64B5DC12FBC476E0C362B053ABFDD7A1378CD282E4AD81057621DBD6B5` 的国服 7.2 `YuanShen.exe`。技能 CD 插件在安装 Hook 前校验国服或国际服 7.2 的进程名、PE 指纹和函数入口；其他构建会停止初始化。帧率切换沿用 1.1.0 的 FPS Hook，快捷键仅在游戏窗口前台生效。

## 构建与源码

自包含版不依赖目标机安装 .NET；轻量版需要 .NET 8 Windows Desktop Runtime x64。两版均内嵌 `Launcher.dll`、`SimpleUnlocker.dll`、`DisplayCD.dll` 和插件默认配置。完整源码、第三方许可证及构建脚本随本次标签发布，见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
