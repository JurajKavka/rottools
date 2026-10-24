#include "Icons.h"
#include "IconsData.h"

#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/debug.h>
#include <wx/gdicmn.h>

namespace rottools::ui::icons {

namespace {

const wxSize kSize16(16, 16);

const wxSize& GetWxSize(Size size)
{
    switch (size)
    {
        case Size::Px16:
            return kSize16;
    }

    wxFAIL_MSG("Unsupported icon size");
    return kSize16;
}

wxBitmap MakeBitmap(const char* svg, Size size, const wxArtID& fallbackId, const wxArtClient& fallbackClient)
{
    const wxSize& bitmapSize = GetWxSize(size);
#ifdef wxHAS_SVG
    if (svg != nullptr && svg[0] != '\0')
    {
        const wxBitmap bitmap = wxBitmapBundle::FromSVG(svg, bitmapSize).GetBitmap(bitmapSize);
        if (bitmap.IsOk())
        {
            return bitmap;
        }
    }
#endif
    return wxArtProvider::GetBitmap(fallbackId, fallbackClient, bitmapSize);
}

}  // namespace

wxBitmap File(Size size)
{
    switch (size)
    {
        case Size::Px16:
        {
            static const wxBitmap bitmap =
                MakeBitmap(detail::kFileSvg, Size::Px16, wxART_NORMAL_FILE, wxART_OTHER);
            return bitmap;
        }
    }

    wxFAIL_MSG("Unsupported file icon size");
    return {};
}

wxBitmap Folder(Size size)
{
    switch (size)
    {
        case Size::Px16:
        {
            static const wxBitmap bitmap =
                MakeBitmap(detail::kFolderSvg, Size::Px16, wxART_FOLDER, wxART_OTHER);
            return bitmap;
        }
    }

    wxFAIL_MSG("Unsupported folder icon size");
    return {};
}

}  // namespace rottools::ui::icons
