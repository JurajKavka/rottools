#include "FlatHomeButton.h"

#include <wx/dc.h>

#include <utility>

FlatHomeButton::FlatHomeButton(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                               wxWindowID id)
    : FlatButtonsBase(parent, label, std::move(onClickHandler), id) {}

void FlatHomeButton::DrawIcon(wxDC& dc, const wxPoint& center) const {
    const int roofHalfWidth = FromDIP(6);
    const int wallHalfWidth = FromDIP(4);
    const int doorHalfWidth = FromDIP(2);
    const int roofTop = center.y - FromDIP(6);
    const int eave = center.y;
    const int doorTop = center.y + FromDIP(2);
    const int base = center.y + FromDIP(5);

    dc.DrawLine(center.x - roofHalfWidth, eave, center.x, roofTop);
    dc.DrawLine(center.x, roofTop, center.x + roofHalfWidth, eave);
    dc.DrawLine(center.x - wallHalfWidth, eave, center.x - wallHalfWidth, base);
    dc.DrawLine(center.x + wallHalfWidth, eave, center.x + wallHalfWidth, base);
    dc.DrawLine(center.x - wallHalfWidth, base, center.x - doorHalfWidth, base);
    dc.DrawLine(center.x - doorHalfWidth, base, center.x - doorHalfWidth, doorTop);
    dc.DrawLine(center.x - doorHalfWidth, doorTop, center.x + doorHalfWidth, doorTop);
    dc.DrawLine(center.x + doorHalfWidth, doorTop, center.x + doorHalfWidth, base);
    dc.DrawLine(center.x + doorHalfWidth, base, center.x + wallHalfWidth, base);
}
