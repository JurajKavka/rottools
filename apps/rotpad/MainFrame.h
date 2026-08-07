#pragma once

#include <wx/filename.h>
#include <wx/splitter.h>

#include "MainFrameWx.h"

class FileBrowserTreePanel;
class TextEditorPanel;

class MainFrame final : public MainFrameWx {
   private:
    wxSplitterWindow* m_mainSplitter = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    TextEditorPanel* m_textEditorPanel = nullptr;

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);

   public:
    explicit MainFrame(wxWindow* parent);

    void OpenTextFile(const wxFileName& filePath);
};
