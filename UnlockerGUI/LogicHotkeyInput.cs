using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace UnlockerGUI;

internal static class LogicHotkeyInput
{
    private static readonly HashSet<string> NamedKeys = new(StringComparer.OrdinalIgnoreCase)
    {
        "space", "enter", "tab", "esc", "backspace", "delete", "insert", "home", "end",
        "page_up", "page_down", "up", "down", "left", "right", "caps_lock", "pause",
        "print_screen", "scroll_lock", "mouse_left", "mouse_right", "mouse_middle",
        "mouse_x1", "mouse_x2", "numpad_add", "numpad_subtract", "numpad_multiply",
        "numpad_divide", "numpad_decimal"
    };

    public static bool TryNormalize(string input, out string normalized)
    {
        normalized = "";
        if (string.IsNullOrWhiteSpace(input)) return true;
        string[] parts = input.Trim().ToLowerInvariant().Split('+');
        if (parts.Length == 0 || parts.Length > 5) return false;
        bool ctrl = false, alt = false, shift = false, win = false;
        for (int i = 0; i < parts.Length - 1; i++)
        {
            switch (parts[i].Trim())
            {
                case "ctrl": case "control": if (ctrl) return false; ctrl = true; break;
                case "alt": if (alt) return false; alt = true; break;
                case "shift": if (shift) return false; shift = true; break;
                case "win": case "windows": case "cmd": if (win) return false; win = true; break;
                default: return false;
            }
        }

        string key = parts[^1].Trim();
        key = key switch
        {
            "return" => "enter", "escape" => "esc", "del" => "delete", "ins" => "insert",
            "pgup" => "page_up", "pgdn" => "page_down", _ => key
        };
        if (!IsPrimaryKey(key)) return false;
        var tokens = new List<string>();
        if (ctrl) tokens.Add("ctrl");
        if (alt) tokens.Add("alt");
        if (shift) tokens.Add("shift");
        if (win) tokens.Add("win");
        tokens.Add(key);
        normalized = string.Join('+', tokens);
        return true;
    }

    private static bool IsPrimaryKey(string key)
    {
        if (NamedKeys.Contains(key)) return true;
        if (key.Length == 1 && (key[0] is >= 'a' and <= 'z' or >= '0' and <= '9')) return true;
        if (key.Length > 1 && key[0] == 'f' && int.TryParse(key[1..], out int f) && f is >= 1 and <= 24) return true;
        if (key.Length == 7 && key.StartsWith("numpad", StringComparison.Ordinal) && key[6] is >= '0' and <= '9') return true;
        if (key.StartsWith("vk:0x", StringComparison.Ordinal) &&
            int.TryParse(key[5..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out int vk) &&
            vk is >= 1 and <= 254 &&
            vk is not (0x10 or 0x11 or 0x12 or 0x5B or 0x5C or 0xA0 or 0xA1 or 0xA2 or 0xA3 or 0xA4 or 0xA5))
            return true;
        return false;
    }

    public static string? Capture(IWin32Window owner, string title = "录制模拟逻辑帧快捷键")
    {
        using var dialog = new Form
        {
            Text = title,
            ClientSize = new Size(390, 130),
            StartPosition = FormStartPosition.CenterParent,
            FormBorderStyle = FormBorderStyle.FixedDialog,
            MaximizeBox = false,
            MinimizeBox = false,
            KeyPreview = true,
            Font = new Font("Microsoft YaHei UI", 9F)
        };
        var hint = new Label
        {
            Text = "按下按键或组合键；也可点击空白处录制鼠标键。\n系统保留的组合键可在主窗口手动输入。",
            Left = 16, Top = 18, Width = 360, Height = 58
        };
        var cancel = new Button { Text = "取消", Left = 290, Top = 88, Width = 82, Height = 28, DialogResult = DialogResult.Cancel };
        dialog.Controls.AddRange(new Control[] { hint, cancel });
        dialog.CancelButton = cancel;
        string? result = null;
        void Accept(string key)
        {
            string chord = BuildChord(key);
            if (!TryNormalize(chord, out string canonical)) return;
            result = canonical;
            dialog.DialogResult = DialogResult.OK;
            dialog.Close();
        }
        dialog.KeyDown += (_, e) =>
        {
            e.SuppressKeyPress = true;
            if (e.KeyCode is Keys.ControlKey or Keys.LControlKey or Keys.RControlKey or
                Keys.Menu or Keys.LMenu or Keys.RMenu or Keys.ShiftKey or Keys.LShiftKey or
                Keys.RShiftKey or Keys.LWin or Keys.RWin) return;
            Accept(KeyName(e.KeyCode));
        };
        void OnMouseDown(object? _, MouseEventArgs e)
        {
            string? key = e.Button switch
            {
                MouseButtons.Left => "mouse_left", MouseButtons.Right => "mouse_right",
                MouseButtons.Middle => "mouse_middle", MouseButtons.XButton1 => "mouse_x1",
                MouseButtons.XButton2 => "mouse_x2", _ => null
            };
            if (key != null) Accept(key);
        }
        dialog.MouseDown += OnMouseDown;
        hint.MouseDown += OnMouseDown;
        dialog.ShowDialog(owner);
        return result;
    }

    private static string BuildChord(string key)
    {
        var parts = new List<string>();
        Keys modifiers = Control.ModifierKeys;
        if ((modifiers & Keys.Control) != 0) parts.Add("ctrl");
        if ((modifiers & Keys.Alt) != 0) parts.Add("alt");
        if ((modifiers & Keys.Shift) != 0) parts.Add("shift");
        if ((GetAsyncKeyState(0x5B) & 0x8000) != 0 || (GetAsyncKeyState(0x5C) & 0x8000) != 0) parts.Add("win");
        parts.Add(key);
        return string.Join('+', parts);
    }

    private static string KeyName(Keys key)
    {
        int code = (int)(key & Keys.KeyCode);
        if (code is >= 0x30 and <= 0x39) return ((char)code).ToString();
        if (code is >= 0x41 and <= 0x5A) return ((char)code).ToString().ToLowerInvariant();
        if (code is >= 0x70 and <= 0x87) return $"f{code - 0x6F}";
        if (code is >= 0x60 and <= 0x69) return $"numpad{code - 0x60}";
        return code switch
        {
            0x20 => "space", 0x0D => "enter", 0x09 => "tab", 0x1B => "esc",
            0x08 => "backspace", 0x2E => "delete", 0x2D => "insert", 0x24 => "home",
            0x23 => "end", 0x21 => "page_up", 0x22 => "page_down", 0x26 => "up",
            0x28 => "down", 0x25 => "left", 0x27 => "right", 0x14 => "caps_lock",
            0x13 => "pause", 0x2C => "print_screen", 0x91 => "scroll_lock",
            0x6B => "numpad_add", 0x6D => "numpad_subtract", 0x6A => "numpad_multiply",
            0x6F => "numpad_divide", 0x6E => "numpad_decimal", _ => $"vk:0x{code:X2}"
        };
    }

    [DllImport("user32.dll")]
    private static extern short GetAsyncKeyState(int virtualKey);
}
