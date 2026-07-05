#pragma once

#include "HtmlSourcePanelWx.h"

class HtmlSourcePanel : public HtmlSourcePanelWx {
   public:
    explicit HtmlSourcePanel(wxWindow* parent);
    void ShowHtml(const wxString& html);

   private:
    void OnMarginClick(wxStyledTextEvent& event);
};
