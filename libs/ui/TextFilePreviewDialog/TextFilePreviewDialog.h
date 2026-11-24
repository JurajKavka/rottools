#pragma once

#include "CsvParser.h"
#include "ScintillaTextEditorPanel.h"
#include "TextFilePreviewDialogWx.h"

/** Read-only, selectable plain-text preview. The caller owns file loading. */
class TextFilePreviewDialog final : public TextFilePreviewDialogWx {
   public:
    explicit TextFilePreviewDialog(wxWindow* parent, const wxString& title = wxEmptyString);

    void ShowText(const wxString& text);
    void ShowCode(const wxString& text, ScintillaTextEditorPanel::Syntax syntax);
    bool ShowCsv(const CsvResult& csv);

   private:
    void RecreateTextControl(bool wrap);
    void HandleWordWrap(wxCommandEvent& event);
    ScintillaTextEditorPanel* m_code = nullptr;
};
