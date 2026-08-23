#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <system_error>

enum class FileOperationKind { CreateFile, Copy, Move, Trash };

/**
 * One filesystem operation.
 *
 * source is the existing entry for Copy, Move, and Trash. For CreateFile it is
 * the path to create. destination is required only for Copy and Move.
 */
struct FileOperationRequest {
    FileOperationKind kind = FileOperationKind::Copy;
    std::filesystem::path source;
    std::optional<std::filesystem::path> destination;
};

struct FileOperationProgress {
    FileOperationRequest request;
    std::uintmax_t bytesCompleted = 0;
    std::uintmax_t totalBytes = 0;
};

enum class FileOperationOutcome { Completed, Cancelled, Failed };

struct FileOperationResult {
    FileOperationRequest request;
    FileOperationOutcome outcome = FileOperationOutcome::Failed;
    std::error_code error;
    /** An entry left behind because the operation could not finish or clean up. */
    std::optional<std::filesystem::path> residualPath;

    [[nodiscard]] bool Succeeded() const noexcept {
        return outcome == FileOperationOutcome::Completed;
    }
};

/**
 * Thread-safe cancellation shared by the caller and native operation.
 *
 * Regular-file copies and cross-volume moves are cancellable while bytes are
 * being copied. Atomic moves, file creation, and some platform trash APIs can
 * only observe cancellation before the native operation begins.
 */
class FileOperationCancellation final {
   public:
    void Cancel() noexcept {
        m_cancelled = true;
    }

    [[nodiscard]] bool IsCancelled() const noexcept {
        return m_cancelled;
    }

   private:
    std::atomic<bool> m_cancelled{false};
};

/**
 * Synchronous, UI-independent filesystem operations.
 *
 * This is the single public execution path. Native platform implementations
 * are private details. Existing destinations are never overwritten. Trash has
 * no permanent-delete fallback.
 */
class FileOperations final {
   public:
    using ProgressCallback = std::function<void(const FileOperationProgress&)>;

    [[nodiscard]] static FileOperationResult Execute(const FileOperationRequest& request,
                                                     const FileOperationCancellation& cancellation,
                                                     const ProgressCallback& handleProgress);
};
