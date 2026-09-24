#pragma once

#include "FlatButtonsBase.h"

class FlatHomeButton final : public FlatButtonsBase {
   public:
    explicit FlatHomeButton(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                            wxWindowID id = wxID_ANY);

   private:
    void DrawIcon(wxDC& dc, const wxPoint& center) const override;
};
