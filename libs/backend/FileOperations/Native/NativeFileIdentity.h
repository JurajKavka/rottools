#pragma once

#include <cstdint>

/** Native entry identity plus a snapshot used to detect content changes. */
struct NativeFileIdentity {
    std::uint64_t volumeId = 0;
    std::uint64_t fileId = 0;
    std::uint64_t size = 0;
    std::uint64_t modificationStamp = 0;
    std::uint64_t changeStamp = 0;
    bool valid = false;

    [[nodiscard]] bool SameEntry(const NativeFileIdentity& other) const noexcept {
        return valid && other.valid && volumeId == other.volumeId && fileId == other.fileId;
    }

    [[nodiscard]] bool ContentMatches(const NativeFileIdentity& other) const noexcept {
        return SameEntry(other) && size == other.size && modificationStamp == other.modificationStamp;
    }

    [[nodiscard]] bool Matches(const NativeFileIdentity& other) const noexcept {
        return ContentMatches(other) && changeStamp == other.changeStamp;
    }
};
