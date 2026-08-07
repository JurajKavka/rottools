#pragma once

#include "ScintillaTextEditorPanelWx.h"

// Forward declarations keep the Scintilla headers out of consumers.
class wxFont;
class wxStyledTextCtrl;

class ScintillaTextEditorPanel : public ScintillaTextEditorPanelWx {
   private:
    wxStyledTextCtrl* m_textEditor = nullptr;

   public:
    explicit ScintillaTextEditorPanel(wxWindow* parent);

    /** Replace the current document and start it with an empty undo history. */
    void LoadText(const wxString& text);

    [[nodiscard]] wxString GetText() const;
    [[nodiscard]] bool HasUnsavedChanges() const;
    void MarkSaved();
    void Undo();
    void Redo();
    void Copy();
    void Cut();
    void Paste();
    [[nodiscard]] bool CanUndo() const;
    [[nodiscard]] bool CanRedo() const;
    [[nodiscard]] bool CanCopy() const;
    [[nodiscard]] bool CanCut() const;
    [[nodiscard]] bool CanPaste() const;

    void SetWordWrap(bool enabled);
    [[nodiscard]] bool IsWordWrapEnabled() const;
    void SetEditorFont(const wxFont& font);
    [[nodiscard]] wxFont GetEditorFont() const;
    void FocusEditor();
};
