#pragma once

#include <filesystem>
#include <system_error>

struct NativeFileCreateResult {
    std::error_code error;

    [[nodiscard]] bool Succeeded() const noexcept {
        return !error;
    }
};

/** Creates one empty regular file without replacing an existing entry. */
class NativeFileCreate final {
   public:
    [[nodiscard]] static NativeFileCreateResult Create(const std::filesystem::path& path);
};
