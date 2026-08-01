#pragma once

#include <functional>

#include "HelperFunctions.h"
#include "MarkdownSourcePanelWx.h"

class MarkdownSourcePanel : public MarkdownSourcePanelWx {
   public:
    using OnSaveCallback = std::function<void(const wxString& markdown)>;

    explicit MarkdownSourcePanel(wxWindow* parent, OnSaveCallback onSaveCallback = nullptr);

    void ShowMarkdown(const wxString& markdown, ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop);

   private:
    OnSaveCallback m_onSaveCallback;

    void HandleKeyDown(wxKeyEvent& event);
};
