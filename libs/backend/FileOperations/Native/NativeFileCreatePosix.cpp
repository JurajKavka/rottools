#include <fcntl.h>
#include <unistd.h>

#include <cerrno>

#include "NativeFileCreate.h"

NativeFileCreateResult NativeFileCreate::Create(const std::filesystem::path& path) {
    const int descriptor = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0666);
    if (descriptor < 0) {
        return {.error = std::error_code(errno, std::generic_category())};
    }
    if (close(descriptor) < 0) {
        return {.error = std::error_code(errno, std::generic_category())};
    }
    return {};
}
