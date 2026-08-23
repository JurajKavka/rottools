#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <string>

#include "NativeFileMove.h"

namespace {

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

std::filesystem::path QuarantinePath(const std::filesystem::path& source) {
    static std::atomic<std::uint64_t> nextId{1};
    std::filesystem::path name = source.filename();
    name += ".rotfm-remove-";
    name += std::to_string(getpid());
    name += "-";
    name += std::to_string(nextId.fetch_add(1));
    return source.parent_path() / name;
}

}  // namespace

NativeFileMoveResult NativeFileMove::MoveNoReplace(const std::filesystem::path& source,
                                                   const std::filesystem::path& destination) {
#ifdef SYS_renameat2
    if (syscall(SYS_renameat2, AT_FDCWD, source.c_str(), AT_FDCWD, destination.c_str(), RENAME_NOREPLACE) == 0) {
        return {.outcome = NativeFileMoveOutcome::Completed};
    }
    if (errno == EXDEV) {
        return {.outcome = NativeFileMoveOutcome::CrossDevice};
    }
    if (errno != ENOSYS && errno != EINVAL) {
        return {
            .outcome = NativeFileMoveOutcome::Failed,
            .error = std::error_code(errno, std::generic_category()),
        };
    }
#endif
    return {
        .outcome = NativeFileMoveOutcome::Failed,
        .error = std::make_error_code(std::errc::operation_not_supported),
    };
}

NativeFileIdentityResult NativeFileMove::GetIdentity(const std::filesystem::path& path) {
    struct stat info{};
    if (lstat(path.c_str(), &info) < 0) {
        return {.error = std::error_code(errno, std::generic_category())};
    }
    return {.identity = IdentityFromStat(info)};
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

#ifndef SYS_renameat2
    return {
        .outcome = NativeFileRemoveOutcome::Failed,
        .error = std::make_error_code(std::errc::operation_not_supported),
    };
#else
    std::filesystem::path quarantine;
    for (;;) {
        quarantine = QuarantinePath(source);
        if (syscall(SYS_renameat2, AT_FDCWD, source.c_str(), AT_FDCWD, quarantine.c_str(), RENAME_NOREPLACE) == 0) {
            break;
        }
        if (errno != EEXIST) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = std::error_code(errno, std::generic_category()),
            };
        }
    }

    const NativeFileIdentityResult quarantinedIdentity = GetIdentity(quarantine);
    if (!quarantinedIdentity.Succeeded()) {
        if (syscall(SYS_renameat2, AT_FDCWD, quarantine.c_str(), AT_FDCWD, source.c_str(), RENAME_NOREPLACE) == 0) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = quarantinedIdentity.error,
            };
        }
        return {
            .outcome = NativeFileRemoveOutcome::Failed,
            .error = std::error_code(errno, std::generic_category()),
        };
    }
    if (!expectedIdentity.ContentMatches(quarantinedIdentity.identity)) {
        if (syscall(SYS_renameat2, AT_FDCWD, quarantine.c_str(), AT_FDCWD, source.c_str(), RENAME_NOREPLACE) < 0) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = std::error_code(errno, std::generic_category()),
            };
        }
        return {.outcome = NativeFileRemoveOutcome::IdentityMismatch};
    }
    if (unlink(quarantine.c_str()) < 0) {
        const std::error_code removeError(errno, std::generic_category());
        if (syscall(SYS_renameat2, AT_FDCWD, quarantine.c_str(), AT_FDCWD, source.c_str(), RENAME_NOREPLACE) == 0) {
            return {
                .outcome = NativeFileRemoveOutcome::Failed,
                .error = removeError,
            };
        }
        return {
            .outcome = NativeFileRemoveOutcome::Failed,
            .error = std::error_code(errno, std::generic_category()),
        };
    }
    return {.outcome = NativeFileRemoveOutcome::Completed};
#endif
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
