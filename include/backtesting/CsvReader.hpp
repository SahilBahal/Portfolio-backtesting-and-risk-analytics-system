// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need PriceTable because this reader converts a CSV file into a PriceTable object.
#include "backtesting/PriceTable.hpp"

// Imports std::string to store the CSV file path.
#include <string>

// CsvReader is responsible for reading historical price data from a CSV file.
// It keeps file parsing separate from the backtest engine.
// This separation is important because BacktestEngine should only run simulations;
// it should not also know how to parse files.
class CsvReader {
public:
    // Read a wide price CSV and return a validated PriceTable.
    //
    // Expected CSV format:
    // date,SPY,TLT,GLD,BTC-USD
    // 2020-01-02,296.888245,114.166550,143.949997,6985.470215
    //
    // The first column must be the date.
    // Every other column is treated as one asset price series.
    static PriceTable read_price_table(const std::string& path);
};
