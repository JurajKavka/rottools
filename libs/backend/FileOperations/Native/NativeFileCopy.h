#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <system_error>

#include "../FileOperations.h"
#include "NativeFileIdentity.h"

struct NativeFileCopyRequest {
    std::filesystem::path source;
    std::filesystem::path destination;
};

struct NativeFileCopyProgress {
    std::filesystem::path source;
    std::filesystem::path destination;
    std::uintmax_t bytesCopied = 0;
    std::uintmax_t totalBytes = 0;
};

enum class NativeFileCopyOutcome { Completed, Cancelled, Failed };

struct NativeFileCopyResult {
    NativeFileCopyRequest request;
    NativeFileCopyOutcome outcome = NativeFileCopyOutcome::Failed;
    std::error_code error;
    NativeFileIdentity sourceIdentity;

    [[nodiscard]] bool Succeeded() const noexcept {
        return outcome == NativeFileCopyOutcome::Completed;
    }
};

/**
 * Copies one regular file with the platform's native transfer primitive.
 *
 * The call is synchronous; FileOperationTask provides the worker-thread layer.
 * Existing destinations are never overwritten. Progress callbacks run on the
 * calling thread and receive immutable snapshots.
 */
class NativeFileCopy final {
   public:
    using ProgressCallback = std::function<void(const NativeFileCopyProgress&)>;

    [[nodiscard]] static NativeFileCopyResult Copy(const NativeFileCopyRequest& request,
                                                   const FileOperationCancellation& cancellation,
                                                   const ProgressCallback& handleProgress);
};
