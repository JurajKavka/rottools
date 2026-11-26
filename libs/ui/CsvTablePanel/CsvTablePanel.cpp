#include "CsvTablePanel.h"

#include <wx/dataview.h>
#include <wx/sizer.h>

CsvTablePanel::CsvTablePanel(wxWindow* parent, std::string_view text, OnErrorCallback onError) : wxPanel(parent) {
    const CsvResult csv = CsvParser({.limitForDisplay = true}).Parse(text);
    if (csv.errorRecord || csv.displayLimitExceeded) {
        if (onError) {
            onError(csv);
        }
        return;
    }
    m_loaded = true;
    auto* table = new wxDataViewListCtrl(this, wxID_ANY);
    const auto columns = csv.columnCount;
    for (std::size_t i = 0; i < columns; ++i) {
        table->AppendTextColumn(wxString::Format("%zu", i + 1), wxDATAVIEW_CELL_INERT, 120);
    }
    for (const auto& row : csv.rows) {
        wxVector<wxVariant> values;
        for (std::size_t i = 0; i < columns; ++i) {
            values.push_back(wxVariant(i < row.size() ? wxString::FromUTF8(row[i].c_str()) : wxString{}));
        }
        table->AppendItem(values);
    }
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(table, 1, wxEXPAND);
    SetSizer(sizer);
}
