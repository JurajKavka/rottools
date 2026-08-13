#pragma once

#include <wx/font.h>

namespace rottools {

/** Provides access to settings stored for the current application. */
class ApplicationConfig final {
   public:
    ApplicationConfig() = delete;

    /** Load the persisted editor font, or return fallback if none is valid. */
    [[nodiscard]] static wxFont LoadEditorFont(const wxFont& fallback);

    /** Persist the editor font in the current application's settings store. */
    static void SaveEditorFont(const wxFont& font);

   private:
    static constexpr auto kEditorFontSetting = "/editor/font";
};

}  // namespace rottools
