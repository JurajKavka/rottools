#pragma once

#include <wx/fdrepdlg.h>
#include <wx/filename.h>
#include <wx/splitter.h>

#include <memory>
#include <optional>
#include <vector>

#include "FileBrowserTreePanel.h"
#include "MainFrameWx.h"

class TextEditorPanel;
class FsWatcher;

class MainFrame final : public MainFrameWx {
   private:
    wxSplitterWindow* m_mainSplitter = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    TextEditorPanel* m_textEditorPanel = nullptr;
    int m_fileBrowserWidth = 100;
    wxFileName m_currentFile;
    wxString m_loadedText;
    std::unique_ptr<FsWatcher> m_browserWatcher;
    std::unique_ptr<FsWatcher> m_documentWatcher;
    wxFileName m_watchedDirectory;
    wxFindReplaceData m_findData{wxFR_DOWN};
    wxFindReplaceDialog* m_findDialog = nullptr;
    bool m_findDialogIsReplace = false;

    void HandleNewFileMenuItemClick(wxCommandEvent& event);
    void HandleNewWindowMenuItemClick(wxCommandEvent& event);
    void HandleCloseWindow(wxCloseEvent& event);
    void HandleCloseMenuItemClick(wxCommandEvent& event);
    void HandleAboutMenuItemClick(wxCommandEvent& event);
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
    void HandleSelectAllMenuItemClick(wxCommandEvent& event);
    void HandleFindMenuItemClick(wxCommandEvent& event);
    void HandleReplaceMenuItemClick(wxCommandEvent& event);
    void HandleFindDialogFind(wxFindDialogEvent& event);
    void HandleFindDialogReplace(wxFindDialogEvent& event);
    void HandleFindDialogReplaceAll(wxFindDialogEvent& event);
    void HandleFindDialogClose(wxFindDialogEvent& event);
    void ShowFindDialog(bool replace);
    void HandleUpdateEditorMenuItem(wxUpdateUIEvent& event);
    void HandleEditorTextChanged(wxCommandEvent& event);
#ifdef __WXOSX__
    void HandleMenuHighlight(wxMenuEvent& event);
#endif
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleFileBrowserHomeRequested();
    void HandleFileBrowserCloseRequested();
    void HandleDirectoryChanged(const wxFileName& directory);
    void HandleBrowserWatcherChange();
    void HandleDocumentWatcherChange();
    void RefreshDocumentWatcher();
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
    void HandleFileTreeBrowserCopyError(const std::vector<FileBrowserTreePanel::CopyError>& errors);
    bool HandleFileTreeBrowserConfirmOverwrite(const FileBrowserTreePanel::OverwritePrompt& prompt);
    void HandleFileTreeBrowserDestinationUpdated(const wxFileName& destination);
    bool HandleFileTreeBrowserBeforeMove(const std::vector<FileBrowserTreePanel::MovePrompt>& paths);
    void HandleFileTreeBrowserPathMoved(const FileBrowserTreePanel::MovePrompt& moved, const wxFileName& destination);
    [[nodiscard]] bool IsCurrentDocumentAffectedByMove(const FileBrowserTreePanel::MovePrompt& moved) const;
    [[nodiscard]] bool IsCurrentDocumentAffectedByDelete(const FileBrowserTreePanel::DeletePrompt& deleted) const;
    void HideFileBrowser();
    void ResetDocument();
    void UpdateDocumentTitle();
    [[nodiscard]] bool ConfirmSaveBeforeDiscard();
    [[nodiscard]] bool SaveDocument();
    [[nodiscard]] bool SaveDocumentAs();
    [[nodiscard]] bool ConfirmOverwriteExternalChanges(const wxFileName& filePath, bool recreationConfirmed);
    void HandleWordWrapMenuItemClick(wxCommandEvent& event);
    void HandleFontMenuItemClick(wxCommandEvent& event);
    bool SaveTextFile(const wxFileName& filePath, bool recreationConfirmed = false);

   public:
    explicit MainFrame(wxWindow* parent, const wxFileName& initialDirectory = {});
    ~MainFrame() override;

    void HandleOpenTextFile(const wxFileName& filePath);
};
