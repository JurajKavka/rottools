#pragma once

#include <wx/event.h>
#include <wx/filename.h>
#include <filesystem>

namespace fs = std::filesystem;

struct FileEntry {
    fs::path path;
    std::string name;
    bool isDirectory;
    uintmax_t size;
};

class DirectoryScannerEvent : public wxEvent {
   public:
    DirectoryScannerEvent(wxEventType type, int id) : wxEvent(id, type) {}

    wxString error;
    wxFileName currentDirectory;
    std::vector<FileEntry> files;

    // Required for wxWidgets event system
    virtual wxEvent* Clone() const override {
        return new DirectoryScannerEvent(*this);
    }
};