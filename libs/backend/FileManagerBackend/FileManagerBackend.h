#pragma once

#include <filesystem>
#include <system_error>
#include <vector>

/** Synchronous filesystem actions for callers such as FileBrowserTreePanel.
 *
 * Names must be a single, non-empty path component. Existing destinations are
 * never replaced. Copy and move preserve symlinks rather than following them.
 * Moves use same-volume atomic renames; a cross-volume move fails with its
 * source intact. Delete is permanent and can partially succeed on an I/O
 * failure. Bulk actions process each item independently and return results in
 * input order; they are not an all-or-nothing transaction.
 */
namespace FileManagerBackend {
namespace fs = std::filesystem;

struct Result {
    fs::path source;
    fs::path destination;  // Empty for create and delete actions.
    std::error_code error;
    // A copy/create may leave a private staging directory if cleanup fails.
    // Check this separately even when Succeeded() is true.
    fs::path leftover;
    std::error_code cleanupError;

    [[nodiscard]] bool Succeeded() const {
        return !error;
    }
};

[[nodiscard]] Result CreateNewDirectory(const fs::path& parent, const fs::path& name);
[[nodiscard]] Result CreateTextFile(const fs::path& parent, const fs::path& name);
[[nodiscard]] Result Rename(const fs::path& source, const fs::path& newName);
[[nodiscard]] Result Copy(const fs::path& source, const fs::path& destination);
[[nodiscard]] Result Move(const fs::path& source, const fs::path& destination);
[[nodiscard]] Result Delete(const fs::path& source);

// destinationDirectory must exist. Each source keeps its existing leaf name.
[[nodiscard]] std::vector<Result> CopyMany(const std::vector<fs::path>& sources, const fs::path& destinationDirectory);
[[nodiscard]] std::vector<Result> MoveMany(const std::vector<fs::path>& sources, const fs::path& destinationDirectory);
[[nodiscard]] std::vector<Result> DeleteMany(const std::vector<fs::path>& sources);
}  // namespace FileManagerBackend
