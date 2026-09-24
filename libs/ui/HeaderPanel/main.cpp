#include <wx/log.h>
#include <wx/wx.h>

#include <functional>
#include <utility>
#include <vector>

#include "FlatHomeButton.h"
#include "HeaderPanel.h"

class HeaderPanelFrame final : public wxFrame {
   public:
    HeaderPanelFrame() : wxFrame(nullptr, wxID_ANY, "Test: Header Panel", wxDefaultPosition, wxSize(480, 150)) {
        std::vector<HeaderPanel::ToolButton> toolButtons{
            HeaderPanel::ToolButton::Make<FlatHomeButton>(
                _("Home"), std::bind_front(&HeaderPanelFrame::HandleHomeButtonClick, this)),
            HeaderPanel::ToolButton::Make<FlatHomeButton>(
                _("Second Home"), std::bind_front(&HeaderPanelFrame::HandleSecondHomeButtonClick, this)),
        };
        auto* header =
            new HeaderPanel(this, std::move(toolButtons),
                            {_("Close Header"), std::bind_front(&HeaderPanelFrame::HandleCloseButtonClick, this)});

        auto* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(header, 0, wxEXPAND);
        sizer->Add(new wxStaticText(this, wxID_ANY, "Click a button or focus it and press Space/Enter."), 0, wxALL,
                   FromDIP(12));
        SetSizer(sizer);
        wxLogMessage("Header panel demo ready; click the left tool buttons or the right close button.");
    }

   private:
    void HandleHomeButtonClick() {
        wxLogMessage("First Home clicked");
    }
    void HandleSecondHomeButtonClick() {
        wxLogMessage("Second Home clicked");
    }
    void HandleCloseButtonClick() {
        wxLogMessage("Close clicked");
    }
};

class HeaderPanelApp final : public wxApp {
   public:
    bool OnInit() override {
        wxLog::SetActiveTarget(new wxLogStderr());
        auto* frame = new HeaderPanelFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(HeaderPanelApp);
