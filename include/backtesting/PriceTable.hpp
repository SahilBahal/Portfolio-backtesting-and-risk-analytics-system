#pragma once

#include "backtesting/Types.hpp"

#include <cstddef>
#include <string>
#include <vector>

// Small shared dependency used by Manuel's backtest engine.
// It stores already-cleaned historical prices in a simple date-by-date table.
class PriceTable {
public:
    PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows);

    // Asset tickers in the same order as every row's price vector.
    const std::vector<std::string>& assets() const;

    // Number of historical dates.
    std::size_t row_count() const;

    // Number of asset columns.
    std::size_t asset_count() const;

    // Date label for one row.
    const std::string& date(std::size_t row) const;

    // Prices for all assets on one row.
    const std::vector<double>& prices(std::size_t row) const;

private:
    std::vector<std::string> assets_;
    std::vector<PriceRow> rows_;
};

