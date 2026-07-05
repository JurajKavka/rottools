#pragma once

#include "MarkdownSourcePanelWx.h"

class MarkdownSourcePanel : public MarkdownSourcePanelWx {
   public:
    explicit MarkdownSourcePanel(wxWindow* parent);
    void ShowMarkdown(const wxString& markdown);
};
