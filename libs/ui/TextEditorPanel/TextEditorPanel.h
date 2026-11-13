#pragma once

#include <optional>
#include <utility>

#include "HelperFunctions.h"
#include "TextEditorPanelWx.h"

class wxFont;
class wxTextCtrl;

class TextEditorPanel : public TextEditorPanelWx {
   private:
    wxTextCtrl* m_textEditor = nullptr;
    wxString m_savedText;
    bool m_wordWrapEnabled = false;

    void RecreateTextEditor(bool wordWrapEnabled);
    [[nodiscard]] std::optional<std::pair<long, long>> FindMatch(const wxString& text, const TextSearchOptions& options,
                                                                 long start) const;

   public:
    explicit TextEditorPanel(wxWindow* parent);

    /** Replace the current document and start it with an empty undo history. */
    void LoadText(const wxString& text, bool keepPosition = false);

    [[nodiscard]] wxString GetText() const;
    [[nodiscard]] bool HasUnsavedChanges() const;
    void MarkSaved();
    void Undo();
    void Redo();
    void Copy();
    void Cut();
    void Paste();
    void SelectAll();
    [[nodiscard]] wxString GetSelectedText() const;
    bool FindText(const wxString& text, const TextSearchOptions& options = {});
    [[nodiscard]] bool ReplaceText(const wxString& text, const wxString& replacement,
                                   const TextSearchOptions& options = {});
    [[nodiscard]] int ReplaceAllText(const wxString& text, const wxString& replacement,
                                     const TextSearchOptions& options = {});
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
