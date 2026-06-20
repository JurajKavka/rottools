#pragma once

#include <wx/filename.h>
#include <wx/string.h>

#include <iostream>
#include <string>

void printCppVersion();

template <typename T>
inline void printLog(const T& msg) {
    std::cout << msg << std::endl;
}

void printLog(const wxString& msg);

std::string trimToStdString(const wxString& str);

wxFileName GetAssetPath(const wxString& filename);
