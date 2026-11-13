#include "FileManagerClipboard.h"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>

#include <algorithm>
#include <utility>

namespace {
struct ClipboardState {
    std::vector<wxFileName> files;
    FileManagerClipboard::Operation operation = FileManagerClipboard::Operation::Copy;
    wxString token;
    unsigned long sequence = 0;
};

ClipboardState& State() {
    static ClipboardState state;
    return state;
}

const wxDataFormat& Format() {
    static const wxDataFormat format("application/x-rottools-filemanager-files");
    return format;
}
}  // namespace

namespace FileManagerClipboard {
bool Set(const std::vector<wxFileName>& paths, Operation operation) {
    if (paths.empty()) {
        return false;
    }
    wxClipboardLocker clipboard;
    if (!clipboard) {
        return false;
    }
    ClipboardState& state = State();
    const wxString token = wxString::Format("%p-%lu", static_cast<void*>(&state), ++state.sequence);
    const wxScopedCharBuffer bytes = token.utf8_str();
    auto* data = new wxCustomDataObject(Format());
    data->SetData(bytes.length(), bytes.data());
    if (!wxTheClipboard->SetData(data)) {
        return false;
    }
    state.files = paths;
    state.operation = operation;
    state.token = token;
    return true;
}

bool HasFiles() {
    wxClipboardLocker clipboard;
    const ClipboardState& state = State();
    if (!clipboard || state.files.empty() || !wxTheClipboard->IsSupported(Format())) {
        return false;
    }
    wxCustomDataObject data(Format());
    if (!wxTheClipboard->GetData(data)) {
        return false;
    }
    const wxScopedCharBuffer token = state.token.utf8_str();
    return data.GetData() && data.GetSize() == token.length() &&
           std::equal(static_cast<const char*>(data.GetData()),
                      static_cast<const char*>(data.GetData()) + data.GetSize(), token.data());
}

std::optional<Contents> GetFiles() {
    if (!HasFiles()) {
        return std::nullopt;
    }
    return Contents{State().files, State().operation};
}

void KeepFiles(std::vector<wxFileName> paths) {
    State().files = std::move(paths);
}
}  // namespace FileManagerClipboard
