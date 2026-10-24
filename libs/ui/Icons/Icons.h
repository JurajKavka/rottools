#pragma once

#include <wx/bitmap.h>

namespace rottools::ui::icons {

/** Supported square icon sizes in logical pixels. */
enum class Size { Px16 = 16 };

/** File icon with a platform-native file fallback. */
[[nodiscard]] wxBitmap File(Size size = Size::Px16);

/** Folder icon with a platform-native folder fallback. */
[[nodiscard]] wxBitmap Folder(Size size = Size::Px16);

}  // namespace rottools::ui::icons
