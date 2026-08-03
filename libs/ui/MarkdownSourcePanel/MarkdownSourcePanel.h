#pragma once

#include <functional>

#include "HelperFunctions.h"
#include "MarkdownSourcePanelWx.h"

class MarkdownSourcePanel : public MarkdownSourcePanelWx {
   public:
    using OnSaveCallback = std::function<void(const wxString& markdown)>;
    using OnCloseCallback = std::function<void()>;

    /**
     * @param parent Parent window
     * @param onSaveCallback Called with the edited text when the user saves (Cmd/Ctrl+S)
     * @param onCloseCallback Called when the user clicks the panel's close button
     */
    explicit MarkdownSourcePanel(wxWindow* parent, OnSaveCallback onSaveCallback = nullptr,
                                 OnCloseCallback onCloseCallback = nullptr);

    void ShowMarkdown(const wxString& markdown, ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop);

   private:
    OnSaveCallback m_onSaveCallback;
    OnCloseCallback m_onCloseCallback;

    void HandleKeyDown(wxKeyEvent& event);
    void HandleCloseButtonClick(wxCommandEvent& event);
};
