#pragma once
#include <wx/panel.h>

#include "CsvParser.h"

class CsvTablePanel : public wxPanel {
   public:
    static bool Fits(const CsvResult& csv);
    CsvTablePanel(wxWindow* parent, const CsvResult& csv);
};
