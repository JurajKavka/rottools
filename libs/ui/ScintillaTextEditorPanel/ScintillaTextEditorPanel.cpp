#include "ScintillaTextEditorPanel.h"

#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stc/stc.h>
#include <wx/version.h>
#include <wx/window.h>

#include <algorithm>
#include <vector>
#include <utility>

#include "HelperFunctions.h"

namespace {
int GetScintillaSearchFlags(const TextSearchOptions& options) {
    // TODO: Use wxSTC_FIND_NONE when Ubuntu provides wxWidgets 3.3.3.
    int flags = 0;
    if (options.wholeWord) {
        flags |= wxSTC_FIND_WHOLEWORD;
    }
    if (options.matchCase) {
        flags |= wxSTC_FIND_MATCHCASE;
    }
    return flags;
}
}  // namespace

namespace {
const char* GetSyntaxKeywords(ScintillaTextEditorPanel::Syntax syntax) {
    using Syntax = ScintillaTextEditorPanel::Syntax;
    switch (syntax) {
        case Syntax::Sql: return "select from where insert into values update delete create table drop alter join left right inner outer on as and or not null is order by group having limit distinct union all case when then else end primary key references index database default";
        case Syntax::Python: return "False None True and as assert async await break class continue def del elif else except finally for from global if import in is lambda nonlocal not or pass raise return try while with yield match case";
        case Syntax::Shell: return "if then else elif fi for while until do done case esac in function select time coproc break continue return exit export local readonly unset shift source eval exec trap set printf echo read test true false";
        case Syntax::Cpp: return "alignas alignof auto bool break case catch char class const constexpr continue default delete do double else enum explicit extern false float for friend if inline int long namespace new nullptr operator private protected public return short signed sizeof static struct switch template this throw true try typedef typename union unsigned using virtual void volatile while";
        case Syntax::Java: return "abstract assert boolean break byte case catch char class const continue default do double else enum extends final finally float for if implements import instanceof int interface long native new package private protected public return short static strictfp super switch synchronized this throw throws transient try void volatile while true false null var record sealed permits yield";
        case Syntax::JavaScript: return "async await break case catch class const continue debugger default delete do else export extends false finally for function if import in instanceof let new null return static super switch this throw true try typeof var void while with yield";
        case Syntax::TypeScript: return "abstract any as asserts async await boolean break case catch class const constructor continue declare default delete do else enum export extends false finally for from function get if implements import in infer instanceof interface is keyof let module namespace never new null number object private protected public readonly return set static string super switch symbol this throw true try type typeof undefined unique unknown var void while yield";
        default: return "";
    }
}
}  // namespace

namespace {
struct SyntaxStyle {
    int id;
    wxColour colour;
};

std::vector<SyntaxStyle> GetSyntaxStyle(ScintillaTextEditorPanel::Syntax syntax) {
    const wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const bool dark = background.Red() * 299 + background.Green() * 587 + background.Blue() * 114 < 128000;
    const wxColour keyword = dark ? wxColour(130, 190, 255) : wxColour(0, 60, 170);
    const wxColour string = dark ? wxColour(255, 180, 140) : wxColour(145, 35, 15);
    const wxColour comment = dark ? wxColour(145, 205, 145) : wxColour(0, 110, 35);
    if (syntax == ScintillaTextEditorPanel::Syntax::Python) {
        return {
            {wxSTC_P_WORD, keyword},
            {wxSTC_P_STRING, string},
            {wxSTC_P_CHARACTER, string},
            {wxSTC_P_TRIPLE, string},
            {wxSTC_P_TRIPLEDOUBLE, string},
            {wxSTC_P_COMMENTLINE, comment},
        };
    } else if (syntax == ScintillaTextEditorPanel::Syntax::Xml) {
        return {
            {wxSTC_H_TAG, keyword},
            {wxSTC_H_TAGUNKNOWN, keyword},
            {wxSTC_H_TAGEND, keyword},
            {wxSTC_H_XMLSTART, keyword},
            {wxSTC_H_XMLEND, keyword},
            {wxSTC_H_ATTRIBUTE, keyword},
            {wxSTC_H_ATTRIBUTEUNKNOWN, keyword},
            {wxSTC_H_DOUBLESTRING, string},
            {wxSTC_H_SINGLESTRING, string},
            {wxSTC_H_COMMENT, comment},
            {wxSTC_H_ENTITY, keyword},
        };
    } else if (syntax == ScintillaTextEditorPanel::Syntax::Shell) {
        return {
            {wxSTC_SH_WORD, keyword},
            {wxSTC_SH_STRING, string},
            {wxSTC_SH_CHARACTER, string},
            {wxSTC_SH_BACKTICKS, string},
            {wxSTC_SH_HERE_DELIM, string},
            {wxSTC_SH_HERE_Q, string},
            {wxSTC_SH_COMMENTLINE, comment},
        };
    } else if (syntax == ScintillaTextEditorPanel::Syntax::Sql) {
        return {
            {wxSTC_SQL_WORD, keyword},
            {wxSTC_SQL_STRING, string},
            {wxSTC_SQL_CHARACTER, string},
            {wxSTC_SQL_COMMENT, comment},
            {wxSTC_SQL_COMMENTLINE, comment},
        };
    } else {
        return {
            {wxSTC_C_WORD, keyword},
            {wxSTC_C_STRING, string},
            {wxSTC_C_CHARACTER, string},
            {wxSTC_C_COMMENT, comment},
            {wxSTC_C_COMMENTLINE, comment},
            {wxSTC_C_COMMENTDOC, comment},
#ifdef wxSTC_C_BACKTICKS
            {wxSTC_C_BACKTICKS, string},
#endif
#ifdef wxSTC_C_STRINGRAW
            {wxSTC_C_STRINGRAW, string},
#endif
        };
    }
}
}  // namespace

