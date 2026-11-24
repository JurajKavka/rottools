#include "CsvParser.h"

#include <algorithm>
#include <utility>

CsvResult ParseCsv(std::string_view text, bool limitForDisplay) {
    CsvResult result;
    std::vector<std::string> row;
    std::string field;
    enum class State { Start, Unquoted, Quoted, Closed };
    State state = State::Start;
    bool pending = false;
    std::size_t maxColumns = 0;
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
            maxColumns = std::max(maxColumns, row.size());
            if (limitForDisplay && (maxColumns > 256 || result.rows.size() >= 10000 ||
                                    result.rows.size() + 1 > 200000 / maxColumns)) {
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
        maxColumns = std::max(maxColumns, row.size());
        if (limitForDisplay && (maxColumns > 256 || result.rows.size() >= 10000 ||
                                result.rows.size() + 1 > 200000 / maxColumns)) {
            result.displayLimitExceeded = true;
            return result;
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}
