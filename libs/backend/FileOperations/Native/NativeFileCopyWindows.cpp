#include <windows.h>

#include "NativeFileCopy.h"
#include "NativeFileMove.h"

namespace {

class FileHandle final {
   public:
    explicit FileHandle(HANDLE handle = INVALID_HANDLE_VALUE) : m_handle(handle) {}
    ~FileHandle() {
        if (m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle);
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    [[nodiscard]] HANDLE Get() const noexcept {
        return m_handle;
    }

   private:
    HANDLE m_handle;
};

NativeFileIdentity IdentityFromInfo(const BY_HANDLE_FILE_INFORMATION& info) {
    ULARGE_INTEGER size{};
    size.LowPart = info.nFileSizeLow;
    size.HighPart = info.nFileSizeHigh;
    ULARGE_INTEGER writeTime{};
    writeTime.LowPart = info.ftLastWriteTime.dwLowDateTime;
    writeTime.HighPart = info.ftLastWriteTime.dwHighDateTime;
    return {
        .volumeId = info.dwVolumeSerialNumber,
        .fileId = (static_cast<std::uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow,
        .size = size.QuadPart,
        .modificationStamp = writeTime.QuadPart,
        .valid = true,
    };
}

struct CopyContext {
    CopyContext(const NativeFileCopyRequest& requestValue, const FileOperationCancellation& cancellationValue,
                const NativeFileCopy::ProgressCallback& handleProgressValue)
        : request(requestValue), cancellation(cancellationValue), handleProgress(handleProgressValue) {}

    const NativeFileCopyRequest& request;
    const FileOperationCancellation& cancellation;
    const NativeFileCopy::ProgressCallback& handleProgress;
    NativeFileIdentity destinationIdentity;
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
    return {
        .request = request,
        .outcome = NativeFileCopyOutcome::Failed,
        .error = error,
    };
}

DWORD CALLBACK HandleCopyProgress(LARGE_INTEGER totalFileSize, LARGE_INTEGER totalBytesTransferred, LARGE_INTEGER,
                                  LARGE_INTEGER, DWORD, DWORD, HANDLE, HANDLE destinationFile, LPVOID rawContext) {
    // Signature and opaque context are imposed by CopyFileExW; all native
    // callback adaptation stays at this boundary.
    auto& context = *static_cast<CopyContext*>(rawContext);
    if (context.cancellation.IsCancelled()) {
        return PROGRESS_CANCEL;
    }
    if (context.handleProgress) {
        try {
            context.handleProgress({
                .source = context.request.source,
                .destination = context.request.destination,
                .bytesCopied = static_cast<std::uintmax_t>(totalBytesTransferred.QuadPart),
                .totalBytes = static_cast<std::uintmax_t>(totalFileSize.QuadPart),
            });
        } catch (...) {
            // Observer failures must never cross the Win32 callback boundary.
        }
    }
    if (!context.destinationIdentity.valid) {
        BY_HANDLE_FILE_INFORMATION destinationInfo{};
        if (GetFileInformationByHandle(destinationFile, &destinationInfo) != FALSE) {
            context.destinationIdentity = IdentityFromInfo(destinationInfo);
        }
    }
    return PROGRESS_CONTINUE;
}

void RemovePartialDestination(const std::filesystem::path& destination, const NativeFileIdentity& expectedIdentity) {
    if (expectedIdentity.valid) {
        static_cast<void>(NativeFileMove::RemoveIfEntryMatches(destination, expectedIdentity));
        return;
    }
    const NativeFileIdentityResult currentIdentity = NativeFileMove::GetIdentity(destination);
    if (currentIdentity.Succeeded()) {
        static_cast<void>(NativeFileMove::RemoveIfIdentityMatches(destination, currentIdentity.identity));
    }
}

}  // namespace

NativeFileCopyResult NativeFileCopy::Copy(const NativeFileCopyRequest& request,
                                          const FileOperationCancellation& cancellation,
                                          const ProgressCallback& handleProgress) {
    if (cancellation.IsCancelled()) {
        return CancelledResult(request);
    }

    FileHandle source(CreateFileW(request.source.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                  FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    if (source.Get() == INVALID_HANDLE_VALUE) {
        return FailedResult(request, std::error_code(static_cast<int>(GetLastError()), std::system_category()));
    }
    BY_HANDLE_FILE_INFORMATION sourceInfo{};
    if (GetFileInformationByHandle(source.Get(), &sourceInfo) == FALSE) {
        return FailedResult(request, std::error_code(static_cast<int>(GetLastError()), std::system_category()));
    }
    if ((sourceInfo.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0) {
        return FailedResult(request, std::make_error_code(std::errc::operation_not_supported));
    }
    const NativeFileIdentity sourceIdentity = IdentityFromInfo(sourceInfo);

    CopyContext context(request, cancellation, handleProgress);
    const BOOL copied = CopyFileExW(request.source.c_str(), request.destination.c_str(), HandleCopyProgress, &context,
                                    nullptr, COPY_FILE_FAIL_IF_EXISTS);
    if (copied != FALSE) {
        BY_HANDLE_FILE_INFORMATION completedSourceInfo{};
        if (GetFileInformationByHandle(source.Get(), &completedSourceInfo) == FALSE) {
            const DWORD sourceError = GetLastError();
            RemovePartialDestination(request.destination, context.destinationIdentity);
            return FailedResult(request, std::error_code(static_cast<int>(sourceError), std::system_category()));
        }
        if (!sourceIdentity.Matches(IdentityFromInfo(completedSourceInfo))) {
            RemovePartialDestination(request.destination, context.destinationIdentity);
            return FailedResult(request, std::error_code(ERROR_BUSY, std::system_category()));
        }
        return CompletedResult(request, sourceIdentity);
    }

    const DWORD error = GetLastError();
    if (cancellation.IsCancelled() || error == ERROR_REQUEST_ABORTED) {
        RemovePartialDestination(request.destination, context.destinationIdentity);
        return CancelledResult(request);
    }
    RemovePartialDestination(request.destination, context.destinationIdentity);
    return FailedResult(request, std::error_code(static_cast<int>(error), std::system_category()));
}
