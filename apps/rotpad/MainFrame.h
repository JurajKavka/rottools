#pragma once

#include <wx/filename.h>
#include <wx/splitter.h>

#include <optional>
#include <vector>

#include "FileBrowserTreePanel.h"
#include "MainFrameWx.h"

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
    void HandleCopyMenuItemClick(wxCommandEvent& event);
    void HandleCutMenuItemClick(wxCommandEvent& event);
    void HandlePasteMenuItemClick(wxCommandEvent& event);
    void HandleUpdateUndoMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateRedoMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateCopyMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateCutMenuItem(wxUpdateUIEvent& event);
    void HandleUpdatePasteMenuItem(wxUpdateUIEvent& event);
#ifdef __WXOSX__
    void HandleMenuHighlight(wxMenuEvent& event);
#endif
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleFileBrowserHomeRequested();
    void HandleFileBrowserCloseRequested();
    std::optional<wxString> HandleSelectBrowserFolderName(const std::optional<wxString>& previousName);
    void HandleBrowserCreateFolderError(const FileBrowserTreePanel::CreateFolderError& error);
    bool HandleConfirmBrowserMoveToTrash(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths);
    bool HandleConfirmBrowserPermanentlyDelete(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths);
    bool ConfirmBrowserDelete(const std::vector<FileBrowserTreePanel::DeletePrompt>& paths, wxString message,
                              const wxString& action);
    void HandleBrowserPathDeleted(const FileBrowserTreePanel::DeletePrompt& deleted);
    void HandleBrowserDeleteError(const std::vector<FileBrowserTreePanel::DeleteError>& errors,
                                  FileBrowserTreePanel::DeleteMode mode);
    void HandleFileTreeBrowserMoveError(const std::vector<FileBrowserTreePanel::MoveError>& errors);
    bool HandleFileTreeBrowserBeforeMove(const std::vector<FileBrowserTreePanel::MovePrompt>& paths);
    void HandleFileTreeBrowserPathMoved(const FileBrowserTreePanel::MovePrompt& moved,
                                       const wxFileName& destination);
    [[nodiscard]] bool IsCurrentDocumentAffectedByMove(const FileBrowserTreePanel::MovePrompt& moved) const;
    [[nodiscard]] bool IsCurrentDocumentAffectedByDelete(const FileBrowserTreePanel::DeletePrompt& deleted) const;
    void HideFileBrowser();
    void HandleWordWrapMenuItemClick(wxCommandEvent& event);
    void HandleFontMenuItemClick(wxCommandEvent& event);
    bool SaveTextFile(const wxFileName& filePath);

   public:
    explicit MainFrame(wxWindow* parent);

    void HandleOpenTextFile(const wxFileName& filePath);
};