ScintillaTextEditorPanel::Syntax ScintillaTextEditorPanel::GetReferenceSyntax(ReferenceFormat format) {
    using Syntax = ScintillaTextEditorPanel::Syntax;
    switch (format) {
        case ReferenceFormat::Sql: return Syntax::Sql;
        case ReferenceFormat::JavaScript: return Syntax::JavaScript;
        case ReferenceFormat::TypeScript: return Syntax::TypeScript;
        case ReferenceFormat::Python: return Syntax::Python;
        case ReferenceFormat::Cpp: return Syntax::Cpp;
        case ReferenceFormat::Shell: return Syntax::Shell;
        case ReferenceFormat::Java: return Syntax::Java;
        case ReferenceFormat::Xml: return Syntax::Xml;
        case ReferenceFormat::PlainText:
        case ReferenceFormat::Csv: return Syntax::None;
    }
    return Syntax::None;
}

ScintillaTextEditorPanel::ScintillaTextEditorPanel(wxWindow* parent) : ScintillaTextEditorPanel(parent, {}, {}) {}

ScintillaTextEditorPanel::ScintillaTextEditorPanel(wxWindow* parent, Options options, Callbacks callbacks)
    : ScintillaTextEditorPanelWx(parent), m_options(std::move(options)), m_callbacks(std::move(callbacks)) {
    m_textEditor = new wxStyledTextCtrl(this, wxID_ANY);

    // Keep the plain-text defaults visually close to a native text editor.
    // Richer editor features are enabled only when requested by the consumer.
    int lexer = wxSTC_LEX_NULL;
    switch (m_options.syntax) {
        case Syntax::Markdown: lexer = wxSTC_LEX_MARKDOWN; break;
        case Syntax::Sql: lexer = wxSTC_LEX_SQL; break;
        case Syntax::Python: lexer = wxSTC_LEX_PYTHON; break;
        case Syntax::Shell: lexer = wxSTC_LEX_BASH; break;
        case Syntax::Xml: lexer = wxSTC_LEX_XML; break;
        case Syntax::Cpp:
        case Syntax::Java:
        case Syntax::JavaScript:
        case Syntax::TypeScript: lexer = wxSTC_LEX_CPP; break;
        case Syntax::None: break;
    }
    m_textEditor->SetLexer(lexer);
    m_textEditor->SetKeyWords(0, GetSyntaxKeywords(m_options.syntax));
    if (m_options.syntax == Syntax::JavaScript || m_options.syntax == Syntax::TypeScript) {
#if wxMAJOR_VERSION > 3 || (wxMAJOR_VERSION == 3 && wxMINOR_VERSION > 3) || \
    (wxMAJOR_VERSION == 3 && wxMINOR_VERSION == 3 && wxRELEASE_NUMBER >= 3)
        m_textEditor->SetProperty("lexer.cpp.backquoted.strings", "2");
#else
        m_textEditor->SetProperty("lexer.cpp.backquoted.strings", "1");
#endif
    }
    m_textEditor->SetCodePage(wxSTC_CP_UTF8);
    m_textEditor->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textEditor->SetMarginWidth(0, 0);
    m_textEditor->SetMarginWidth(1, 0);
    m_textEditor->SetMarginWidth(2, 0);
#ifdef __WXOSX__
    // Work around the reverse-arrow cursor leak (wxWidgets PR #27419).
    m_textEditor->SetMarginCursor(0, wxSTC_CURSORARROW);
#endif
    m_textEditor->SetIndentationGuides(false);
    m_textEditor->SetViewEOL(false);
    m_textEditor->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
    m_textEditor->SetWrapMode(wxSTC_WRAP_NONE);
    m_textEditor->SetScrollWidthTracking(true);
#ifdef __WXMSW__
    m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
#else
    m_textEditor->SetEOLMode(wxSTC_EOL_LF);
#endif

    ApplyEditorStyles(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    ApplySelectionColours();

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_textEditor, 1, wxEXPAND | wxALL, 0);
    Layout();

    if (m_options.lineNumbers) {
        m_textEditor->Bind(wxEVT_STC_MODIFIED, &ScintillaTextEditorPanel::HandleEditorMetricsChanged, this);
        m_textEditor->Bind(wxEVT_STC_ZOOM, &ScintillaTextEditorPanel::HandleEditorMetricsChanged, this);
        UpdateLineNumberMarginWidth();
    }
    if (m_callbacks.onEditorContextMenuOpening) {
        m_textEditor->Bind(wxEVT_MENU_OPEN, &ScintillaTextEditorPanel::HandleEditorMenuOpen, this);
    }
    Bind(wxEVT_SYS_COLOUR_CHANGED, &ScintillaTextEditorPanel::HandleSystemColourChanged, this);

    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();

    SetWordWrap(m_options.wordWrap);
    m_textEditor->SetReadOnly(m_options.readOnly);
}

