#include <iostream>

#include "CsvParser.h"

int main() {
    int successCalls = 0;
    bool validCsv = false;
    bool unexpectedError = false;
    const auto HandleSuccess = [&](const CsvResult& csv) {
        ++successCalls;
        validCsv = csv.columnCount == 5 && csv.rows.size() == 2 && csv.rows[0].size() == 5 &&
                   csv.rows[0][0] == "00123" && csv.rows[0][1] == "a,b" && csv.rows[0][2] == "two\nlines" &&
                   csv.rows[0][3] == "say \"hi\"" && csv.rows[0][4].empty();
    };
    const auto HandleUnexpectedError = [&](const CsvError&) { unexpectedError = true; };
    const CsvParser parser(HandleSuccess, HandleUnexpectedError);
    parser.Parse("00123,\"a,b\",\"two\nlines\",\"say \"\"hi\"\"\",\r\nx,y\n");
    if (successCalls != 1 || !validCsv || unexpectedError) {
        std::cerr << "CSV field parsing check failed\n";
        return 1;
    }
    struct ErrorCase {
        std::string_view input;
        CsvError expectedError{};
    };
    const ErrorCase errorCases[] = {
        {"ok\n\"bad", {CsvError::Code::InvalidSyntax, 2}},
        {"a\"b\nx,y", {CsvError::Code::InvalidSyntax, 1}},
        {"a,b\n\"bad", {CsvError::Code::DisplayLimitExceeded, 1}},
        {"ok\na,b", {CsvError::Code::DisplayLimitExceeded, 2}},
    };
    std::size_t errorCalls = 0;
    bool unexpectedSuccess = false;
    CsvError reportedError{};
    const auto HandleUnexpectedSuccess = [&](const CsvResult&) { unexpectedSuccess = true; };
    const auto HandleError = [&](const CsvError& error) {
        ++errorCalls;
        reportedError = error;
    };
    const CsvParser limitedParser({.limitForDisplay = true, .maxColumns = 1}, HandleUnexpectedSuccess, HandleError);
    std::size_t expectedErrorCalls = 0;
    for (const auto& test : errorCases) {
        ++expectedErrorCalls;
        limitedParser.Parse(test.input);
        if (errorCalls != expectedErrorCalls || unexpectedSuccess || reportedError.code != test.expectedError.code ||
            reportedError.recordNumber != test.expectedError.recordNumber) {
            std::cerr << "CSV error callback check failed for input: " << test.input << '\n';
            return 1;
        }
    }
    std::cout << "CSV parser checks passed\n";
}
