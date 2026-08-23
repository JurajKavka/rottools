#pragma once

#include <wx/filename.h>
#include <wx/panel.h>

#include <functional>
#include <optional>

class FileBrowserTreePanel;
class wxFocusEvent;
class wxTextCtrl;

/** One independently navigable side of the dual-pane file manager. */
class FileManagerPane final : public wxPanel {
   public:
    using ActivatedCallback = std::function<void(FileManagerPane*)>;

    explicit FileManagerPane(wxWindow* parent, ActivatedCallback onActivated);

    void SetDirectory(const wxFileName& directory);
    [[nodiscard]] wxFileName GetCurrentDirectory() const;
    [[nodiscard]] std::optional<wxFileName> GetSelectedPath() const;
    void Reload();
    void FocusFileList();

   private:
    wxTextCtrl* m_pathText = nullptr;
    FileBrowserTreePanel* m_browser = nullptr;
    ActivatedCallback m_onActivated;

    void HandleDirectoryChanged(const wxFileName& directory);
    void HandleFileOpened(const wxFileName& file);
    void HandleHomeRequested();
    void HandleActivated();
    void HandlePathEnter(wxCommandEvent& event);
    void HandlePathFocus(wxFocusEvent& event);
};
