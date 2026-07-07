#pragma once

#include <wx/dnd.h>
#include <wx/filename.h>

#include <functional>

class FileDropTarget : public wxFileDropTarget {
   public:
    using DropCallback = std::function<void(const wxFileName&)>;

    explicit FileDropTarget(DropCallback callback);

    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

   private:
    DropCallback m_callback;
};
