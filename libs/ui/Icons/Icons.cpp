#include "Icons.h"
#include "IconsData.h"

#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/gdicmn.h>

namespace rottools::ui::icons {

namespace {

wxSize GetWxSize(Size size)
{
    const int dimension = static_cast<int>(size);
    return {dimension, dimension};
}

wxBitmap MakeBitmap(const char* svg, Size size, const wxArtID& fallbackId, const wxArtClient& fallbackClient)
{
    const wxSize bitmapSize = GetWxSize(size);
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

wxBitmap DefaultFile(Size size)
{
    static const wxBitmap bitmap = MakeBitmap(detail::kDefaultFileSvg, size, wxART_NORMAL_FILE, wxART_OTHER);
    return bitmap;
}

wxBitmap MarkdownFile(Size size)
{
    static const wxBitmap bitmap = MakeBitmap(detail::kMarkdownFileSvg, size, wxART_NORMAL_FILE, wxART_OTHER);
    return bitmap;
}

wxBitmap Folder(Size size)
{
    static const wxBitmap bitmap = MakeBitmap(detail::kFolderSvg, size, wxART_FOLDER, wxART_OTHER);
    return bitmap;
}

}  // namespace rottools::ui::icons
