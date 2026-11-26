#include <cassert>
#include <iostream>

#include "CsvParser.h"

int main() {
    const auto csv = CsvParser().Parse("00123,\"a,b\",\"two\nlines\",\"say \"\"hi\"\"\",\r\nx,y\n");
    assert(!csv.errorRecord && csv.rows.size() == 2 && csv.rows[0].size() == 5);
    assert(csv.rows[0][0] == "00123" && csv.rows[0][1] == "a,b");
    assert(csv.rows[0][2] == "two\nlines" && csv.rows[0][3] == "say \"hi\"" && csv.rows[0][4].empty());
    assert(CsvParser().Parse("ok\n\"bad").errorRecord == 2);
    assert(CsvParser().Parse("a\"b").errorRecord == 1);
    std::cout << "CSV parser checks passed\n";
}
