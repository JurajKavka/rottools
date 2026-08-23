#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <vector>

#include "NativeFileCopy.h"
#include "NativeFileMove.h"

namespace fs = std::filesystem;

namespace {

constexpr std::size_t kCopyChunkSize = 8 * 1024 * 1024;
constexpr std::size_t kFallbackBufferSize = 1024 * 1024;

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
        .modificationStamp = static_cast<std::uint64_t>(info.st_mtim.tv_sec) * 1'000'000'000ULL +
                             static_cast<std::uint64_t>(info.st_mtim.tv_nsec),
        .changeStamp = static_cast<std::uint64_t>(info.st_ctim.tv_sec) * 1'000'000'000ULL +
                       static_cast<std::uint64_t>(info.st_ctim.tv_nsec),
        .valid = true,
    };
}

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

NativeFileCopyResult FailedResult(const NativeFileCopyRequest& request, int error) {
    return {
        .request = request,
        .outcome = NativeFileCopyOutcome::Failed,
        .error = std::error_code(error, std::generic_category()),
    };
}

void RemovePartialDestination(const fs::path& destination, const NativeFileIdentity& expectedIdentity) {
    static_cast<void>(NativeFileMove::RemoveIfEntryMatches(destination, expectedIdentity));
}

void ReportProgress(const NativeFileCopyRequest& request, std::uintmax_t bytesCopied, std::uintmax_t totalBytes,
                    const NativeFileCopy::ProgressCallback& handleProgress) {
    if (handleProgress) {
        try {
            handleProgress({
                .source = request.source,
                .destination = request.destination,
                .bytesCopied = bytesCopied,
                .totalBytes = totalBytes,
            });
        } catch (...) {
            // Progress is observational and must not interrupt the transfer.
        }
    }
}

int CopyWithReadWrite(const FileDescriptor& source, const FileDescriptor& destination,
                      const NativeFileCopyRequest& request, const FileOperationCancellation& cancellation,
                      const NativeFileCopy::ProgressCallback& handleProgress, std::uintmax_t totalBytes,
                      std::uintmax_t bytesAlreadyCopied) {
    std::vector<char> buffer(kFallbackBufferSize);
    std::uintmax_t bytesCopied = bytesAlreadyCopied;
    while (bytesCopied < totalBytes) {
        if (cancellation.IsCancelled()) {
            return ECANCELED;
        }

        const std::size_t requestedBytes =
            static_cast<std::size_t>(std::min<std::uintmax_t>(buffer.size(), totalBytes - bytesCopied));
        const ssize_t bytesRead = read(source.Get(), buffer.data(), requestedBytes);
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            return errno;
        }
        if (bytesRead == 0) {
            return EIO;
        }

        ssize_t bytesWritten = 0;
        while (bytesWritten < bytesRead) {
            const ssize_t writeResult = write(destination.Get(), buffer.data() + bytesWritten,
                                              static_cast<std::size_t>(bytesRead - bytesWritten));
            if (writeResult < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return errno;
            }
            bytesWritten += writeResult;
        }

        bytesCopied += static_cast<std::uintmax_t>(bytesRead);
        ReportProgress(request, bytesCopied, totalBytes, handleProgress);
    }
    return 0;
}

bool RequiresReadWriteFallback(int error) {
    return error == ENOSYS || error == EOPNOTSUPP || error == EXDEV || error == EINVAL;
}

}  // namespace

NativeFileCopyResult NativeFileCopy::Copy(const NativeFileCopyRequest& request,
                                          const FileOperationCancellation& cancellation,
                                          const ProgressCallback& handleProgress) {
    if (cancellation.IsCancelled()) {
        return CancelledResult(request);
    }

    struct stat sourceLinkInfo{};
    if (lstat(request.source.c_str(), &sourceLinkInfo) < 0) {
        return FailedResult(request, errno);
    }
    if (!S_ISREG(sourceLinkInfo.st_mode)) {
        return FailedResult(request, EINVAL);
    }

    FileDescriptor source(open(request.source.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (source.Get() < 0) {
        return FailedResult(request, errno);
    }

    struct stat sourceInfo{};
    if (fstat(source.Get(), &sourceInfo) < 0) {
        return FailedResult(request, errno);
    }
    if (!S_ISREG(sourceInfo.st_mode)) {
        return FailedResult(request, EINVAL);
    }
    const NativeFileIdentity sourceIdentity = IdentityFromStat(sourceInfo);

    FileDescriptor destination(open(request.destination.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW,
                                    sourceInfo.st_mode & 07777));
    if (destination.Get() < 0) {
        return FailedResult(request, errno);
    }
    struct stat destinationInfo{};
    if (fstat(destination.Get(), &destinationInfo) < 0) {
        return FailedResult(request, errno);
    }
    const NativeFileIdentity destinationIdentity = IdentityFromStat(destinationInfo);

    const std::uintmax_t totalBytes = static_cast<std::uintmax_t>(sourceInfo.st_size);
    std::uintmax_t bytesCopied = 0;
    ReportProgress(request, bytesCopied, totalBytes, handleProgress);

    int copyError = 0;
    while (bytesCopied < totalBytes) {
        if (cancellation.IsCancelled()) {
            copyError = ECANCELED;
            break;
        }

        const std::size_t requestedBytes =
            static_cast<std::size_t>(std::min<std::uintmax_t>(kCopyChunkSize, totalBytes - bytesCopied));
        const ssize_t copyResult =
            copy_file_range(source.Get(), nullptr, destination.Get(), nullptr, requestedBytes, 0);
        if (copyResult < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (RequiresReadWriteFallback(errno)) {
                copyError = CopyWithReadWrite(source, destination, request, cancellation, handleProgress, totalBytes,
                                              bytesCopied);
            } else {
                copyError = errno;
            }
            break;
        }
        if (copyResult == 0) {
            copyError = EIO;
            break;
        }

        bytesCopied += static_cast<std::uintmax_t>(copyResult);
        ReportProgress(request, bytesCopied, totalBytes, handleProgress);
    }

    if (copyError == 0) {
        const struct timespec times[2] = {sourceInfo.st_atim, sourceInfo.st_mtim};
        if (fchmod(destination.Get(), sourceInfo.st_mode & 07777) < 0 || futimens(destination.Get(), times) < 0) {
            copyError = errno;
        }
    }

    if (copyError != 0) {
        RemovePartialDestination(request.destination, destinationIdentity);
        return copyError == ECANCELED ? CancelledResult(request) : FailedResult(request, copyError);
    }

    struct stat completedSourceInfo{};
    if (fstat(source.Get(), &completedSourceInfo) < 0) {
        const int sourceError = errno;
        RemovePartialDestination(request.destination, destinationIdentity);
        return FailedResult(request, sourceError);
    }
    if (!sourceIdentity.Matches(IdentityFromStat(completedSourceInfo))) {
        RemovePartialDestination(request.destination, destinationIdentity);
        return FailedResult(request, EBUSY);
    }

    ReportProgress(request, totalBytes, totalBytes, handleProgress);
    return CompletedResult(request, sourceIdentity);
}
