#include <iostream>

#include "CsvParser.h"

int main() {
    const CsvParser parser;
    const auto csv = parser.Parse("00123,\"a,b\",\"two\nlines\",\"say \"\"hi\"\"\",\r\nx,y\n");
    if (csv.errorRecord || csv.rows.size() != 2 || csv.rows[0].size() != 5 || csv.rows[0][0] != "00123" ||
        csv.rows[0][1] != "a,b" || csv.rows[0][2] != "two\nlines" || csv.rows[0][3] != "say \"hi\"" ||
        !csv.rows[0][4].empty()) {
        std::cerr << "CSV field parsing check failed\n";
        return 1;
    }
    if (parser.Parse("ok\n\"bad").errorRecord != 2 || parser.Parse("a\"b").errorRecord != 1) {
        std::cerr << "CSV error record check failed\n";
        return 1;
    }
    std::cout << "CSV parser checks passed\n";
}
