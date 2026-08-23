#include <copyfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>

#include "NativeFileCopy.h"
#include "NativeFileMove.h"

namespace fs = std::filesystem;

namespace {

class FileDescriptor final {
   public:
    explicit FileDescriptor(int descriptor = -1) : m_descriptor(descriptor) {}
    ~FileDescriptor() {
        if (m_descriptor >= 0) {
            close(m_descriptor);
        }
    }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    [[nodiscard]] int Get() const noexcept {
        return m_descriptor;
    }

   private:
    int m_descriptor;
};

NativeFileIdentity IdentityFromStat(const struct stat& info) {
    return {
        .volumeId = static_cast<std::uint64_t>(info.st_dev),
        .fileId = static_cast<std::uint64_t>(info.st_ino),
        .size = static_cast<std::uint64_t>(info.st_size),
        .modificationStamp = static_cast<std::uint64_t>(info.st_mtimespec.tv_sec) * 1'000'000'000ULL +
                             static_cast<std::uint64_t>(info.st_mtimespec.tv_nsec),
        .changeStamp = static_cast<std::uint64_t>(info.st_ctimespec.tv_sec) * 1'000'000'000ULL +
                       static_cast<std::uint64_t>(info.st_ctimespec.tv_nsec),
        .valid = true,
    };
}

struct CopyContext {
    CopyContext(const NativeFileCopyRequest& requestValue, const FileOperationCancellation& cancellationValue,
                const NativeFileCopy::ProgressCallback& handleProgressValue, std::uintmax_t totalBytesValue)
        : request(requestValue),
          cancellation(cancellationValue),
          handleProgress(handleProgressValue),
          totalBytes(totalBytesValue) {}

    const NativeFileCopyRequest& request;
    const FileOperationCancellation& cancellation;
    const NativeFileCopy::ProgressCallback& handleProgress;
    std::uintmax_t totalBytes;
};

NativeFileCopyResult CompletedResult(const NativeFileCopyRequest& request, const NativeFileIdentity& sourceIdentity) {
    return {
        .request = request,
        .outcome = NativeFileCopyOutcome::Completed,
        .sourceIdentity = sourceIdentity,
    };
}

NativeFileCopyResult CancelledResult(const NativeFileCopyRequest& request) {
    return {
        .request = request,
        .outcome = NativeFileCopyOutcome::Cancelled,
        .error = std::make_error_code(std::errc::operation_canceled),
    };
}

NativeFileCopyResult FailedResult(const NativeFileCopyRequest& request, std::error_code error) {
    return {.request = request, .outcome = NativeFileCopyOutcome::Failed, .error = error};
}

void RemovePartialDestination(const fs::path& destination, const NativeFileIdentity& expectedIdentity) {
    static_cast<void>(NativeFileMove::RemoveIfEntryMatches(destination, expectedIdentity));
}

int HandleCopyfileStatus(int what, int stage, copyfile_state_t state, const char*, const char*, void* rawContext) {
    // Signature and opaque context are imposed by copyfile(3); mutation is
    // confined to this native callback adapter.
    auto& context = *static_cast<CopyContext*>(rawContext);
    if (context.cancellation.IsCancelled()) {
        return COPYFILE_QUIT;
    }
    if (what != COPYFILE_COPY_DATA) {
        return COPYFILE_CONTINUE;
    }
    if (stage == COPYFILE_ERR) {
        return COPYFILE_QUIT;
    }
    if (stage != COPYFILE_PROGRESS || !context.handleProgress) {
        return COPYFILE_CONTINUE;
    }

    off_t copied = 0;
    if (copyfile_state_get(state, COPYFILE_STATE_COPIED, &copied) == 0) {
        try {
            context.handleProgress({
                .source = context.request.source,
                .destination = context.request.destination,
                .bytesCopied = std::min<std::uintmax_t>(static_cast<std::uintmax_t>(copied), context.totalBytes),
                .totalBytes = context.totalBytes,
            });
        } catch (...) {
            // Observer failures must never cross the C callback boundary.
        }
    }
    return COPYFILE_CONTINUE;
}

}  // namespace

