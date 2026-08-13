#pragma once

#include <wx/filename.h>
#include <wx/string.h>

#include <format>
#include <iostream>
#include <string>
#include <utility>

class wxWindow;

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

/**
 * @brief Reads a whole file as UTF-8 text.
 *
 * @param filePath File to read
 * @param contents Receives the text; untouched when the read fails
 * @return false when the file could not be opened or read
 */
bool ReadFileUtf8(const wxFileName& filePath, wxString& contents);

/**
 * @brief Writes text to a file as UTF-8, replacing anything already there.
 *
 * @param filePath File to write
 * @param contents Text to write
 * @return false when the file could not be opened or written
 */
bool WriteFileUtf8(const wxFileName& filePath, const wxString& contents);

/**
 * @brief Tests whether a window is a candidate window or one of its ancestors.
 *
 * Composite controls often contain the native widget that actually receives
 * focus. Use this when routing a command to the containing panel should work
 * for both the panel itself and any focused child control.
 *
 * @param window Possible ancestor window
 * @param candidate Window to test
 * @return true when both windows exist and candidate is window or its descendant
 */
[[nodiscard]] bool IsWindowOrDescendant(wxWindow* window, wxWindow* candidate);

/**
 * @brief Tests whether keyboard focus is inside a window's subtree.
 *
 * Use this to route frame-level actions such as Copy to the composite panel
 * that owns the focused native child.
 *
 * @param window Window whose subtree should be checked
 * @return true when window or one of its descendants has focus
 */
[[nodiscard]] bool ContainsFocus(wxWindow* window);

// 1. Keep this for simple, single wxString prints: printLog(myWxString);
void printLog(const wxString& msg);
void printError(const wxString& msg);

// 2. Teach C++20 std::format how to handle wxString automatically!
template <>
struct std::formatter<wxString> : std::formatter<std::string> {
    auto format(const wxString& str, std::format_context& ctx) const {
        // std::format produces a narrow string. Keep that string explicitly
        // UTF-8 instead of using wxString::ToStdString(), whose default
        // conversion depends on the process locale and can return an empty
        // string for otherwise valid Unicode text.
        const std::string utf8 = str.utf8_string();
        return std::formatter<std::string>::format(utf8, ctx);
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
