# LiteUnlocker 1.0.1

## 修复

- 修复游戏 7.0 更新后“隐藏 UID 水印”不生效的问题。
- 分别适配国服与国际服的新版 `SetActive` 地址。
- 增加地址有效性与函数头校验，版本失配时仅停用隐藏 UID，降低错误地址调用导致崩溃的风险。

## 说明

- 本次仅修复隐藏 UID 相关实现，保留 1.0.0 的 FPS、FOV、队伍动画、大招镜头、雾效、角色淡出、随身合成和随身烹饪功能。
- 注入与 Hook 可能违反游戏服务条款并带来账号风险，仅供学习、研究和本地测试。
- 实现参考原始 [LiteUnlocker](https://github.com/wannabeabetterman/LiteUnlocker) v1.4.0 与 [FufuLauncher.UnlockerIsland](https://github.com/FufuLauncher/FufuLauncher.UnlockerIsland) 当前公开源码，许可与归属详见仓库内 `LICENSE` 和 `THIRD_PARTY_NOTICES.md`。
