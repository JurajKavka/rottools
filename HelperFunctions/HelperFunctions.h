#pragma once

#include <wx/filename.h>
#include <wx/string.h>

#include <iostream>
#include <string>

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

// 3. The magic variadic template for implicit formatting
template <typename... Args>
void printLog(std::format_string<Args...> fmt, Args&&... args) {
    // std::vformat is used under the hood by std::format, but using std::format
    // here directly with forwarded arguments works perfectly.
    std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
}

template <typename... Args>
void printError(std::format_string<Args...> fmt, Args&&... args) {
    std::cerr << std::format(fmt, std::forward<Args>(args)...) << std::endl;
}