NativeFileCopyResult NativeFileCopy::Copy(const NativeFileCopyRequest& request,
                                          const FileOperationCancellation& cancellation,
                                          const ProgressCallback& handleProgress) {
    if (cancellation.IsCancelled()) {
        return CancelledResult(request);
    }

    FileDescriptor source(open(request.source.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (source.Get() < 0) {
        return FailedResult(request, std::error_code(errno, std::generic_category()));
    }

    struct stat sourceInfo{};
    if (fstat(source.Get(), &sourceInfo) < 0) {
        return FailedResult(request, std::error_code(errno, std::generic_category()));
    }
    if (!S_ISREG(sourceInfo.st_mode)) {
        return FailedResult(request, std::make_error_code(std::errc::operation_not_supported));
    }
    const NativeFileIdentity sourceIdentity = IdentityFromStat(sourceInfo);
    const std::uintmax_t totalBytes = static_cast<std::uintmax_t>(sourceInfo.st_size);

    FileDescriptor destination(open(request.destination.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW,
                                    sourceInfo.st_mode & 07777));
    if (destination.Get() < 0) {
        return FailedResult(request, std::error_code(errno, std::generic_category()));
    }
    struct stat destinationInfo{};
    if (fstat(destination.Get(), &destinationInfo) < 0) {
        return FailedResult(request, std::error_code(errno, std::generic_category()));
    }
    const NativeFileIdentity destinationIdentity = IdentityFromStat(destinationInfo);

    if (handleProgress) {
        try {
            handleProgress({
                .source = request.source,
                .destination = request.destination,
                .totalBytes = totalBytes,
            });
        } catch (...) {
        }
    }

    copyfile_state_t state = copyfile_state_alloc();
    if (state == nullptr) {
        RemovePartialDestination(request.destination, destinationIdentity);
        return FailedResult(request, std::error_code(ENOMEM, std::generic_category()));
    }

    CopyContext context(request, cancellation, handleProgress, totalBytes);
    const copyfile_callback_t callback = HandleCopyfileStatus;
    const int callbackResult =
        copyfile_state_set(state, COPYFILE_STATE_STATUS_CB, reinterpret_cast<const void*>(callback));
    const int contextResult = copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, &context);
    if (callbackResult < 0 || contextResult < 0) {
        const int savedError = errno;
        copyfile_state_free(state);
        RemovePartialDestination(request.destination, destinationIdentity);
        return FailedResult(request, std::error_code(savedError == 0 ? EIO : savedError, std::generic_category()));
    }

    errno = 0;
    const int copyResult = fcopyfile(source.Get(), destination.Get(), state, COPYFILE_ALL);
    const int savedError = errno;
    copyfile_state_free(state);

    if (copyResult == 0) {
        struct stat completedSourceInfo{};
        if (fstat(source.Get(), &completedSourceInfo) < 0) {
            const int sourceError = errno;
            RemovePartialDestination(request.destination, destinationIdentity);
            return FailedResult(request, std::error_code(sourceError, std::generic_category()));
        }
        if (!sourceIdentity.Matches(IdentityFromStat(completedSourceInfo))) {
            RemovePartialDestination(request.destination, destinationIdentity);
            return FailedResult(request, std::error_code(EBUSY, std::generic_category()));
        }
        if (handleProgress) {
            try {
                handleProgress({
                    .source = request.source,
                    .destination = request.destination,
                    .bytesCopied = totalBytes,
                    .totalBytes = totalBytes,
                });
            } catch (...) {
            }
        }
        return CompletedResult(request, sourceIdentity);
    }

    if (cancellation.IsCancelled()) {
        RemovePartialDestination(request.destination, destinationIdentity);
        return CancelledResult(request);
    }
    RemovePartialDestination(request.destination, destinationIdentity);
    return FailedResult(request, std::error_code(savedError == 0 ? EIO : savedError, std::generic_category()));
}
