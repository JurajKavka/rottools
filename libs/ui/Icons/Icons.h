#pragma once

#include <wx/bitmap.h>

namespace rottools::ui::icons {

/** Supported square icon sizes in logical pixels. */
enum class Size { Px16 = 16 };

/** Default file icon with a platform-native file fallback. */
[[nodiscard]] wxBitmap DefaultFile(Size size = Size::Px16);

/** Markdown file icon with a platform-native file fallback. */
[[nodiscard]] wxBitmap MarkdownFile(Size size = Size::Px16);

/** Folder icon with a platform-native folder fallback. */
[[nodiscard]] wxBitmap Folder(Size size = Size::Px16);

}  // namespace rottools::ui::icons
