#include "TextFilePreviewDialog.h"

#include <wx/settings.h>

TextFilePreviewDialog::TextFilePreviewDialog(wxWindow* parent, const wxString& title)
    : TextFilePreviewDialogWx(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    const wxFont fixedFont = wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT);
    if (fixedFont.IsOk()) {
        m_textCtrl2->SetFont(fixedFont);
    }

    m_wrapCheckBox->SetValue(false);
    m_wrapCheckBox->Bind(wxEVT_CHECKBOX, &TextFilePreviewDialog::HandleWordWrap, this);
    SetSize(FromDIP(wxSize(983, 610)));
    CentreOnParent();
}

void TextFilePreviewDialog::ShowText(const wxString& text) {
    m_textCtrl2->SetValue(text);
    m_textCtrl2->SetInsertionPoint(0);
}

void TextFilePreviewDialog::RecreateTextControl(bool wrap) {
    wxTextCtrl* previous = m_textCtrl2;
    const wxString text = previous->GetValue();
    long selectionStart = 0;
    long selectionEnd = 0;
    previous->GetSelection(&selectionStart, &selectionEnd);

    long style = wxTE_MULTILINE | wxTE_READONLY;
    if (!wrap) {
        style |= wxTE_DONTWRAP;
    }
#ifdef __WXMSW__
    style |= wxTE_RICH2;
#endif

    auto* replacement = new wxTextCtrl(this, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, style);
    replacement->SetFont(previous->GetFont());
    GetSizer()->Replace(previous, replacement);
    m_textCtrl2 = replacement;
    previous->Destroy();
    m_textCtrl2->SetSelection(selectionStart, selectionEnd);
    Layout();
    m_textCtrl2->SetFocus();
}

void TextFilePreviewDialog::HandleWordWrap(wxCommandEvent&) {
    RecreateTextControl(m_wrapCheckBox->GetValue());
}
