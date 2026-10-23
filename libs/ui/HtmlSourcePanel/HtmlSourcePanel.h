#pragma once

#include <functional>

#include "HtmlSourcePanelWx.h"

class wxCommandEvent;
class wxMenuEvent;

class HtmlSourcePanel : public HtmlSourcePanelWx {
   public:
    using OnCloseCallback = std::function<void()>;

    /**
     * @param parent Parent window
     * @param onCloseCallback Called when the user selects Close from the source context menu
     */
    explicit HtmlSourcePanel(wxWindow* parent, OnCloseCallback onCloseCallback = nullptr);
    void ShowHtml(const wxString& html);
    void Copy();
    [[nodiscard]] bool CanCopy() const;

   private:
    OnCloseCallback m_onCloseCallback;

    void HandleEditorMenuOpen(wxMenuEvent& event);
    void HandleEditorCloseMenu(wxCommandEvent& event);
    void HandleMarginClick(wxStyledTextEvent& event);
};
