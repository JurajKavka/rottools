#pragma once

#include "ScintillaTextEditorPanelWx.h"

// Forward declarations keep the Scintilla headers out of consumers.
class wxFont;
class wxStyledTextCtrl;
class wxStyledTextEvent;
class wxSysColourChangedEvent;

class ScintillaTextEditorPanel : public ScintillaTextEditorPanelWx {
   public:
    enum class Syntax {
        None,
        Markdown,
    };

    struct Options {
        Syntax syntax = Syntax::None;
        bool lineNumbers = false;
    };

    enum class LoadBehavior {
        ResetToTop,
        KeepPosition,
    };

   private:
    wxStyledTextCtrl* m_textEditor = nullptr;
    Options m_options;

    void ApplyEditorStyles(const wxFont& font);
    void ApplyMarkdownStyles();
    void ApplySelectionColours();
    void UpdateEolModeForText(const wxString& text);
    void UpdateLineNumberMarginWidth();
    void HandleEditorMetricsChanged(wxStyledTextEvent& event);
    void HandleSystemColourChanged(wxSysColourChangedEvent& event);

   public:
    explicit ScintillaTextEditorPanel(wxWindow* parent);
    ScintillaTextEditorPanel(wxWindow* parent, Options options);

    /** Replace the current document and start it with an empty undo history. */
    void LoadText(const wxString& text);

    /**
     * Replace the saved document and clear its undo history. KeepPosition also
     * retains the primary selection and visible viewport.
     */
    void LoadText(const wxString& text, LoadBehavior loadBehavior);

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
