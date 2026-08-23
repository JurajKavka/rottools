#include <windows.h>

#include <atomic>
#include <string>

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

std::filesystem::path QuarantinePath(const std::filesystem::path& source) {
    static std::atomic<std::uint64_t> nextId{1};
    std::filesystem::path name = source.filename();
    name += L".rotfm-remove-";
    name += std::to_wstring(GetCurrentProcessId());
    name += L"-";
    name += std::to_wstring(nextId.fetch_add(1));
    return source.parent_path() / name;
}

}  // namespace

NativeFileMoveResult NativeFileMove::MoveNoReplace(const std::filesystem::path& source,
                                                   const std::filesystem::path& destination) {
    if (MoveFileExW(source.c_str(), destination.c_str(), 0) != FALSE) {
        return {.outcome = NativeFileMoveOutcome::Completed};
    }

    const DWORD error = GetLastError();
    if (error == ERROR_NOT_SAME_DEVICE) {
        return {.outcome = NativeFileMoveOutcome::CrossDevice};
    }
    return {
        .outcome = NativeFileMoveOutcome::Failed,
        .error = std::error_code(static_cast<int>(error), std::system_category()),
    };
}

NativeFileIdentityResult NativeFileMove::GetIdentity(const std::filesystem::path& path) {
    FileHandle file(CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
    if (file.Get() == INVALID_HANDLE_VALUE) {
        return {.error = std::error_code(static_cast<int>(GetLastError()), std::system_category())};
    }

    BY_HANDLE_FILE_INFORMATION info{};
    if (GetFileInformationByHandle(file.Get(), &info) == FALSE) {
        return {.error = std::error_code(static_cast<int>(GetLastError()), std::system_category())};
    }
    return {.identity = IdentityFromInfo(info)};
}

NativeFileRemoveResult NativeFileMove::RemoveIfIdentityMatches(const std::filesystem::path& source,
                                                               const NativeFileIdentity& expectedIdentity) {
    if (!expectedIdentity.valid) {
        return {
            .outcome = NativeFileRemoveOutcome::Failed,
            .error = std::make_error_code(std::errc::invalid_argument),
        };
    }

    const NativeFileIdentityResult sourceIdentity = GetIdentity(source);
    if (!sourceIdentity.Succeeded()) {
        return {.outcome = NativeFileRemoveOutcome::Failed, .error = sourceIdentity.error};
    }
    if (!expectedIdentity.Matches(sourceIdentity.identity)) {
        return {.outcome = NativeFileRemoveOutcome::IdentityMismatch};
    }

    std::filesystem::path quarantine;
    for (;;) {
        quarantine = QuarantinePath(source);
        if (MoveFileExW(source.c_str(), quarantine.c_str(), 0) != FALSE) {
            break;
        }
        const DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS && error != ERROR_FILE_EXISTS) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = std::error_code(static_cast<int>(error), std::system_category()),
            };
        }
    }

    const NativeFileIdentityResult quarantinedIdentity = GetIdentity(quarantine);
    if (!quarantinedIdentity.Succeeded()) {
        if (MoveFileExW(quarantine.c_str(), source.c_str(), 0) != FALSE) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = quarantinedIdentity.error,
            };
        }
        return {
            .outcome = NativeFileRemoveOutcome::Failed,
            .error = std::error_code(static_cast<int>(GetLastError()), std::system_category()),
        };
    }
    if (!expectedIdentity.ContentMatches(quarantinedIdentity.identity)) {
        if (MoveFileExW(quarantine.c_str(), source.c_str(), 0) == FALSE) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = std::error_code(static_cast<int>(GetLastError()), std::system_category()),
            };
        }
        return {.outcome = NativeFileRemoveOutcome::IdentityMismatch};
    }

    if (DeleteFileW(quarantine.c_str()) == FALSE) {
        const std::error_code removeError(static_cast<int>(GetLastError()), std::system_category());
        if (MoveFileExW(quarantine.c_str(), source.c_str(), 0) != FALSE) {
            return {.outcome = NativeFileRemoveOutcome::Failed, .error = removeError};
        }
        return {
            .outcome = NativeFileRemoveOutcome::Failed,
            .error = std::error_code(static_cast<int>(GetLastError()), std::system_category()),
        };
    }
    return {.outcome = NativeFileRemoveOutcome::Completed};
}

NativeFileRemoveResult NativeFileMove::RemoveIfEntryMatches(const std::filesystem::path& path,
                                                            const NativeFileIdentity& expectedIdentity) {
    const NativeFileIdentityResult currentIdentity = GetIdentity(path);
    if (!currentIdentity.Succeeded()) {
        return {.outcome = NativeFileRemoveOutcome::Failed, .error = currentIdentity.error};
    }
    if (!expectedIdentity.SameEntry(currentIdentity.identity)) {
        return {.outcome = NativeFileRemoveOutcome::IdentityMismatch};
    }
    return RemoveIfIdentityMatches(path, currentIdentity.identity);
}