bool ScintillaTextEditorPanel::NewDocument() {
    if (!ConfirmSaveBeforeDiscard()) {
        return false;
    }

    DiscardDocument();
    return true;
}

void ScintillaTextEditorPanel::DiscardDocument() {
    m_currentFile = wxFileName();
    m_loadedText.clear();
    LoadText(wxString{});
    RequestDocumentWatch();
    NotifyDocumentChanged(ChangeReason::NewDocument, wxString{});
}

bool ScintillaTextEditorPanel::ShowOpenDialog() {
    if (!m_callbacks.selectOpenFile) {
        return false;
    }

    const std::optional<wxFileName> selectedFile = m_callbacks.selectOpenFile();
    return selectedFile && selectedFile->IsOk() && OpenFile(*selectedFile);
}

bool ScintillaTextEditorPanel::OpenFile(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (!absolutePath.FileExists()) {
        NotifyError(ErrorCode::FileDoesNotExist, absolutePath);
        return false;
    }
    if (!absolutePath.IsFileReadable()) {
        NotifyError(ErrorCode::FileNotReadable, absolutePath);
        return false;
    }
    if (!ConfirmSaveBeforeDiscard()) {
        return false;
    }

    wxString text;
    if (!ReadFileUtf8(absolutePath, text)) {
        NotifyError(ErrorCode::FileReadFailed, absolutePath);
        return false;
    }

    m_currentFile = absolutePath;
    m_loadedText = text;
    LoadText(text);
    RequestDocumentWatch();
    NotifyStatusChanged(Status::Loading);
    NotifyDocumentChanged(ChangeReason::Opened, text);
    return true;
}

bool ScintillaTextEditorPanel::Save() {
    return m_currentFile.IsOk() ? SaveFile(m_currentFile) : SaveAs();
}

bool ScintillaTextEditorPanel::SaveAs() {
    if (!m_callbacks.selectSaveFile) {
        return false;
    }

    const std::optional<wxFileName> selectedFile = m_callbacks.selectSaveFile(m_currentFile);
    return selectedFile && selectedFile->IsOk() && SaveFile(*selectedFile);
}

