#pragma once

#include <wx/event.h>
#include <wx/filename.h>

class MarkdownToHtmlAsyncEvent : public wxEvent {
   public:
    MarkdownToHtmlAsyncEvent(wxEventType type, int id) : wxEvent(id, type) {}

    wxString html;
    wxString error;
    wxFileName filePath;

    // Required for wxWidgets event system
    virtual wxEvent* Clone() const override {
        return new MarkdownToHtmlAsyncEvent(*this);
    }
};