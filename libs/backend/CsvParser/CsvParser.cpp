#include "CsvParser.h"

#include <algorithm>
#include <utility>

CsvParser::CsvParser(OnSuccessCallback onSuccess, OnErrorCallback onError)
    : CsvParser({}, std::move(onSuccess), std::move(onError)) {}

CsvParser::CsvParser(const Options& options, OnSuccessCallback onSuccess, OnErrorCallback onError)
    : m_options(options), m_onSuccess(std::move(onSuccess)), m_onError(std::move(onError)) {}

void CsvParser::Parse(std::string_view text) const {
    CsvResult result;
    const auto ReportError = [this, &result](CsvError::Code code) { m_onError({code, result.rows.size() + 1}); };

    std::vector<std::string> row;
    std::string field;
    enum class State { Start, Unquoted, Quoted, Closed };
    State state = State::Start;
    bool pending = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        pending = true;
        if (state == State::Quoted) {
            // Inside a quoted field, commas and newlines are part of the field value.
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    state = State::Closed;
                }
            } else {
                field += c;
            }
        } else if (c == ',' || c == '\n' || (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n')) {
            row.push_back(std::move(field));
            result.columnCount = std::max(result.columnCount, row.size());
            if (m_options.limitForDisplay &&
                (result.columnCount > m_options.maxColumns || result.rows.size() >= m_options.maxRows ||
                 result.rows.size() + 1 > m_options.maxCells / result.columnCount)) {
                ReportError(CsvError::Code::DisplayLimitExceeded);
                return;
            }
            field.clear();
            state = State::Start;
            if (c != ',') {
                if (c == '\r') {
                    ++i;
                }
                result.rows.push_back(std::move(row));
                row.clear();
                pending = false;
            }
        } else if (state == State::Start && c == '"') {
            state = State::Quoted;
        } else if (state == State::Closed || c == '"' || c == '\r') {
            ReportError(CsvError::Code::InvalidSyntax);
            return;
        } else {
            field += c;
            state = State::Unquoted;
        }
    }
    if (state == State::Quoted) {
        // The input ended before the field's closing quote. Report the one-based
        // record number and skip appending the incomplete record.
        ReportError(CsvError::Code::InvalidSyntax);
        return;
    }
    if (pending) {
        row.push_back(std::move(field));
        result.columnCount = std::max(result.columnCount, row.size());
        if (m_options.limitForDisplay &&
            (result.columnCount > m_options.maxColumns || result.rows.size() >= m_options.maxRows ||
             result.rows.size() + 1 > m_options.maxCells / result.columnCount)) {
            ReportError(CsvError::Code::DisplayLimitExceeded);
            return;
        }
        result.rows.push_back(std::move(row));
    }
    m_onSuccess(result);
}
