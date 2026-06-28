#pragma once

#include "HelperFunctions.h"
#include "FileBrowserTreePanelWx.h"

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    explicit FileBrowserTreePanel(wxWindow* parent);
    ~FileBrowserTreePanel();
};
