#pragma once
#include <wx/panel.h>

#include <functional>
#include <string_view>

#include "CsvParser.h"

class CsvTablePanel : public wxPanel {
   public:
    using OnErrorCallback = std::function<void(const CsvResult&)>;
    CsvTablePanel(wxWindow* parent, std::string_view text, OnErrorCallback onError = {});
    bool IsLoaded() const { return m_loaded; }

   private:
    bool m_loaded = false;
};
