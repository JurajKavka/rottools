#pragma once

#include <wx/event.h>

class MarkdownToHtmlAsyncEvent : public wxEvent {
   public:
    MarkdownToHtmlAsyncEvent(wxEventType type, int id) : wxEvent(id, type) {}

    // Add any data you want
    wxString html;
    wxString filePath;

    // Required for wxWidgets event system
    virtual wxEvent* Clone() const override {
        return new MarkdownToHtmlAsyncEvent(*this);
    }
};