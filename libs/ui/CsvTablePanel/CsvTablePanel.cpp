#include "CsvTablePanel.h"

#include <wx/dataview.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>

#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

namespace {
constexpr std::size_t kRowsToMeasure = 200;
constexpr int kMinColumnWidthDip = 96;
constexpr int kMaxContentWidthDip = 320;
constexpr int kCellPaddingDip = 24;
}  // namespace

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
    const int minimumWidth = table->FromDIP(kMinColumnWidthDip);
    const int maximumContentWidth = table->FromDIP(kMaxContentWidthDip);
    const int cellPadding = table->FromDIP(kCellPaddingDip);
    std::vector<int> columnWidths(columns, minimumWidth);
    for (std::size_t i = 0; i < columns; ++i) {
        auto* column = table->AppendTextColumn(wxString::Format("%zu", i + 1), wxDATAVIEW_CELL_INERT, minimumWidth);
        column->SetMinWidth(minimumWidth);
    }

    wxClientDC textDC(table);
    textDC.SetFont(table->GetFont());
    for (std::size_t rowIndex = 0; rowIndex < csv.rows.size(); ++rowIndex) {
        const auto& row = csv.rows[rowIndex];
        wxVector<wxVariant> values;
        for (std::size_t i = 0; i < columns; ++i) {
            const wxString value = i < row.size() ? wxString::FromUTF8(row[i].c_str()) : wxString{};
            if (rowIndex < kRowsToMeasure && !value.empty() && columnWidths[i] < maximumContentWidth) {
                const int width = textDC.GetMultiLineTextExtent(value).GetWidth() + cellPadding;
                columnWidths[i] = std::clamp(width, columnWidths[i], maximumContentWidth);
            }
            values.push_back(wxVariant(value));
        }
        table->AppendItem(values);
    }
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(table, 1, wxEXPAND);
    SetSizer(sizer);

    if (columns != 0) {
        // Wait for the parent to lay out the panel before sharing the available width.
        table->CallAfter([table, columnWidths = std::move(columnWidths)] {
            const int contentWidth = std::accumulate(columnWidths.begin(), columnWidths.end(), 0);
            const int spareWidth = std::max(0, table->GetClientSize().GetWidth() - contentWidth);
            const int extraPerColumn = spareWidth / static_cast<int>(columnWidths.size());
            for (std::size_t i = 0; i < columnWidths.size(); ++i) {
                table->GetColumn(static_cast<unsigned int>(i))->SetWidth(columnWidths[i] + extraPerColumn);
            }
        });
    }
}
