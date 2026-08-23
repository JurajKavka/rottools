#include <windows.h>

#include "NativeFileCreate.h"

NativeFileCreateResult NativeFileCreate::Create(const std::filesystem::path& path) {
    const HANDLE file =
        CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return {.error = std::error_code(static_cast<int>(GetLastError()), std::system_category())};
    }
    if (CloseHandle(file) == FALSE) {
        return {.error = std::error_code(static_cast<int>(GetLastError()), std::system_category())};
    }
    return {};
}
