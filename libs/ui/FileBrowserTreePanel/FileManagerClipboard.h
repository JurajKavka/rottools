#pragma once

#include <wx/filename.h>

#include <optional>
#include <vector>

/** Process-local file clipboard, invalidated when the system clipboard changes. */
namespace FileManagerClipboard {
enum class Operation { Cut, Copy };

struct Contents {
    std::vector<wxFileName> files;
    Operation operation = Operation::Copy;
};

bool Set(const std::vector<wxFileName>& paths, Operation operation);
bool HasFiles();
std::optional<Contents> GetFiles();
/** Retain failed or skipped cut items after a move. Copy contents remain available for repeated pastes. */
void KeepFiles(std::vector<wxFileName> paths);
}  // namespace FileManagerClipboard
