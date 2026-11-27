#pragma once
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

struct CsvResult {
    std::vector<std::vector<std::string>> rows;
    std::size_t columnCount = 0;
};

struct CsvError {
    enum class Code { InvalidSyntax, DisplayLimitExceeded };

    Code code;
    std::size_t recordNumber;  // One-based CSV record number.
};

class CsvParser {
   public:
    using OnSuccessCallback = std::function<void(const CsvResult&)>;
    using OnErrorCallback = std::function<void(const CsvError&)>;

    struct Options {
        bool limitForDisplay = false;
        std::size_t maxColumns = 256;
        std::size_t maxRows = 10000;
        std::size_t maxCells = 200000;
    };

    CsvParser(OnSuccessCallback onSuccess, OnErrorCallback onError);
    CsvParser(const Options& options, OnSuccessCallback onSuccess, OnErrorCallback onError);

    // Calls exactly one of the constructor's handlers synchronously. Its payload
    // is valid for the duration of the call to that handler.
    void Parse(std::string_view text) const;

   private:
    Options m_options;
    OnSuccessCallback m_onSuccess;
    OnErrorCallback m_onError;
};
