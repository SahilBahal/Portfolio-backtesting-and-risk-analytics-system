#include "backtesting/PriceTable.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

PriceTable::PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows)
    : assets_(std::move(assets)), rows_(std::move(rows)) {
    // Class invariant: the table must contain at least one asset and enough
    // dates to calculate a return.
    if (assets_.empty()) {
        throw std::runtime_error("PriceTable must contain at least one asset.");
    }
    if (rows_.size() < 2) {
        throw std::runtime_error("PriceTable must contain at least two rows.");
    }

    // Validate all prices once during construction so the backtest engine can
    // assume the table is clean.
    for (const auto& row : rows_) {
        if (row.prices.size() != assets_.size()) {
            throw std::runtime_error("Price row has the wrong number of columns.");
        }
        for (double price : row.prices) {
            if (!std::isfinite(price) || price <= 0.0) {
                throw std::runtime_error("PriceTable contains an invalid price.");
            }
        }
    }
}

const std::vector<std::string>& PriceTable::assets() const {
    return assets_;
}

std::size_t PriceTable::row_count() const {
    return rows_.size();
}

std::size_t PriceTable::asset_count() const {
    return assets_.size();
}

const std::string& PriceTable::date(std::size_t row) const {
    return rows_.at(row).date;
}

const std::vector<double>& PriceTable::prices(std::size_t row) const {
    return rows_.at(row).prices;
}

