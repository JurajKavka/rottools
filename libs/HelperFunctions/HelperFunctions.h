#pragma once

#include <wx/filename.h>
#include <wx/string.h>

#include <format>
#include <iostream>
#include <string>
#include <utility>

/**
 * @brief What happens to the scroll position when a panel's content is replaced.
 *
 * Shared by the content panels (web view, file browser tree): a live reload of
 * the same content keeps the view still, a navigation starts at the top.
 */
enum class ScrollBehavior {
    /// Start at the top, as a fresh load does
    ResetToTop,
    /// Keep the current vertical scroll offset across the update
    KeepPosition,
};

namespace detail {
// Serializes writes so a whole line stays atomic even when worker threads log
// concurrently (std::osyncstream is not available in Apple's libc++ yet).
void WriteLogLine(std::ostream& stream, const std::string& line);
}  // namespace detail

void printCppVersion();
std::string trimToStdString(const wxString& str);
wxFileName GetAssetPath(const wxString& filename);

// 1. Keep this for simple, single wxString prints: printLog(myWxString);
void printLog(const wxString& msg);
void printError(const wxString& msg);

// 2. Teach C++20 std::format how to handle wxString automatically!
template <>
struct std::formatter<wxString> : std::formatter<std::string> {
    auto format(const wxString& str, std::format_context& ctx) const {
        // Convert to std::string under the hood and pass it to the standard formatter
        return std::formatter<std::string>::format(str.ToStdString(), ctx);
    }
};

// 3. The magic variadic template for implicit formatting.
template <typename... Args>
void printLog(std::format_string<Args...> fmt, Args&&... args) {
    detail::WriteLogLine(std::cout, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void printError(std::format_string<Args...> fmt, Args&&... args) {
    detail::WriteLogLine(std::cerr, std::format(fmt, std::forward<Args>(args)...));
}
