#include "AppIcon.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

namespace rottools {

wxIconBundle MakeIconBundle(const EmbeddedPng* pngs, std::size_t count) {
    // The caller may not have installed the PNG handler yet (a library demo app,
    // for one), and wxImage silently fails to decode without it.
    if (wxImage::FindHandler(wxBITMAP_TYPE_PNG) == nullptr) {
        wxImage::AddHandler(new wxPNGHandler());
    }

    wxIconBundle bundle;
    for (std::size_t i = 0; i < count; ++i) {
        wxMemoryInputStream stream(pngs[i].data, pngs[i].size);
        wxImage image(stream, wxBITMAP_TYPE_PNG);
        if (!image.IsOk()) {
            continue;
        }
        wxIcon icon;
        icon.CopyFromBitmap(wxBitmap(image));
        bundle.AddIcon(icon);
    }
    return bundle;
}

}  // namespace rottools
