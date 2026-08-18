#pragma once

#include <functional>

#include "HtmlSourcePanelWx.h"

class HtmlSourcePanel : public HtmlSourcePanelWx {
   public:
    using OnCloseCallback = std::function<void()>;

    /**
     * @param parent Parent window
     * @param onCloseCallback Called when the user clicks the panel's close button
     */
    explicit HtmlSourcePanel(wxWindow* parent, OnCloseCallback onCloseCallback = nullptr);
    void ShowHtml(const wxString& html);
    void Copy();
    [[nodiscard]] bool CanCopy() const;
    [[nodiscard]] bool ContainsFocus() const;

   private:
    OnCloseCallback m_onCloseCallback;

    void HandleMarginClick(wxStyledTextEvent& event);
    void HandleCloseButtonClick(wxCommandEvent& event);
};
