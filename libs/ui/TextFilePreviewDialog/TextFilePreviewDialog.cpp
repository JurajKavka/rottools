#include "TextFilePreviewDialog.h"

#include <wx/settings.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>

#include <algorithm>
#include <utility>

#include "CsvTablePanel.h"

TextFilePreviewDialog::TextFilePreviewDialog(wxWindow* parent, const wxString& title)
    : TextFilePreviewDialogWx(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
    const wxFont fixedFont = wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT);
    if (fixedFont.IsOk()) {
        m_textCtrl2->SetFont(fixedFont);
    }

    m_wrapCheckBox->SetValue(false);
    m_wrapCheckBox->Bind(wxEVT_CHECKBOX, &TextFilePreviewDialog::HandleWordWrap, this);

    const int pointSize = m_textCtrl2->GetFont().GetPointSize();
    m_fontSizeSlider->SetName(_("Text size"));
    UpdateFontSizeControls(pointSize);
    m_fontSizeSlider->Bind(wxEVT_SLIDER, &TextFilePreviewDialog::HandleFontSizeChanged, this);

    SetSize(FromDIP(wxSize(983, 610)));
    Layout();
    CentreOnParent();
}

void TextFilePreviewDialog::ShowText(const wxString& text) {
    m_textCtrl2->SetValue(text);
    m_textCtrl2->SetInsertionPoint(0);
}

void TextFilePreviewDialog::ShowCode(const wxString& text, ScintillaTextEditorPanel::Syntax syntax) {
    m_code = new ScintillaTextEditorPanel(
        this, {.syntax = syntax, .lineNumbers = true, .wordWrap = m_wrapCheckBox->GetValue(), .readOnly = true});
    m_code->LoadText(text);
    m_code->Bind(wxEVT_STC_ZOOM, &TextFilePreviewDialog::HandleCodeZoom, this);
    UpdateFontSizeControls(m_code->GetEditorPointSize());
    GetSizer()->Replace(m_textCtrl2, m_code);
    m_textCtrl2->Hide();
    Layout();
}

bool TextFilePreviewDialog::ShowCsv(const wxString& text, CsvTablePanel::OnErrorCallback onError) {
    const auto utf8 = text.ToUTF8();
    auto* table = new CsvTablePanel(this, std::string_view(utf8.data(), utf8.length()), std::move(onError));
    if (!table->IsLoaded()) {
        table->Destroy();
        return false;
    }
    GetSizer()->Replace(m_textCtrl2, table);
    m_textCtrl2->Hide();
    m_toolBar1->Hide();
    Layout();
    return true;
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
    if (m_code) {
        m_code->SetWordWrap(m_wrapCheckBox->GetValue());
    } else {
        RecreateTextControl(m_wrapCheckBox->GetValue());
    }
}

void TextFilePreviewDialog::UpdateFontSizeControls(int pointSize) {
    // Scintilla's built-in zoom can extend beyond the slider's initial range.
    const int min = m_fontSizeSlider->GetMin();
    const int max = m_fontSizeSlider->GetMax();
    if (pointSize < min || pointSize > max) {
        m_fontSizeSlider->SetRange(std::min(min, pointSize), std::max(max, pointSize));
    }
    if (m_fontSizeSlider->GetValue() != pointSize) {
        m_fontSizeSlider->SetValue(pointSize);
    }
    m_fontSizeValue->SetLabel(wxString::Format(_("%d pt"), pointSize));
}

void TextFilePreviewDialog::HandleFontSizeChanged(wxCommandEvent& event) {
    const int pointSize = event.GetInt();
    if (m_code) {
        m_code->SetEditorPointSize(pointSize);
    } else {
        wxFont font = m_textCtrl2->GetFont();
        font.SetPointSize(pointSize);
        m_textCtrl2->SetFont(font);
        UpdateFontSizeControls(m_textCtrl2->GetFont().GetPointSize());
    }
}

void TextFilePreviewDialog::HandleCodeZoom(wxStyledTextEvent& event) {
    UpdateFontSizeControls(m_code->GetEditorPointSize());
    event.Skip();
}
