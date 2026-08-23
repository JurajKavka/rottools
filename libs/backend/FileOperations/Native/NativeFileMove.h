#pragma once

#include <filesystem>
#include <system_error>

#include "NativeFileIdentity.h"

enum class NativeFileMoveOutcome { Completed, CrossDevice, Failed };

struct NativeFileMoveResult {
    NativeFileMoveOutcome outcome = NativeFileMoveOutcome::Failed;
    std::error_code error;
};

enum class NativeFileRemoveOutcome { Completed, IdentityMismatch, Failed };

struct NativeFileRemoveResult {
    NativeFileRemoveOutcome outcome = NativeFileRemoveOutcome::Failed;
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return outcome == NativeFileRemoveOutcome::Completed;
    }
};

struct NativeFileIdentityResult {
    NativeFileIdentity identity;
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return identity.valid && !error;
    }
};

/** Performs a same-volume regular-file move without replacing a destination. */
class NativeFileMove final {
   public:
    [[nodiscard]] static NativeFileMoveResult MoveNoReplace(const std::filesystem::path& source,
                                                            const std::filesystem::path& destination);
    [[nodiscard]] static NativeFileIdentityResult GetIdentity(const std::filesystem::path& path);
    /** Removes source only when it still names the file that was copied. */
    [[nodiscard]] static NativeFileRemoveResult RemoveIfIdentityMatches(const std::filesystem::path& source,
                                                                        const NativeFileIdentity& expectedIdentity);
    /** Removes a partial copy only when its native directory entry still matches. */
    [[nodiscard]] static NativeFileRemoveResult RemoveIfEntryMatches(const std::filesystem::path& path,
                                                                     const NativeFileIdentity& expectedIdentity);
};
