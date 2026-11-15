# 秘境内奖励来源跳转地图

FufuLauncher [1.7.2.2 更新公告](https://github.com/FufuLauncher/FufuLauncher/releases/tag/1.7.2.2)新增了“秘境内通过奖励物品来源跳转地图”。本实现改编自其 [DomainMap 源码](https://github.com/FufuLauncher/FufuLauncher.UnlockerIsland/tree/master/FufuLauncher.UnlockerIsland/DomainMap)，并接入 LiteUnlocker 的“体验优化”页面。

- 在 GUI 的“体验优化”中勾选“秘境内通过奖励来源跳转地图”，即可写入 `[EnableDomainMap] Value=1`。默认关闭，运行中切换会热重载。
- 只有当前游戏主程序 SHA-256 为 `8B014F64B5DC12FBC476E0C362B053ABFDD7A1378CD282E4AD81057621DBD6B5`，且来源跳转、类型检查、调用者和模块引用都匹配上游国服 7.2 特征时，才安装两个 Hook。失配时保持游戏原生限制。
- 只在奖励物品来源触发的调用链中，且秘境场景处于稳定状态时放行地图跳转；其他入口维持原结果。关闭后恢复目标函数入口。
- GUI 的特征码诊断中查看 `DomainMap`。显示“active”表示 Hook 已安装；显示 mismatch 或 unsupported 时不会启用。

当前只针对上述国服 7.2 EXE；用户已确认该构建中的功能可用。国际服和其他 7.2 构建尚未适配。

同一版更新公告中的“全队技能 CD 显示”是单独的第三方插件。其[独立源码仓库](https://github.com/ChiyukiKazama/Skill-CD-Display-Plugin-for-FufuLauncher)现已接入 LiteUnlocker，见[功能说明](./README_SKILL_CD.md)。
