#include <wx/msgdlg.h>
#include <wx/stockitem.h>
#include <wx/textdlg.h>
#include <wx/wx.h>

#include <functional>
#include <optional>

#include "FileBrowserTreePanel.h"

class FileBrowserTreePanelFrame : public wxFrame {
   public:
    FileBrowserTreePanelFrame()
        : wxFrame(nullptr, wxID_ANY, "Test: File Browser", wxDefaultPosition, wxSize(400, 600)) {
        auto* treePanel = new FileBrowserTreePanel(
            this,
            {.selectNewFolderName = std::bind_front(&FileBrowserTreePanelFrame::HandleSelectNewFolderName, this),
             .onCreateFolderError = std::bind_front(&FileBrowserTreePanelFrame::HandleCreateFolderError, this),
             .confirmDelete = std::bind_front(&FileBrowserTreePanelFrame::HandleConfirmDelete, this),
             .onDeleteError = std::bind_front(&FileBrowserTreePanelFrame::HandleDeleteError, this)});

        auto* frameSizer = new wxBoxSizer(wxVERTICAL);
        frameSizer->Add(treePanel, 1, wxEXPAND);
        SetSizer(frameSizer);

        wxFileName homeDirectory;
        homeDirectory.AssignHomeDir();
        treePanel->ListDir(homeDirectory);
    }

   private:
    std::optional<wxString> HandleSelectNewFolderName(const std::optional<wxString>& previousName) {
        wxTextEntryDialog dialog(this, _("Name for the new folder:"), _("New Folder"),
                                 previousName.value_or(_("New Folder")));
        if (dialog.ShowModal() != wxID_OK) {
            return std::nullopt;
        }
        return dialog.GetValue();
    }

    void HandleCreateFolderError(const FileBrowserTreePanel::CreateFolderError& error) {
        const wxString detail = wxString::FromUTF8(error.error.message().c_str());
        const wxString message = wxString::Format(_("Could not create folder \"%s\": %s"), error.name.c_str(),
                                                  detail.c_str());
        wxMessageBox(message, _("New Folder"), wxOK | wxICON_ERROR, this);
    }

    bool HandleConfirmDelete(const FileBrowserTreePanel::DeletePrompt& prompt) {
        const wxString name = prompt.isDirectory ? GetLastDirectoryName(prompt.path) : prompt.path.GetFullName();
        const wxString message =
            prompt.isDirectory
                ? wxString::Format(_("Permanently delete the folder \"%s\" and all its contents?"), name.c_str())
                : wxString::Format(_("Permanently delete the file \"%s\"?"), name.c_str());
        wxMessageDialog dialog(this, message, _("Delete"), wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING);
        dialog.SetOKCancelLabels(_("Delete"), wxGetStockLabel(wxID_CANCEL));
        return dialog.ShowModal() == wxID_OK;
    }

    void HandleDeleteError(const FileBrowserTreePanel::DeleteError& error) {
        const wxString name = error.isDirectory ? GetLastDirectoryName(error.path) : error.path.GetFullName();
        const wxString detail = wxString::FromUTF8(error.error.message().c_str());
        const wxString message = wxString::Format(_("Could not completely delete \"%s\": %s"), name.c_str(),
                                                  detail.c_str());
        wxMessageBox(message, _("Delete"), wxOK | wxICON_ERROR, this);
    }
};

class FileBrowserTreePanelApp : public wxApp {
   public:
    bool OnInit() override {
        auto* mainFrame = new FileBrowserTreePanelFrame;
        mainFrame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(FileBrowserTreePanelApp);
