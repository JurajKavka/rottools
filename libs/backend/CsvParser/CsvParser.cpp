#include "CsvParser.h"

#include <algorithm>
#include <utility>

CsvParser::CsvParser() = default;

CsvParser::CsvParser(const Options& options) : m_options(options) {}

CsvResult CsvParser::Parse(std::string_view text) const {
    CsvResult result;
    std::vector<std::string> row;
    std::string field;
    enum class State { Start, Unquoted, Quoted, Closed };
    State state = State::Start;
    bool pending = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        pending = true;
        if (state == State::Quoted) {
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
            if (m_options.limitForDisplay && (result.columnCount > m_options.maxColumns || result.rows.size() >= m_options.maxRows ||
                                    result.rows.size() + 1 > m_options.maxCells / result.columnCount)) {
                result.displayLimitExceeded = true;
                return result;
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
            result.errorRecord = result.rows.size() + 1;
            return result;
        } else {
            field += c;
            state = State::Unquoted;
        }
    }
    if (state == State::Quoted) {
        result.errorRecord = result.rows.size() + 1;
    } else if (pending) {
        row.push_back(std::move(field));
        result.columnCount = std::max(result.columnCount, row.size());
        if (m_options.limitForDisplay && (result.columnCount > m_options.maxColumns || result.rows.size() >= m_options.maxRows ||
                                result.rows.size() + 1 > m_options.maxCells / result.columnCount)) {
            result.displayLimitExceeded = true;
            return result;
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}