bool ScintillaTextEditorPanel::ConfirmSaveBeforeDiscard() {
    const bool fileMissing = m_currentFile.IsOk() && !m_currentFile.FileExists();
    if (!HasUnsavedChanges() && !fileMissing) {
        return true;
    }

    if (!m_callbacks.confirmSaveBeforeDiscard) {
        return false;
    }

    const SavePrompt prompt = {
        .reason = fileMissing ? SavePromptReason::FileRemoved : SavePromptReason::UnsavedChanges,
        .filePath = m_currentFile,
    };
    switch (m_callbacks.confirmSaveBeforeDiscard(prompt)) {
        case SavePromptDecision::Save:
            return m_currentFile.IsOk() ? SaveFile(m_currentFile, fileMissing) : SaveAs();
        case SavePromptDecision::Discard:
            return true;
        case SavePromptDecision::Cancel:
            return false;
    }

    return false;
}

bool ScintillaTextEditorPanel::SaveFile(const wxFileName& filePath, bool missingFileRecreationConfirmed) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();
    const bool targetWasMissing = !absolutePath.FileExists();

    if (!targetWasMissing && !absolutePath.IsFileWritable()) {
        NotifyError(ErrorCode::FileNotWritable, absolutePath);
        return false;
    }
    if (!ConfirmOverwriteExternalChanges(absolutePath, missingFileRecreationConfirmed)) {
        return false;
    }

    const wxString text = GetText();
    if (!WriteFileUtf8(absolutePath, text)) {
        NotifyError(ErrorCode::FileWriteFailed, absolutePath);
        return false;
    }

    const bool pathChanged = !m_currentFile.IsOk() || !m_currentFile.SameAs(absolutePath);
    m_currentFile = absolutePath;
    m_loadedText = text;
    MarkSaved();
    if (pathChanged || targetWasMissing) {
        RequestDocumentWatch();
    }

    NotifyStatusChanged(Status::Saved);
    NotifyDocumentChanged(ChangeReason::Saved, text, pathChanged || targetWasMissing);
    return true;
}

bool ScintillaTextEditorPanel::ConfirmOverwriteExternalChanges(const wxFileName& filePath,
                                                               bool missingFileRecreationConfirmed) {
    if (!m_currentFile.IsOk() || !m_currentFile.SameAs(filePath)) {
        return true;
    }

    OverwritePromptReason reason = OverwritePromptReason::FileRemoved;
    if (!filePath.FileExists()) {
        if (missingFileRecreationConfirmed) {
            return true;
        }
    } else {
        wxString onDisk;
        if (!ReadFileUtf8(filePath, onDisk)) {
            NotifyError(ErrorCode::ExternalChangeCheckFailed, filePath);
            return false;
        }
        if (onDisk == m_loadedText) {
            return true;
        }
        reason = OverwritePromptReason::FileChanged;
    }

    if (!m_callbacks.confirmOverwriteExternalChanges) {
        return false;
    }
    return m_callbacks.confirmOverwriteExternalChanges({.reason = reason, .filePath = filePath}) ==
           OverwritePromptDecision::Proceed;
}

void ScintillaTextEditorPanel::LoadText(const wxString& text, LoadBehavior loadBehavior) {
    m_textEditor->SetReadOnly(false);
    UpdateEolModeForText(text);

    if (loadBehavior == LoadBehavior::KeepPosition) {
        const int caretPosition = m_textEditor->GetCurrentPos();
        const int anchorPosition = m_textEditor->GetAnchor();
        const int firstVisibleLine = m_textEditor->GetFirstVisibleLine();
        const int horizontalOffset = m_textEditor->GetXOffset();

        m_textEditor->SetText(text);
        m_textEditor->EmptyUndoBuffer();
        m_textEditor->SetSavePoint();
        m_textEditor->SetAnchor(std::min(anchorPosition, m_textEditor->GetLength()));
        m_textEditor->SetCurrentPos(std::min(caretPosition, m_textEditor->GetLength()));
        // FirstVisibleLine is a display-line index (wrapped sub-lines count),
        // so let Scintilla clamp it rather than comparing it with document lines.
        m_textEditor->SetFirstVisibleLine(firstVisibleLine);
        m_textEditor->SetXOffset(horizontalOffset);
        UpdateLineNumberMarginWidth();
        m_textEditor->SetReadOnly(m_options.readOnly);
        return;
    }

    m_textEditor->SetText(text);
    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();
    m_textEditor->GotoPos(0);
    m_textEditor->SetFirstVisibleLine(0);
    m_textEditor->SetXOffset(0);
    UpdateLineNumberMarginWidth();
    m_textEditor->SetReadOnly(m_options.readOnly);
}

