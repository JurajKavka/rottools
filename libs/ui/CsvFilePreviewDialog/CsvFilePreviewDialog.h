#pragma once

#include <vector>

#include "CsvFilePreviewDialogWx.h"
#include "CsvParser.h"

/** Read-only table preview. The caller owns file loading. */
class CsvFilePreviewDialog final : public CsvFilePreviewDialogWx {
   public:
    explicit CsvFilePreviewDialog(wxWindow* parent, const wxString& title = wxEmptyString);

    void ShowCsvModal(const wxString& text, const CsvParser::OnErrorCallback& onError);

   private:
    void PopulateGrid(const CsvResult& csv);
    void HandleFirstLineAsHeaderChanged(wxCommandEvent& event);
    void SetFirstLineAsHeader(bool useHeader);
    void UpdateColumnLabels();

    std::vector<wxString> m_firstRecord;
    bool m_firstLineAsHeader = false;
    int m_defaultLabelHeight;
};
