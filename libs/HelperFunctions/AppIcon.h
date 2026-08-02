#pragma once

#include <wx/iconbndl.h>

#include <cstddef>

namespace rottools {

/**
 * @brief One PNG image compiled into the executable.
 *
 * The arrays are produced by rottools_embed_window_icon() (cmake/RotToolsIcons.cmake)
 * from the app's generated icon set; see docs/graphics/README.md.
 */
struct EmbeddedPng {
    const unsigned char* data;
    std::size_t size;
};

/**
 * @brief Build the window-icon set from embedded PNGs.
 *
 * The icons are compiled in rather than read from disk because there is no one
 * path that works everywhere: a Linux install puts them under
 * share/icons/hicolor, a portable .tar.gz does not, and a plain build tree has
 * no installed files at all.
 *
 * Pass the result to wxTopLevelWindow::SetIcons. On macOS the window icon comes
 * from the .app bundle instead, so there is nothing to set there.
 *
 * @param pngs  Array of embedded PNGs, one per size.
 * @param count Number of entries in @p pngs.
 * @return The icon bundle; empty if none of the PNGs could be decoded.
 */
wxIconBundle MakeIconBundle(const EmbeddedPng* pngs, std::size_t count);

}  // namespace rottools
