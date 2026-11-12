#include "CutFilesClipboard.h"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>

#include <algorithm>
#include <utility>

namespace {
struct CutState {
    std::vector<wxFileName> files;
    wxString token;
    unsigned long sequence = 0;
};

CutState& State() {
    static CutState state;
    return state;
}

const wxDataFormat& Format() {
    static const wxDataFormat format("application/x-rottools-cut-files");
    return format;
}
}  // namespace

namespace CutFilesClipboard {
bool Set(const std::vector<wxFileName>& paths) {
    if (paths.empty()) {
        return false;
    }
    wxClipboardLocker clipboard;
    if (!clipboard) {
        return false;
    }
    CutState& state = State();
    const wxString token = wxString::Format("%p-%lu", static_cast<void*>(&state), ++state.sequence);
    const wxScopedCharBuffer bytes = token.utf8_str();
    auto* data = new wxCustomDataObject(Format());
    data->SetData(bytes.length(), bytes.data());
    if (!wxTheClipboard->SetData(data)) {
        return false;
    }
    state.files = paths;
    state.token = token;
    return true;
}

bool HasFiles() {
    wxClipboardLocker clipboard;
    const CutState& state = State();
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

std::vector<wxFileName> GetFiles() {
    return HasFiles() ? State().files : std::vector<wxFileName>{};
}

void KeepFiles(std::vector<wxFileName> paths) {
    State().files = std::move(paths);
}
}  // namespace CutFilesClipboard
