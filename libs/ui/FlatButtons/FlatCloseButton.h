#pragma once

#include "FlatButtonsBase.h"

class FlatCloseButton final : public FlatButtonsBase {
   public:
    explicit FlatCloseButton(wxWindow* parent, const wxString& label, std::function<void()> onClickHandler,
                             wxWindowID id = wxID_ANY);

   private:
    void DrawIcon(wxDC& dc, const wxPoint& center) const override;
};
