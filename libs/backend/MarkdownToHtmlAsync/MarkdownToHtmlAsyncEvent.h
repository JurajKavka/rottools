#pragma once

#include <wx/event.h>
#include <wx/filename.h>

#include <cstdint>

class MarkdownToHtmlAsyncEvent : public wxEvent {
   public:
    MarkdownToHtmlAsyncEvent(wxEventType type, int id) : wxEvent(id, type) {}

    wxString html;
    wxString markdown;
    wxString error;
    wxFileName filePath;
    std::uint64_t requestId = 0;

    // Required for wxWidgets event system
    wxEvent* Clone() const override {
        return new MarkdownToHtmlAsyncEvent(*this);
    }
};
