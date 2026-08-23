#include "FileOperations.h"

#include <atomic>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "Native/NativeFileCopy.h"
#include "Native/NativeFileCreate.h"
#include "Native/NativeFileMove.h"
#include "Native/NativeFileTrash.h"

namespace fs = std::filesystem;

namespace {

std::error_code MakeError(std::errc error) {
    return std::make_error_code(error);
}

struct PathResult {
    fs::path path;
    std::error_code error;
};

struct EntryStatusResult {
    fs::file_status status;
    std::error_code error;

    [[nodiscard]] bool IsMissing() const noexcept {
        return status.type() == fs::file_type::not_found || error == MakeError(std::errc::no_such_file_or_directory);
    }

    [[nodiscard]] bool HasError() const noexcept {
        return error && !IsMissing();
    }

    [[nodiscard]] bool Exists() const noexcept {
        return !HasError() && !IsMissing() && status.type() != fs::file_type::none;
    }
};

struct NormalizedRequestResult {
    FileOperationRequest request;
    std::error_code error;
};

PathResult AbsolutePath(const fs::path& path) {
    if (path.empty()) {
        return {.error = MakeError(std::errc::invalid_argument)};
    }

    std::error_code error;
    fs::path absolute = fs::absolute(path, error);
    if (error) {
        return {.error = error};
    }
    return {.path = absolute.lexically_normal()};
}

EntryStatusResult EntryStatus(const fs::path& path) {
    std::error_code error;
    const fs::file_status status = fs::symlink_status(path, error);
    return {.status = status, .error = error};
}

bool IsRootPath(const fs::path& path) {
    return path.has_root_path() && path == path.root_path();
}

NormalizedRequestResult NormalizeRequest(const FileOperationRequest& input) {
    FileOperationRequest request = input;
    const PathResult source = AbsolutePath(input.source);
    if (source.error) {
        return {.request = input, .error = source.error};
    }
    request.source = source.path;

    const bool needsDestination = input.kind == FileOperationKind::Copy || input.kind == FileOperationKind::Move;
    if (needsDestination != input.destination.has_value()) {
        return {.request = std::move(request), .error = MakeError(std::errc::invalid_argument)};
    }
    if (input.destination) {
        const PathResult destination = AbsolutePath(*input.destination);
        if (destination.error) {
            return {.request = std::move(request), .error = destination.error};
        }
        request.destination = destination.path;
    }
    return {.request = std::move(request)};
}

FileOperationResult CompletedResult(FileOperationRequest request) {
    return {
        .request = std::move(request),
        .outcome = FileOperationOutcome::Completed,
    };
}

FileOperationResult CancelledResult(FileOperationRequest request, std::optional<fs::path> residualPath = std::nullopt) {
    return {
        .request = std::move(request),
        .outcome = FileOperationOutcome::Cancelled,
        .error = MakeError(std::errc::operation_canceled),
        .residualPath = std::move(residualPath),
    };
}

FileOperationResult FailedResult(FileOperationRequest request, std::error_code error,
                                 std::optional<fs::path> residualPath = std::nullopt) {
    return {
        .request = std::move(request),
        .outcome = FileOperationOutcome::Failed,
        .error = error ? error : MakeError(std::errc::io_error),
        .residualPath = std::move(residualPath),
    };
}

std::error_code ValidateDestination(const fs::path& destination) {
    const EntryStatusResult destinationStatus = EntryStatus(destination);
    if (destinationStatus.HasError()) {
        return destinationStatus.error;
    }
    if (destinationStatus.Exists()) {
        return MakeError(std::errc::file_exists);
    }

    std::error_code parentError;
    if (!fs::is_directory(destination.parent_path(), parentError)) {
        return parentError ? parentError : MakeError(std::errc::not_a_directory);
    }
    return {};
}

enum class PlannedEntryKind { Directory, RegularFile, SymbolicLink };

struct PlannedEntry {
    fs::path source;
    fs::path relativePath;
    PlannedEntryKind kind = PlannedEntryKind::RegularFile;
    std::uintmax_t size = 0;
};

struct DirectoryCopyPlanResult {
    std::vector<PlannedEntry> entries;
    std::uintmax_t totalBytes = 0;
    std::error_code error;
};

struct PathRelationResult {
    bool sameOrNested = false;
    std::error_code error;
};

PathRelationResult IsSameOrNestedPath(const fs::path& possibleParent, const fs::path& possibleChild) {
    std::error_code parentError;
    std::error_code childError;
    const fs::path parent = fs::weakly_canonical(possibleParent, parentError);
    const fs::path child = fs::weakly_canonical(possibleChild, childError);
    if (parentError) {
        return {.error = parentError};
    }
    if (childError) {
        return {.error = childError};
    }

    auto parentPart = parent.begin();
    auto childPart = child.begin();
    for (; parentPart != parent.end() && childPart != child.end(); ++parentPart, ++childPart) {
        if (*parentPart != *childPart) {
            return {};
        }
    }
    return {.sameOrNested = parentPart == parent.end()};
}

DirectoryCopyPlanResult BuildDirectoryCopyPlan(const fs::path& source, const FileOperationCancellation& cancellation) {
    DirectoryCopyPlanResult result;
    std::error_code error;
    fs::recursive_directory_iterator entry(source, fs::directory_options::none, error);
    const fs::recursive_directory_iterator end;
    while (!error && entry != end) {
        if (cancellation.IsCancelled()) {
            result.error = MakeError(std::errc::operation_canceled);
            return result;
        }

        const fs::path entryPath = entry->path();
        const fs::path relativePath = entryPath.lexically_relative(source);
        if (relativePath.empty() || *relativePath.begin() == "..") {
            result.error = MakeError(std::errc::invalid_argument);
            return result;
        }

        const fs::file_status status = entry->symlink_status(error);
        if (error) {
            break;
        }

        PlannedEntry planned{.source = entryPath, .relativePath = relativePath};
        if (fs::is_directory(status)) {
            planned.kind = PlannedEntryKind::Directory;
        } else if (fs::is_regular_file(status)) {
            planned.kind = PlannedEntryKind::RegularFile;
            planned.size = entry->file_size(error);
            if (error) {
                break;
            }
            if (planned.size > std::numeric_limits<std::uintmax_t>::max() - result.totalBytes) {
                result.error = MakeError(std::errc::value_too_large);
                return result;
            }
            result.totalBytes += planned.size;
        } else if (fs::is_symlink(status)) {
            planned.kind = PlannedEntryKind::SymbolicLink;
        } else {
            result.error = MakeError(std::errc::operation_not_supported);
            return result;
        }
        result.entries.push_back(std::move(planned));
        entry.increment(error);
    }
    result.error = error;
    return result;
}

PathResult CreateTemporaryDirectory(const fs::path& destination) {
    static std::atomic<std::uint64_t> nextId{1};
    for (;;) {
        fs::path temporaryName = destination.filename();
        temporaryName += ".rotfm-copy-";
        temporaryName += std::to_string(nextId.fetch_add(1));
        const fs::path temporary = destination.parent_path() / temporaryName;

        std::error_code error;
        if (fs::create_directory(temporary, error)) {
            return {.path = temporary};
        }
        if (!error || error == MakeError(std::errc::file_exists)) {
            continue;
        }
        return {.error = error};
    }
}

std::optional<fs::path> TrashPartialDirectory(const fs::path& temporary) {
    const FileOperationCancellation cleanupCancellation;
    const NativeFileTrashResult cleanup = NativeFileTrash::Trash(temporary, cleanupCancellation);
    return cleanup.Succeeded() ? std::nullopt : std::optional<fs::path>{temporary};
}

void ReportProgress(const FileOperationRequest& request, std::uintmax_t bytesCompleted, std::uintmax_t totalBytes,
                    const FileOperations::ProgressCallback& handleProgress) {
    if (!handleProgress) {
        return;
    }
    try {
        handleProgress({
            .request = request,
            .bytesCompleted = bytesCompleted,
            .totalBytes = totalBytes,
        });
    } catch (...) {
        // Progress observers cannot change operation behavior.
    }
}

FileOperationResult CopyDirectory(FileOperationRequest request, const FileOperationCancellation& cancellation,
                                  const FileOperations::ProgressCallback& handleProgress) {
    const PathRelationResult relation = IsSameOrNestedPath(request.source, *request.destination);
    if (relation.error) {
        return FailedResult(std::move(request), relation.error);
    }
    if (relation.sameOrNested) {
        return FailedResult(std::move(request), MakeError(std::errc::invalid_argument));
    }

    const DirectoryCopyPlanResult plan = BuildDirectoryCopyPlan(request.source, cancellation);
    if (plan.error == MakeError(std::errc::operation_canceled)) {
        return CancelledResult(std::move(request));
    }
    if (plan.error) {
        return FailedResult(std::move(request), plan.error);
    }

    const PathResult temporary = CreateTemporaryDirectory(*request.destination);
    if (temporary.error) {
        return FailedResult(std::move(request), temporary.error);
    }

    std::uintmax_t completedBytes = 0;
    ReportProgress(request, completedBytes, plan.totalBytes, handleProgress);
    for (const PlannedEntry& entry : plan.entries) {
        if (cancellation.IsCancelled()) {
            return CancelledResult(std::move(request), TrashPartialDirectory(temporary.path));
        }

        const fs::path target = temporary.path / entry.relativePath;
        std::error_code error;
        if (entry.kind == PlannedEntryKind::Directory) {
            const bool created = fs::create_directory(target, error);
            if (!created && !error) {
                error = MakeError(std::errc::file_exists);
            }
        } else if (entry.kind == PlannedEntryKind::SymbolicLink) {
            fs::copy_symlink(entry.source, target, error);
        } else {
            const NativeFileCopyResult copyResult = NativeFileCopy::Copy(
                {.source = entry.source, .destination = target}, cancellation,
                [&](const NativeFileCopyProgress& progress) {
                    ReportProgress(request, completedBytes + progress.bytesCopied, plan.totalBytes, handleProgress);
                });
            if (copyResult.outcome == NativeFileCopyOutcome::Cancelled) {
                return CancelledResult(std::move(request), TrashPartialDirectory(temporary.path));
            }
            error = copyResult.error;
            if (copyResult.Succeeded()) {
                completedBytes += entry.size;
            }
        }

        if (error) {
            return FailedResult(std::move(request), error, TrashPartialDirectory(temporary.path));
        }
    }

    const NativeFileMoveResult publish = NativeFileMove::MoveNoReplace(temporary.path, *request.destination);
    if (publish.outcome != NativeFileMoveOutcome::Completed) {
        const std::error_code error = publish.error ? publish.error : MakeError(std::errc::cross_device_link);
        return FailedResult(std::move(request), error, TrashPartialDirectory(temporary.path));
    }
    ReportProgress(request, plan.totalBytes, plan.totalBytes, handleProgress);
    return CompletedResult(std::move(request));
}

FileOperationResult ExecuteCreate(FileOperationRequest request) {
    if (IsRootPath(request.source)) {
        return FailedResult(std::move(request), MakeError(std::errc::operation_not_permitted));
    }
    const std::error_code destinationError = ValidateDestination(request.source);
    if (destinationError) {
        return FailedResult(std::move(request), destinationError);
    }

    const NativeFileCreateResult result = NativeFileCreate::Create(request.source);
    return result.error ? FailedResult(std::move(request), result.error) : CompletedResult(std::move(request));
}

FileOperationResult ExecuteCopy(FileOperationRequest request, const EntryStatusResult& sourceStatus,
                                const FileOperationCancellation& cancellation,
                                const FileOperations::ProgressCallback& handleProgress) {
    const std::error_code destinationError = ValidateDestination(*request.destination);
    if (destinationError) {
        return FailedResult(std::move(request), destinationError);
    }

    if (fs::is_directory(sourceStatus.status)) {
        return CopyDirectory(std::move(request), cancellation, handleProgress);
    }
    if (fs::is_symlink(sourceStatus.status)) {
        if (cancellation.IsCancelled()) {
            return CancelledResult(std::move(request));
        }
        std::error_code error;
        fs::copy_symlink(request.source, *request.destination, error);
        return error ? FailedResult(std::move(request), error) : CompletedResult(std::move(request));
    }
    if (!fs::is_regular_file(sourceStatus.status)) {
        return FailedResult(std::move(request), MakeError(std::errc::operation_not_supported));
    }

    const NativeFileCopyResult copyResult =
        NativeFileCopy::Copy({.source = request.source, .destination = *request.destination}, cancellation,
                             [&](const NativeFileCopyProgress& progress) {
                                 ReportProgress(request, progress.bytesCopied, progress.totalBytes, handleProgress);
                             });
    if (copyResult.outcome == NativeFileCopyOutcome::Cancelled) {
        return CancelledResult(std::move(request));
    }
    return copyResult.Succeeded() ? CompletedResult(std::move(request))
                                  : FailedResult(std::move(request), copyResult.error);
}

FileOperationResult ExecuteMove(FileOperationRequest request, const EntryStatusResult& sourceStatus,
                                const FileOperationCancellation& cancellation,
                                const FileOperations::ProgressCallback& handleProgress) {
    const std::error_code destinationError = ValidateDestination(*request.destination);
    if (destinationError) {
        return FailedResult(std::move(request), destinationError);
    }

    const NativeFileMoveResult moveResult = NativeFileMove::MoveNoReplace(request.source, *request.destination);
    if (moveResult.outcome == NativeFileMoveOutcome::Completed) {
        return CompletedResult(std::move(request));
    }
    if (moveResult.outcome == NativeFileMoveOutcome::Failed) {
        return FailedResult(std::move(request), moveResult.error);
    }

    if (!fs::is_regular_file(sourceStatus.status)) {
        return FailedResult(std::move(request), MakeError(std::errc::operation_not_supported));
    }

    const NativeFileCopyResult copyResult =
        NativeFileCopy::Copy({.source = request.source, .destination = *request.destination}, cancellation,
                             [&](const NativeFileCopyProgress& progress) {
                                 ReportProgress(request, progress.bytesCopied, progress.totalBytes, handleProgress);
                             });
    if (copyResult.outcome == NativeFileCopyOutcome::Cancelled) {
        return CancelledResult(std::move(request));
    }
    if (!copyResult.Succeeded()) {
        return FailedResult(std::move(request), copyResult.error);
    }

    const fs::path copiedDestination = *request.destination;
    const NativeFileRemoveResult removeResult =
        NativeFileMove::RemoveIfIdentityMatches(request.source, copyResult.sourceIdentity);
    if (removeResult.outcome == NativeFileRemoveOutcome::IdentityMismatch) {
        return FailedResult(std::move(request), MakeError(std::errc::device_or_resource_busy), copiedDestination);
    }
    return removeResult.Succeeded() ? CompletedResult(std::move(request))
                                    : FailedResult(std::move(request), removeResult.error, copiedDestination);
}

FileOperationResult ExecuteTrash(FileOperationRequest request, const FileOperationCancellation& cancellation) {
    if (IsRootPath(request.source)) {
        return FailedResult(std::move(request), MakeError(std::errc::operation_not_permitted));
    }

    const NativeFileTrashResult trashResult = NativeFileTrash::Trash(request.source, cancellation);
    if (trashResult.outcome == NativeFileTrashOutcome::Cancelled) {
        return CancelledResult(std::move(request));
    }
    return trashResult.Succeeded() ? CompletedResult(std::move(request))
                                   : FailedResult(std::move(request), trashResult.error);
}

}  // namespace

