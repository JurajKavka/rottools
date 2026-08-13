#include "ApplicationConfig.h"

#include <wx/config.h>
#include <wx/string.h>

namespace rottools {

wxFont ApplicationConfig::LoadEditorFont(const wxFont& fallback) {
    wxString nativeFontInfo;
    if (!wxConfigBase::Get()->Read(kEditorFontSetting, &nativeFontInfo)) {
        return fallback;
    }

    wxFont font;
    if (!font.SetNativeFontInfo(nativeFontInfo) || !font.IsOk()) {
        return fallback;
    }

    return font;
}

void ApplicationConfig::SaveEditorFont(const wxFont& font) {
    wxConfigBase* config = wxConfigBase::Get();
    if (config->Write(kEditorFontSetting, font.GetNativeFontInfoDesc())) {
        config->Flush();
    }
}

}  // namespace rottools
