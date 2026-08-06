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

    void OpenTextFile(const wxFileName& filePath);

   public:
    explicit MainFrame(wxWindow* parent);
};
