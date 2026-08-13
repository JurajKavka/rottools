#include "HelperFunctions.h"

#include <wx/window.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

namespace detail {
void WriteLogLine(std::ostream& stream, const std::string& line) {
    static std::mutex logMutex;
    std::lock_guard lock(logMutex);
    stream << line << '\n';
}
}  // namespace detail

void printLog(const wxString& msg) {
    detail::WriteLogLine(std::cout, msg.ToStdString());
}

void printError(const wxString& msg) {
    detail::WriteLogLine(std::cerr, msg.ToStdString());
}

std::string trimToStdString(const wxString& str) {
    wxString copy = str;
    return copy.Trim(true).Trim(false).ToStdString();
}

void printCppVersion() {
    switch (__cplusplus) {
        case 202302L:
            std::cout << "C++23" << std::endl;
            break;
        case 202002L:
            std::cout << "C++20" << std::endl;
            break;
        case 201703L:
            std::cout << "C++17" << std::endl;
            break;
        case 201402L:
            std::cout << "C++14" << std::endl;
            break;
        case 201103L:
            std::cout << "C++11" << std::endl;
            break;
        case 199711L:
            std::cout << "C++98" << std::endl;
            break;
        default:
            std::cout << "Unknown/Experimental (" << __cplusplus << ")" << std::endl;
            break;
    }
}

bool ReadFileUtf8(const wxFileName& filePath, wxString& contents) {
    std::ifstream in(filePath.GetFullPath().fn_str(), std::ios::binary);
    if (!in) {
        return false;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    contents = wxString::FromUTF8(buffer.str());
    return true;
}

bool WriteFileUtf8(const wxFileName& filePath, const wxString& contents) {
    const wxScopedCharBuffer utf8 = contents.utf8_str();
    std::ofstream out(filePath.GetFullPath().fn_str(), std::ios::binary | std::ios::trunc);
    if (!out) {
        return false;
    }
    out.write(utf8.data(), static_cast<std::streamsize>(utf8.length()));
    out.close();
    return static_cast<bool>(out);
}

bool IsWindowOrDescendant(wxWindow* window, wxWindow* candidate) {
    return window != nullptr && candidate != nullptr && (candidate == window || window->IsDescendant(candidate));
}

bool ContainsFocus(wxWindow* window) {
    return IsWindowOrDescendant(window, wxWindow::FindFocus());
}
