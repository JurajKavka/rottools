#pragma once

#include <vector>

#include <wx/filename.h>

namespace CutFilesClipboard {
bool Set(const std::vector<wxFileName>& paths);
bool HasFiles();
std::vector<wxFileName> GetFiles();
void KeepFiles(std::vector<wxFileName> paths);
}  // namespace CutFilesClipboard
