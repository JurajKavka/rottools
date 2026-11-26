#pragma once

#include "CsvTablePanel.h"
#include "ScintillaTextEditorPanel.h"
#include "TextFilePreviewDialogWx.h"

/** Read-only reference preview. The caller owns file loading. */
class TextFilePreviewDialog final : public TextFilePreviewDialogWx {
   public:
    explicit TextFilePreviewDialog(wxWindow* parent, const wxString& title = wxEmptyString);

    void ShowText(const wxString& text);
    void ShowCode(const wxString& text, Syntax syntax);
    bool ShowCsv(const wxString& text, CsvTablePanel::OnErrorCallback onError = {});

   private:
    void RecreateTextControl(bool wrap);
    void UpdateFontSizeControls(int pointSize);
    void HandleWordWrap(wxCommandEvent& event);
    void HandleFontSizeChanged(wxCommandEvent& event);
    void HandleCodeZoom(wxStyledTextEvent& event);

    ScintillaTextEditorPanel* m_code = nullptr;
};
