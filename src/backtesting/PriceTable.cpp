#include "backtesting/PriceTable.hpp"

// Gives std::isfinite to validate price values.
#include <cmath>
// Gives std::runtime_error for throwing exceptions when the input data is invalid.
#include <stdexcept>
// Gives std::move for efficiently moving input vectors into the class without copying.
#include <utility>

// Here we define the constructor and member functions of the PriceTable
// class declared in PriceTable.hpp.
PriceTable::PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows)
    : assets_(std::move(assets)), rows_(std::move(rows)) {
    // Here we move incoming assets and rows into the class's private member variables,
    // avoiding copies.

    // The table must contain at least one asset and enough
    // dates (2) to calculate a return.
    if (assets_.empty()) {
        throw std::runtime_error("PriceTable must contain at least one asset.");
    }
    if (rows_.size() < 2) {
        throw std::runtime_error("PriceTable must contain at least two rows.");
    }


    // Validate all prices once during construction so the backtest engine can
    // assume the table is clean.
    for (const auto& row : rows_) {
        // C++ infers the type, uses a reference to avoid copying, and does not modify the row.

        // Each row must have one price per asset.
        if (row.prices.size() != assets_.size()) {
            throw std::runtime_error("Price row has the wrong number of columns.");
        }
        // every price must be finite and greater than 0.
        for (double price : row.prices) {
            if (!std::isfinite(price) || price <= 0.0) {
                throw std::runtime_error("PriceTable contains an invalid price.");
            }
        }
    }
}

const std::vector<std::string>& PriceTable::assets() const {
    // Return a reference to the vector of asset tickers.
    return assets_;
}

std::size_t PriceTable::row_count() const {
    // Return the number of rows in the price table, meaning how many dates exist.
    return rows_.size();
}

std::size_t PriceTable::asset_count() const {
    // Return the number of assets in the price table.
    return assets_.size();
}

const std::string& PriceTable::date(std::size_t row) const {
    // Return the date at index row; .at() checks bounds and throws if row is out of range.
    return rows_.at(row).date;
}

const std::vector<double>& PriceTable::prices(std::size_t row) const {
    // Return the prices at index row; .at() checks bounds and throws if row is out of range.
    return rows_.at(row).prices;
}
