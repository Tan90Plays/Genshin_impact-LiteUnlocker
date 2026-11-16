#pragma once

namespace StygianHpDisplay {
// Installs CN 7.2-only hooks on first enable. Later configuration changes are
// applied without replacing the already validated hook targets.
bool SetEnabled(bool enabled, bool showMaximum);
const char* DiagnosticNote();
}
