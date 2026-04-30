// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need PriceRow, which is declared in Types.hpp.
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for row and asset counts.
#include <cstddef>

// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Small shared dependency used by the backtest engine.
// It stores already-cleaned historical prices in a simple date-by-date table:
// rows are dates, columns are assets, and each cell is a price.
// The public methods below let other components read the data safely.
// The private member variables below prevent outside code from changing the table directly.
class PriceTable {
public:
    // This constructor validates the table's shape and price values.
    // It takes two inputs: a vector of asset tickers and a vector of PriceRow objects.
    // Here we only declare the constructor; we define it in PriceTable.cpp.
    PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows);

    // Asset tickers in the same order as every row's price vector.
    // We declare a function called assets() that returns a reference to the vector of asset tickers.
    // 1. std::vector<std::string>: a list of strings.
    // 2. &: return by reference, avoiding an expensive copy.
    // 3. const: the caller is not allowed to modify the returned vector.
    const std::vector<std::string>& assets() const;

    // Number of historical dates (rows)
    // We declare a function called row_count() that returns the number of rows in the price table.
    std::size_t row_count() const;

    // Number of assets (columns)
    std::size_t asset_count() const;

    // Date label for one row.
    // We declare a function called date() that takes a row index and returns the date string for that row.
    const std::string& date(std::size_t row) const;

    // Prices for all assets on one row.
    // We declare a function called prices() that takes a row index and returns a reference to the vector of prices for that row.
    const std::vector<double>& prices(std::size_t row) const;

private:
    // Outside code cannot modify this data directly because it is private,
    // but the class's own functions can access it. This is encapsulation.

    // This variable stores the asset tickers in the same order as every row's price vector.
    std::vector<std::string> assets_;
    // This variable stores the historical price data. Each PriceRow contains one date
    // and one vector of prices for all assets on that date.
    std::vector<PriceRow> rows_;
};
