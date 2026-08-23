#pragma once

#include <filesystem>
#include <optional>
#include <system_error>

#include "FileOperations.h"

enum class FileClipboardMode { Copy, Move };

struct FileClipboardSelectionResult {
    std::filesystem::path selectedPath;
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return !error;
    }
};

struct FileClipboardPasteRequestResult {
    std::optional<FileOperationRequest> request;
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return request.has_value() && !error;
    }
};

/**
 * In-memory file-manager clipboard for one selected entry.
 *
 * It performs no filesystem I/O. Paste only creates a request for
 * FileOperationTask; the completed result is passed back to clear a successful
 * move selection.
 */
class FileClipboard final {
   public:
    [[nodiscard]] FileClipboardSelectionResult SelectForCopy(const std::filesystem::path& path);
    [[nodiscard]] FileClipboardSelectionResult SelectForMove(const std::filesystem::path& path);
    [[nodiscard]] FileClipboardPasteRequestResult CreatePasteRequest(
        const std::filesystem::path& destinationDirectory) const;

    void HandleCompleted(const FileOperationResult& result);
    void Clear() noexcept;

    [[nodiscard]] bool HasSelection() const noexcept;
    [[nodiscard]] std::optional<FileClipboardMode> GetMode() const noexcept;
    [[nodiscard]] const std::filesystem::path& GetPath() const noexcept;

   private:
    std::optional<FileClipboardMode> m_mode;
    std::filesystem::path m_path;

    [[nodiscard]] FileClipboardSelectionResult Select(const std::filesystem::path& path, FileClipboardMode mode);
};