FileOperationResult FileOperations::Execute(const FileOperationRequest& input,
                                            const FileOperationCancellation& cancellation,
                                            const ProgressCallback& handleProgress) {
    try {
        const NormalizedRequestResult normalized = NormalizeRequest(input);
        if (normalized.error) {
            return FailedResult(normalized.request, normalized.error);
        }
        FileOperationRequest request = normalized.request;

        if (cancellation.IsCancelled()) {
            return CancelledResult(std::move(request));
        }
        if (request.kind == FileOperationKind::CreateFile) {
            return ExecuteCreate(std::move(request));
        }

        const EntryStatusResult sourceStatus = EntryStatus(request.source);
        if (sourceStatus.HasError()) {
            return FailedResult(std::move(request), sourceStatus.error);
        }
        if (!sourceStatus.Exists()) {
            return FailedResult(std::move(request), MakeError(std::errc::no_such_file_or_directory));
        }

        switch (request.kind) {
            case FileOperationKind::Copy:
                return ExecuteCopy(std::move(request), sourceStatus, cancellation, handleProgress);
            case FileOperationKind::Move:
                return ExecuteMove(std::move(request), sourceStatus, cancellation, handleProgress);
            case FileOperationKind::Trash:
                return ExecuteTrash(std::move(request), cancellation);
            case FileOperationKind::CreateFile:
                break;
        }
        return FailedResult(std::move(request), MakeError(std::errc::invalid_argument));
    } catch (...) {
        return FailedResult(input, MakeError(std::errc::io_error));
    }
}
