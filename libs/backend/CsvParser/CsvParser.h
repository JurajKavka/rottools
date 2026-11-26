#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct CsvResult {
    std::vector<std::vector<std::string>> rows;
    std::optional<std::size_t> errorRecord;
    bool displayLimitExceeded = false;
    std::size_t columnCount = 0;
};

class CsvParser {
   public:
    struct Options {
        bool limitForDisplay = false;
        std::size_t maxColumns = 256;
        std::size_t maxRows = 10000;
        std::size_t maxCells = 200000;
    };

    CsvParser();
    explicit CsvParser(Options options);
    CsvResult Parse(std::string_view text) const;

   private:
    Options m_options;
};
