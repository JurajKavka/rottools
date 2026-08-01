#pragma once

#include <functional>

#include "MarkdownSourcePanelWx.h"

class MarkdownSourcePanel : public MarkdownSourcePanelWx {
   public:
    using OnCloseCallback = std::function<void()>;

    explicit MarkdownSourcePanel(wxWindow* parent, OnCloseCallback onCloseCallback = nullptr);
    void ShowMarkdown(const wxString& markdown);

   private:
    OnCloseCallback m_onCloseCallback;

    void HandleCloseButtonClick(wxCommandEvent& event);
};