wxString ScintillaTextEditorPanel::GetText() const {
    return m_textEditor->GetText();
}

bool ScintillaTextEditorPanel::HasUnsavedChanges() const {
    return m_textEditor->GetModify();
}

void ScintillaTextEditorPanel::MarkSaved() {
    m_textEditor->SetSavePoint();
}

void ScintillaTextEditorPanel::Undo() {
    if (m_textEditor->CanUndo()) {
        m_textEditor->Undo();
    }
    FocusEditor();
}

void ScintillaTextEditorPanel::Redo() {
    if (m_textEditor->CanRedo()) {
        m_textEditor->Redo();
    }
    FocusEditor();
}

void ScintillaTextEditorPanel::Copy() {
    if (CanCopy()) {
        m_textEditor->Copy();
    }
    FocusEditor();
}

void ScintillaTextEditorPanel::Cut() {
    if (CanCut()) {
        m_textEditor->Cut();
    }
    FocusEditor();
}

void ScintillaTextEditorPanel::Paste() {
    if (CanPaste()) {
        m_textEditor->Paste();
    }
    FocusEditor();
}

bool ScintillaTextEditorPanel::CanUndo() const {
    return m_textEditor->CanUndo();
}

bool ScintillaTextEditorPanel::CanRedo() const {
    return m_textEditor->CanRedo();
}

bool ScintillaTextEditorPanel::CanCopy() const {
    return m_textEditor->GetSelectionStart() != m_textEditor->GetSelectionEnd();
}

bool ScintillaTextEditorPanel::CanCut() const {
    return CanCopy() && !m_textEditor->GetReadOnly();
}

bool ScintillaTextEditorPanel::CanPaste() const {
    return m_textEditor->CanPaste();
}

wxString ScintillaTextEditorPanel::GetSelectedText() const {
    return m_textEditor->GetSelectedText();
}

bool ScintillaTextEditorPanel::FindText(const wxString& text, const TextSearchOptions& options) {
    if (text.IsEmpty()) {
        return false;
    }

    const int documentEnd = m_textEditor->GetLength();
    const int selectionStart = m_textEditor->GetSelectionStart();
    const int selectionEnd = m_textEditor->GetSelectionEnd();
    const int searchStart = options.backwards ? selectionStart : selectionEnd;
    const int searchEnd = options.backwards ? 0 : documentEnd;
    const int flags = GetScintillaSearchFlags(options);

    int matchEnd = wxSTC_INVALID_POSITION;
    int matchStart = m_textEditor->FindText(searchStart, searchEnd, text, flags, &matchEnd);
    if (matchStart == wxSTC_INVALID_POSITION && options.wrap) {
        const int wrapStart = options.backwards ? documentEnd : 0;
        matchStart = m_textEditor->FindText(wrapStart, searchStart, text, flags, &matchEnd);
    }

    if (matchStart == wxSTC_INVALID_POSITION) {
        return false;
    }

    m_textEditor->SetSelection(matchStart, matchEnd);
    m_textEditor->EnsureCaretVisible();
    return true;
}

bool ScintillaTextEditorPanel::ReplaceText(const wxString& text, const wxString& replacement,
                                           const TextSearchOptions& options) {
    if (text.IsEmpty()) {
        return false;
    }

    const int selectionStart = m_textEditor->GetSelectionStart();
    const int selectionEnd = m_textEditor->GetSelectionEnd();
    int matchEnd = wxSTC_INVALID_POSITION;
    const int matchStart =
        m_textEditor->FindText(selectionStart, selectionEnd, text, GetScintillaSearchFlags(options), &matchEnd);
    const bool selectionIsMatch = matchStart == selectionStart && matchEnd == selectionEnd;
    if (!selectionIsMatch) {
        return FindText(text, options);
    }

    m_textEditor->ReplaceSelection(replacement);
    if (options.backwards) {
        // ReplaceSelection leaves the caret after the replacement. Searching
        // backwards from there could immediately select text just inserted by
        // this replacement, so continue from the replaced range's start.
        m_textEditor->SetEmptySelection(selectionStart);
    }
    FindText(text, options);
    return true;
}

