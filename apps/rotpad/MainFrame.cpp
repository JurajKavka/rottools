#include "MainFrame.h"

#ifndef __WXOSX__
#include "AppIcon.h"
#include "AppIconData.h"
#endif

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
#ifndef __WXOSX__
    // macOS gets its window and Dock icon from AppIcon.icns in the bundle.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif
}
