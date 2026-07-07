#include "HelperFunctions.h"

#include <wx/stdpaths.h>

#include <iostream>
#include <mutex>

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

wxFileName GetAssetPath(const wxString& filename) {
    wxFileName f;
    f.AssignDir(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());
    f.AppendDir("assets");
    f.SetFullName(filename);
    return f;
}
