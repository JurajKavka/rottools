#include "CsvFilePreviewDialog.h"

#include <wx/dcclient.h>

#include <algorithm>
#include <string_view>

namespace {
// Sample the first 200 records when sizing columns so large files open faster.
constexpr int kRowsToMeasure = 200;

// DIP means device-independent pixels. FromDIP scales these sizes for the display.
// Start columns at this width so empty and short fields still have space.
constexpr int kMinColumnWidthDip = 96;
// Cap the initial width so a long field cannot make a column excessively wide.
constexpr int kMaxContentWidthDip = 320;
// Add horizontal padding to the measured width of cell and header text.
constexpr int kCellPaddingDip = 24;
// Limit the height of multiline headers so the data rows still have room.
constexpr int kMaxLabelHeightDip = 96;
}  // namespace

CsvFilePreviewDialog::CsvFilePreviewDialog(wxWindow* parent, const wxString& title)
    : CsvFilePreviewDialogWx(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_defaultLabelHeight(m_grid1->GetColLabelSize()) {
    m_grid1->EnableEditing(false);
    auto* gridItem = GetSizer()->GetItem(m_grid1);
    gridItem->SetFlag(gridItem->GetFlag() | wxEXPAND);
    m_firstLineAsHeaderCheckBox->Bind(wxEVT_CHECKBOX, &CsvFilePreviewDialog::HandleFirstLineAsHeaderChanged, this);

    SetSize(FromDIP(wxSize(983, 610)));
    Layout();
    CentreOnParent();
}

void CsvFilePreviewDialog::ShowCsvModal(const wxString& text, const CsvParser::OnErrorCallback& onError) {
    const auto utf8 = text.ToUTF8();
    const auto input = utf8.length() == 0 ? std::string_view{} : std::string_view(utf8.data(), utf8.length());
    bool loaded = false;
    const auto HandleSuccess = [this, &loaded](const CsvResult& csv) {
        PopulateGrid(csv);
        loaded = true;
    };
    const CsvParser parser({.limitForDisplay = true}, HandleSuccess, onError);
    parser.Parse(input);
    if (loaded) {
        ShowModal();
    }
}

void CsvFilePreviewDialog::PopulateGrid(const CsvResult& csv) {
    const wxGridUpdateLocker updates(m_grid1);
    if (m_grid1->GetNumberRows() > 0) {
        m_grid1->DeleteRows(0, m_grid1->GetNumberRows());
    }
    if (m_grid1->GetNumberCols() > 0) {
        m_grid1->DeleteCols(0, m_grid1->GetNumberCols());
    }
    m_firstRecord.clear();
    m_firstLineAsHeader = false;
    m_firstLineAsHeaderCheckBox->Enable(!csv.rows.empty());
    if (csv.rows.empty()) {
        m_grid1->SetColLabelSize(m_defaultLabelHeight);
        return;
    }

    const int columns = static_cast<int>(csv.columnCount);
    const int rows = static_cast<int>(csv.rows.size());
    m_grid1->AppendCols(columns);
    m_grid1->AppendRows(rows);
    m_firstRecord.resize(columns);

    const int minimumWidth = m_grid1->FromDIP(kMinColumnWidthDip);
    const int maximumContentWidth = m_grid1->FromDIP(kMaxContentWidthDip);
    const int cellPadding = m_grid1->FromDIP(kCellPaddingDip);
    std::vector<int> columnWidths(columns, minimumWidth);
    wxClientDC textDC(m_grid1);
    textDC.SetFont(m_grid1->GetDefaultCellFont());
    for (int row = 0; row < rows; ++row) {
        const auto& fields = csv.rows[row];
        bool multiline = false;
        for (int col = 0; col < static_cast<int>(fields.size()); ++col) {
            const auto& field = fields[col];
            const wxString value = wxString::FromUTF8(field.data(), field.size());
            m_grid1->SetCellValue(row, col, value);
            multiline = multiline || field.find('\n') != std::string::npos;
            if (row == 0) {
                m_firstRecord[col] = value;
            }
            if (row < kRowsToMeasure && !value.empty() && columnWidths[col] < maximumContentWidth) {
                const int width = textDC.GetMultiLineTextExtent(value).GetWidth() + cellPadding;
                columnWidths[col] = std::clamp(width, columnWidths[col], maximumContentWidth);
            }
        }
        if (multiline) {
            m_grid1->AutoSizeRow(row, false);
        }
    }

    textDC.SetFont(m_grid1->GetLabelFont());
    for (int col = 0; col < columns; ++col) {
        const int headerWidth = textDC.GetMultiLineTextExtent(m_firstRecord[col]).GetWidth() + cellPadding;
        m_grid1->SetColSize(col, std::clamp(headerWidth, columnWidths[col], maximumContentWidth));
    }
    if (m_firstLineAsHeaderCheckBox->GetValue()) {
        SetFirstLineAsHeader(true);
    } else {
        UpdateColumnLabels();
    }
}

void CsvFilePreviewDialog::HandleFirstLineAsHeaderChanged(wxCommandEvent& event) {
    SetFirstLineAsHeader(event.IsChecked());
}

void CsvFilePreviewDialog::SetFirstLineAsHeader(bool useHeader) {
    if (m_firstRecord.empty() || m_firstLineAsHeader == useHeader) {
        return;
    }
    const wxGridUpdateLocker updates(m_grid1);
    m_firstLineAsHeader = useHeader;
    if (useHeader) {
        m_grid1->DeleteRows(0);
    } else {
        m_grid1->InsertRows(0);
        for (int col = 0; col < m_grid1->GetNumberCols(); ++col) {
            m_grid1->SetCellValue(0, col, m_firstRecord[col]);
        }
        m_grid1->AutoSizeRow(0, false);
    }
    UpdateColumnLabels();
}

void CsvFilePreviewDialog::UpdateColumnLabels() {
    wxClientDC labelDC(m_grid1);
    labelDC.SetFont(m_grid1->GetLabelFont());
    const int maximumHeight = std::max(m_defaultLabelHeight, FromDIP(kMaxLabelHeightDip));
    const int padding = FromDIP(4);
    int labelHeight = m_defaultLabelHeight;
    for (int col = 0; col < m_grid1->GetNumberCols(); ++col) {
        const auto& value = m_firstRecord[col];
        if (m_firstLineAsHeader && !value.empty()) {
            m_grid1->SetColLabelValue(col, value);
            const int height = labelDC.GetMultiLineTextExtent(value).GetHeight() + padding;
            labelHeight = std::clamp(height, labelHeight, maximumHeight);
        } else {
            m_grid1->SetColLabelValue(col, wxString::Format("%d", col + 1));
        }
    }
    m_grid1->SetColLabelSize(labelHeight);
}
