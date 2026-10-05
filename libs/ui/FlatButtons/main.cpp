#include <wx/log.h>
#include <wx/wx.h>

#include <functional>

#include "FlatCloseButton.h"
#include "FlatHomeButton.h"

class FlatButtonsFrame final : public wxFrame {
   public:
    FlatButtonsFrame() : wxFrame(nullptr, wxID_ANY, "Test: Flat Buttons", wxDefaultPosition, wxSize(420, 180)) {
        auto* panel = new wxPanel(this);
        auto* homeButton =
            new FlatHomeButton(panel, _("Home"), std::bind_front(&FlatButtonsFrame::HandleHomeButtonClick, this));
        auto* closeButton =
            new FlatCloseButton(panel, _("Close"), std::bind_front(&FlatButtonsFrame::HandleCloseButtonClick, this));
        auto* hint = new wxStaticText(panel, wxID_ANY, "Click a button or focus it and press Space/Enter.");

        auto* homeRow = new wxBoxSizer(wxHORIZONTAL);
        homeRow->Add(new wxStaticText(panel, wxID_ANY, "Home"), 1, wxALIGN_CENTER_VERTICAL);
        homeRow->Add(homeButton, 0, wxALIGN_CENTER_VERTICAL);

        auto* closeRow = new wxBoxSizer(wxHORIZONTAL);
        closeRow->Add(new wxStaticText(panel, wxID_ANY, "Close"), 1, wxALIGN_CENTER_VERTICAL);
        closeRow->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL);

        auto* panelSizer = new wxBoxSizer(wxVERTICAL);
        panelSizer->Add(hint, 0, wxALL, FromDIP(16));
        panelSizer->Add(homeRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(16));
        panelSizer->Add(closeRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(16));
        panel->SetSizer(panelSizer);

        wxLogMessage("Flat buttons demo ready; click Home or Close, or focus a button and press Space/Enter.");
    }

   private:
    int m_homeClickCount = 0;
    int m_closeClickCount = 0;

    void HandleHomeButtonClick() {
        wxLogMessage("FlatHomeButton clicked (%d)", ++m_homeClickCount);
    }

    void HandleCloseButtonClick() {
        wxLogMessage("FlatCloseButton clicked (%d)", ++m_closeClickCount);
    }
};

class FlatButtonsApp final : public wxApp {
   public:
    bool OnInit() override {
        wxLog::SetActiveTarget(new wxLogStderr());
        auto* frame = new FlatButtonsFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(FlatButtonsApp);
