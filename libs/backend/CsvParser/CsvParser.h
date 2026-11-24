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
};

CsvResult ParseCsv(std::string_view text, bool limitForDisplay = false);
