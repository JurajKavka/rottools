#include <wx/log.h>
#include <wx/statbmp.h>
#include <wx/wx.h>

#include <utility>

#include "Icons.h"

class IconsFrame final : public wxFrame {
   public:
    IconsFrame() : wxFrame(nullptr, wxID_ANY, "Test: SVG Icons", wxDefaultPosition, wxSize(300, 180)) {
        auto* panel = new wxPanel(this);
        auto* panelSizer = new wxBoxSizer(wxVERTICAL);
        panelSizer->Add(new wxStaticText(panel, wxID_ANY, "File-browser icons at their supported UI size"), 0,
                        wxALL | wxALIGN_CENTER_HORIZONTAL, FromDIP(16));

        auto* iconSizer = new wxBoxSizer(wxHORIZONTAL);
        for (const auto& [label, bitmap] : {
                 std::pair{"Default file", rottools::ui::icons::DefaultFile()},
                 std::pair{"Markdown", rottools::ui::icons::MarkdownFile()},
                 std::pair{"Folder", rottools::ui::icons::Folder()},
             })
        {
            auto* itemSizer = new wxBoxSizer(wxVERTICAL);
            itemSizer->Add(new wxStaticBitmap(panel, wxID_ANY, bitmap), 0, wxALIGN_CENTER_HORIZONTAL);
            itemSizer->Add(new wxStaticText(panel, wxID_ANY, label), 0, wxTOP | wxALIGN_CENTER_HORIZONTAL,
                           FromDIP(8));
            iconSizer->Add(itemSizer, 1, wxLEFT | wxRIGHT | wxALIGN_BOTTOM, FromDIP(12));
        }
        wxLogMessage("Icons demo ready; default file, Markdown, and folder icons rendered at 16 px.");

        panelSizer->Add(iconSizer, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, FromDIP(16));
        panel->SetSizer(panelSizer);
    }
};

class IconsApp final : public wxApp {
   public:
    bool OnInit() override {
        wxLog::SetActiveTarget(new wxLogStderr());
        auto* frame = new IconsFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(IconsApp);
