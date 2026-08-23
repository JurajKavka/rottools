#pragma once

#include <filesystem>
#include <system_error>

#include "../FileOperations.h"

enum class NativeFileTrashOutcome { Completed, Cancelled, Failed };

struct NativeFileTrashResult {
    NativeFileTrashOutcome outcome = NativeFileTrashOutcome::Failed;
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return outcome == NativeFileTrashOutcome::Completed;
    }
};

/** Moves one filesystem entry to the platform's recoverable system trash. */
class NativeFileTrash final {
   public:
    [[nodiscard]] static NativeFileTrashResult Trash(const std::filesystem::path& path,
                                                     const FileOperationCancellation& cancellation);
};
