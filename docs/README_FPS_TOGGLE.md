# 帧率备选挡位与切换快捷键

1.1.1 在 GUI 的「帧率设置」中提供主挡位、备选挡位和切换快捷键。主挡位沿用原目标 FPS，范围为 30–2000；备选挡位可留空，留空时写入 `0`。快捷键也可留空。只有勾选「解锁 FPS」、填写有效且不同于主挡位的备选值，并设置快捷键后，游戏窗口处于前台时按键才会在两个挡位之间切换。长按只触发一次，松开后可再次切换。

快捷键可手动输入或点击「录制」，例如 `ctrl+alt+f`、`shift+mouse_x1`。游戏运行时按键由插件处理；GUI 保持打开时，「当前」标签会同步显示切换后的目标 FPS。关闭 GUI 不会停用已设置的快捷键。

配置使用以下段落：

```ini
[FpsUnlock]
Value=1
[TargetFps]
Value=120
[FpsPrimary]
Value=120
[FpsAlternate]
Value=144
[FpsToggleHotkey]
Value=ctrl+alt+f
```

`TargetFps` 是当前生效值；按键切换时插件只改写这一项。`FpsPrimary` 与 `FpsAlternate` 保存两个挡位，旧配置未设置 `FpsPrimary` 时会沿用 `TargetFps`。备选值为 `0`、快捷键为空或 FPS 解锁关闭时，切换不执行。设置修改会通过现有配置热重载机制生效。

此功能包含在 1.1.1 发布版中。快捷键由注入的原生插件监听；GUI 退出后仍可使用。
