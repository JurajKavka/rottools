#include "FileDropTarget.h"

FileDropTarget::FileDropTarget(DropCallback callback) : m_callback(callback) {}

bool FileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
    if (filenames.GetCount() > 0 && m_callback) {
        m_callback(filenames[0]);
        return true;
    }
    return false;
}
