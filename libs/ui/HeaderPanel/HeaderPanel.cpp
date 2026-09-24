#include "HeaderPanel.h"

#include <wx/sizer.h>

#include <utility>

#include "FlatCloseButton.h"

HeaderPanel::HeaderPanel(wxWindow* parent, std::vector<ToolButton> toolButtons, CloseButtonConfig closeButtonConfig)
    : wxPanel(parent) {
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    for (ToolButton& toolButton : toolButtons) {
        auto* button = toolButton.create(this, toolButton.label, std::move(toolButton.onClickHandler));
        sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(0));
    }

    sizer->AddStretchSpacer();

    auto* closeButton = new FlatCloseButton(this, closeButtonConfig.label, std::move(closeButtonConfig.onClickHandler));
    sizer->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(0));
    SetSizer(sizer);
}
