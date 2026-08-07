#pragma once

#include "TextEditorPanelWx.h"

// Forward declarations keep the Scintilla headers out of consumers.
class wxFont;
class wxStyledTextCtrl;

class TextEditorPanel : public TextEditorPanelWx {
   private:
    wxStyledTextCtrl* m_textEditor = nullptr;

   public:
    explicit TextEditorPanel(wxWindow* parent);

    /** Replace the current document and start it with an empty undo history. */
    void LoadText(const wxString& text);

    [[nodiscard]] wxString GetText() const;
    [[nodiscard]] bool HasUnsavedChanges() const;
    void MarkSaved();
    void Undo();
    void Redo();
    [[nodiscard]] bool CanUndo() const;
    [[nodiscard]] bool CanRedo() const;

    void SetWordWrap(bool enabled);
    [[nodiscard]] bool IsWordWrapEnabled() const;
    void SetEditorFont(const wxFont& font);
    void FocusEditor();
};
