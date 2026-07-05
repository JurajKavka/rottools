#pragma once

#include <wx/event.h>
#include <wx/filename.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct FileEntry {
    std::filesystem::path path;
    std::string name;
    bool isDirectory = false;
    std::uintmax_t size = 0;
};

class DirectoryScannerEvent : public wxEvent {
   public:
    DirectoryScannerEvent(wxEventType type, int id) : wxEvent(id, type) {}

    wxString error;
    wxFileName currentDirectory;
    std::vector<FileEntry> files;

    // Required for wxWidgets event system
    wxEvent* Clone() const override {
        return new DirectoryScannerEvent(*this);
    }
};
