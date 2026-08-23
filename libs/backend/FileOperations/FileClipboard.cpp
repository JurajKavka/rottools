#include "FileClipboard.h"

namespace fs = std::filesystem;

namespace {

std::error_code InvalidArgument() {
    return std::make_error_code(std::errc::invalid_argument);
}

}  // namespace

FileClipboardSelectionResult FileClipboard::SelectForCopy(const fs::path& path) {
    return Select(path, FileClipboardMode::Copy);
}

FileClipboardSelectionResult FileClipboard::SelectForMove(const fs::path& path) {
    return Select(path, FileClipboardMode::Move);
}

FileClipboardSelectionResult FileClipboard::Select(const fs::path& path, FileClipboardMode mode) {
    if (path.empty() || path.filename().empty()) {
        return {.selectedPath = path, .error = InvalidArgument()};
    }
    m_mode = mode;
    m_path = path;
    return {.selectedPath = m_path};
}

FileClipboardPasteRequestResult FileClipboard::CreatePasteRequest(const fs::path& destinationDirectory) const {
    if (!HasSelection() || destinationDirectory.empty()) {
        return {.error = InvalidArgument()};
    }

    const FileOperationKind operation =
        *m_mode == FileClipboardMode::Copy ? FileOperationKind::Copy : FileOperationKind::Move;
    return {
        .request =
            FileOperationRequest{
                .kind = operation,
                .source = m_path,
                .destination = destinationDirectory / m_path.filename(),
            },
    };
}

void FileClipboard::HandleCompleted(const FileOperationResult& result) {
    if (!result.Succeeded() || m_mode != FileClipboardMode::Move || result.request.kind != FileOperationKind::Move) {
        return;
    }
    if (result.request.source.lexically_normal() == m_path.lexically_normal()) {
        Clear();
    }
}

void FileClipboard::Clear() noexcept {
    m_mode.reset();
    m_path.clear();
}

bool FileClipboard::HasSelection() const noexcept {
    return m_mode.has_value() && !m_path.empty();
}

std::optional<FileClipboardMode> FileClipboard::GetMode() const noexcept {
    return m_mode;
}

const fs::path& FileClipboard::GetPath() const noexcept {
    return m_path;
}
