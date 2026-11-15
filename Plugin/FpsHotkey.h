#pragma once
#include <string>

namespace FpsHotkey {
void Configure(const std::string& value);
bool Poll(bool gameFocused);
}