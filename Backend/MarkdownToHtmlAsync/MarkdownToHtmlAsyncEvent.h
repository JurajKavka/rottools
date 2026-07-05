#pragma once

#include <wx/event.h>
#include <wx/filename.h>

class MarkdownToHtmlAsyncEvent : public wxEvent {
   public:
    MarkdownToHtmlAsyncEvent(wxEventType type, int id) : wxEvent(id, type) {}

    wxString html;
    // The raw markdown source the html was generated from
    wxString markdown;
    wxString error;
    wxFileName filePath;

    // Required for wxWidgets event system
    wxEvent* Clone() const override {
        return new MarkdownToHtmlAsyncEvent(*this);
    }
};
