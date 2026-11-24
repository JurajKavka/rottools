#include "CsvTablePanel.h"

#include <wx/dataview.h>
#include <wx/sizer.h>

#include <algorithm>

namespace {
std::size_t ColumnCount(const CsvResult& csv) {
    std::size_t count = 0;
    for (const auto& row : csv.rows) {
        count = std::max(count, row.size());
    }
    return count;
}
}  // namespace

bool CsvTablePanel::Fits(const CsvResult& csv) {
    const std::size_t columns = ColumnCount(csv);
    return !csv.displayLimitExceeded && columns <= 256 && csv.rows.size() <= 10000 &&
           (columns == 0 || csv.rows.size() <= 200000 / columns);
}

CsvTablePanel::CsvTablePanel(wxWindow* parent, const CsvResult& csv) : wxPanel(parent) {
    auto* table = new wxDataViewListCtrl(this, wxID_ANY);
    const auto columns = ColumnCount(csv);
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