int ScintillaTextEditorPanel::ReplaceAllText(const wxString& text, const wxString& replacement,
                                             const TextSearchOptions& options) {
    if (text.IsEmpty()) {
        return 0;
    }

    int replacementCount = 0;
    m_textEditor->BeginUndoAction();
    m_textEditor->SetSearchFlags(GetScintillaSearchFlags(options));
    m_textEditor->SetTargetStart(0);
    m_textEditor->SetTargetEnd(m_textEditor->GetLength());

    while (m_textEditor->SearchInTarget(text) != wxSTC_INVALID_POSITION) {
        m_textEditor->ReplaceTarget(replacement);
        ++replacementCount;
        m_textEditor->SetTargetStart(m_textEditor->GetTargetEnd());
        m_textEditor->SetTargetEnd(m_textEditor->GetLength());
    }

    m_textEditor->EndUndoAction();
    return replacementCount;
}

void ScintillaTextEditorPanel::SetWordWrap(bool enabled) {
    const bool editorHadFocus = ::ContainsFocus(this);
    m_textEditor->SetWrapMode(enabled ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
    if (editorHadFocus) {
        FocusEditor();
    }
}

bool ScintillaTextEditorPanel::IsWordWrapEnabled() const {
    return m_textEditor->GetWrapMode() != wxSTC_WRAP_NONE;
}

void ScintillaTextEditorPanel::SetEditorFont(const wxFont& font) {
    ApplyEditorStyles(font);
    UpdateLineNumberMarginWidth();
}

wxFont ScintillaTextEditorPanel::GetEditorFont() const {
    return m_textEditor->StyleGetFont(wxSTC_STYLE_DEFAULT);
}

void ScintillaTextEditorPanel::SetEditorPointSize(int pointSize) {
    m_textEditor->SetZoom(pointSize - m_textEditor->StyleGetSize(wxSTC_STYLE_DEFAULT));
}

int ScintillaTextEditorPanel::GetEditorPointSize() const {
    return m_textEditor->StyleGetSize(wxSTC_STYLE_DEFAULT) + m_textEditor->GetZoom();
}

void ScintillaTextEditorPanel::FocusEditor() {
    m_textEditor->SetFocus();
}

void ScintillaTextEditorPanel::RequestDocumentWatch() const {
    if (m_callbacks.documentWatchRequested) {
        m_callbacks.documentWatchRequested(m_currentFile);
    }
}

void ScintillaTextEditorPanel::HandleDocumentWatcherChange() {
    if (!m_currentFile.IsOk()) {
        return;
    }
    if (!m_currentFile.FileExists()) {
        NotifyStatusChanged(Status::FileRemoved);
        return;
    }

    wxString onDisk;
    if (!ReadFileUtf8(m_currentFile, onDisk)) {
        printError("[Watcher] Could not read changed file: {}", m_currentFile.GetFullPath());
        return;
    }

    // Ignore an echo of our own write, or a repeated event for text already
    // accepted as the new disk baseline.
    if (onDisk == m_loadedText) {
        return;
    }

    if (HasUnsavedChanges()) {
        NotifyStatusChanged(Status::FileChangedWithUnsavedEdits);
        return;
    }

    m_loadedText = onDisk;
    LoadText(onDisk, LoadBehavior::KeepPosition);
    NotifyStatusChanged(Status::Reloading);
    NotifyDocumentChanged(ChangeReason::Reloaded, onDisk);
}

void ScintillaTextEditorPanel::NotifyDocumentChanged(ChangeReason reason, const wxString& text,
                                                     bool diskEntryChanged) const {
    if (m_callbacks.documentChanged) {
        m_callbacks.documentChanged(
            {.reason = reason, .diskEntryChanged = diskEntryChanged, .text = text, .filePath = m_currentFile});
    }
}

void ScintillaTextEditorPanel::NotifyStatusChanged(Status status) const {
    if (m_callbacks.statusChanged) {
        m_callbacks.statusChanged(status, m_currentFile);
    }
}

void ScintillaTextEditorPanel::NotifyError(ErrorCode errorCode, const wxFileName& filePath) const {
    if (m_callbacks.onError) {
        m_callbacks.onError(errorCode, filePath);
    }
}

void ScintillaTextEditorPanel::ApplyEditorStyles(const wxFont& font) {
    const wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    m_textEditor->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);
    m_textEditor->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textEditor->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    m_textEditor->StyleClearAll();
    m_textEditor->SetCaretForeground(foreground);

    if (m_options.syntax == Syntax::Markdown) {
        ApplyMarkdownStyles();
    } else if (m_options.syntax != Syntax::None) {
        ApplySourceStyles();
    }

    if (m_options.lineNumbers) {
        m_textEditor->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

void ScintillaTextEditorPanel::ApplySourceStyles() {
    const auto apply = [this](const std::vector<SyntaxStyle>& styles) {
        for (const auto& style : styles) {
            m_textEditor->StyleSetForeground(style.id, style.colour);
        }
    };
    apply(GetSyntaxStyle(m_options.syntax));
}

void ScintillaTextEditorPanel::ApplyMarkdownStyles() {
    const wxColour mutedColour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
    const wxColour headerColour(0, 0, 200);
    const wxColour codeColour(160, 30, 30);
    const wxColour linkColour(0, 100, 200);
    const wxColour listColour(200, 100, 0);

    for (int style = wxSTC_MARKDOWN_HEADER1; style <= wxSTC_MARKDOWN_HEADER6; ++style) {
        m_textEditor->StyleSetForeground(style, headerColour);
        m_textEditor->StyleSetBold(style, true);
    }

    m_textEditor->StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
    m_textEditor->StyleSetBold(wxSTC_MARKDOWN_STRONG2, true);
    m_textEditor->StyleSetItalic(wxSTC_MARKDOWN_EM1, true);
    m_textEditor->StyleSetItalic(wxSTC_MARKDOWN_EM2, true);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODE, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODE2, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODEBK, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_BLOCKQUOTE, mutedColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_LINK, linkColour);
    m_textEditor->StyleSetUnderline(wxSTC_MARKDOWN_LINK, true);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_ULIST_ITEM, listColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_OLIST_ITEM, listColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_HRULE, mutedColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_STRIKEOUT, mutedColour);
}

