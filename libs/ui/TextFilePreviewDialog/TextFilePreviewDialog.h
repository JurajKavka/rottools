#pragma once

#include "TextFilePreviewDialogWx.h"

/** Read-only, selectable plain-text preview. The caller owns file loading. */
class TextFilePreviewDialog final : public TextFilePreviewDialogWx {
   public:
    explicit TextFilePreviewDialog(wxWindow* parent, const wxString& title = wxEmptyString);

    void ShowText(const wxString& text);

   private:
    void RecreateTextControl(bool wrap);
    void HandleWordWrap(wxCommandEvent& event);
};
