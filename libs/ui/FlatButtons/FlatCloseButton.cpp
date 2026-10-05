#include "FlatCloseButton.h"

#include <wx/dc.h>

#include <utility>

FlatCloseButton::FlatCloseButton(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                                 wxWindowID id)
    : FlatButtonsBase(parent, label, std::move(onClickHandler), id) {}

void FlatCloseButton::DrawIcon(wxDC& dc, const wxPoint& center) const {
    const int arm = FromDIP(4);
    dc.DrawLine(center.x - arm, center.y - arm, center.x + arm, center.y + arm);
    dc.DrawLine(center.x - arm, center.y + arm, center.x + arm, center.y - arm);
}