void ScintillaTextEditorPanel::ApplySelectionColours() {
#ifndef __WXOSX__
    // Cocoa supplies Scintilla's native selection appearance. Overriding its
    // text colour splits selected text into separate shaped runs, which can
    // make glyphs appear to shift as the selection changes.
    m_textEditor->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_textEditor->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
#endif
}

void ScintillaTextEditorPanel::UpdateEolModeForText(const wxString& text) {
    if (text.Find("\r\n") != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
    } else if (text.Find('\n') != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_LF);
    } else if (text.Find('\r') != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_CR);
    } else {
#ifdef __WXMSW__
        m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
#else
        m_textEditor->SetEOLMode(wxSTC_EOL_LF);
#endif
    }
}

void ScintillaTextEditorPanel::UpdateLineNumberMarginWidth() {
    if (!m_options.lineNumbers) {
        return;
    }

    int digitCount = 1;
    for (int lineCount = std::max(1, m_textEditor->GetLineCount()); lineCount >= 10; lineCount /= 10) {
        ++digitCount;
    }

    // Three digits avoids constant resizing in small documents; larger files
    // grow and shrink the margin as their line-count digit count changes.
    digitCount = std::max(3, digitCount);
    wxString widthTemplate("_");
    for (int digit = 0; digit < digitCount; ++digit) {
        widthTemplate += '9';
    }

    const int requiredWidth = m_textEditor->TextWidth(wxSTC_STYLE_LINENUMBER, widthTemplate);
    if (m_textEditor->GetMarginWidth(0) != requiredWidth) {
        m_textEditor->SetMarginWidth(0, requiredWidth);
    }
}

void ScintillaTextEditorPanel::HandleEditorMetricsChanged(wxStyledTextEvent& event) {
    UpdateLineNumberMarginWidth();
    event.Skip();
}

void ScintillaTextEditorPanel::HandleEditorMenuOpen(wxMenuEvent& event) {
    if (wxMenu* menu = event.GetMenu()) {
        m_callbacks.onEditorContextMenuOpening(*menu);
    }
    event.Skip();
}

void ScintillaTextEditorPanel::HandleSystemColourChanged(wxSysColourChangedEvent& event) {
    ApplyEditorStyles(GetEditorFont());
    ApplySelectionColours();
    UpdateLineNumberMarginWidth();
    event.Skip();
}
