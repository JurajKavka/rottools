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
    int m_fileBrowserWidth = 100;
    wxFileName m_currentFile;

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleSaveMenuItemClick(wxCommandEvent& event);
    void HandleSaveAsMenuItemClick(wxCommandEvent& event);
    void HandleUndoMenuItemClick(wxCommandEvent& event);
    void HandleRedoMenuItemClick(wxCommandEvent& event);
    void HandleUpdateUndoMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateRedoMenuItem(wxUpdateUIEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleWordWrapMenuItemClick(wxCommandEvent& event);
    bool SaveTextFile(const wxFileName& filePath);

   public:
    explicit MainFrame(wxWindow* parent);

    void OpenTextFile(const wxFileName& filePath);
};
