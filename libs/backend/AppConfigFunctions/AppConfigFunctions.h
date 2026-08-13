#pragma once

#include <wx/font.h>

namespace rottools {

/** Load the persisted editor font, or return fallback if none is valid. */
[[nodiscard]] wxFont LoadEditorFont(const wxFont& fallback);

/** Persist the editor font in the current application's settings store. */
void SaveEditorFont(const wxFont& font);

}  // namespace rottools
