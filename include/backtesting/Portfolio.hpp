// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need Trade, which is declared in Types.hpp.
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for asset counts.
#include <cstddef>

// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Portfolio owns the accounting state during a backtest.
// This class is encapsulated: cash and holdings are private, and
// outside code must use public methods to inspect value or request trades.
class Portfolio {
public:
    // We create a portfolio with starting cash and a number of assets.
    Portfolio(double initial_capital, std::size_t asset_count);

    // We invest all starting capital into the target weights on day 0.
    // This function does not return a value (void).
    void invest_to_target(
        const std::vector<double>& prices,
        const std::vector<double>& target_weights
    );

    // We mark the portfolio to market using the latest prices.
    // Const because it reads state but does not change holdings.
    double value(const std::vector<double>& prices) const;

    // Return current portfolio weights using latest prices.
    // Const because it reads state but does not change values.
    // This is useful for reporting final allocations and stress tests.
    std::vector<double> weights(const std::vector<double>& prices) const;

    // This function rebalances current holdings back to target weights.
    // It also returns a Trade log so the reporting module can show exactly
    // what was bought or sold.
    std::vector<Trade> rebalance(
        const std::string& date,
        const std::string& strategy_name,
        const std::vector<std::string>& assets,
        const std::vector<double>& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    );

private:
    // Private state enforces encapsulation.
    // cash_ stores cash, and holdings_ stores the number of shares held for each asset.
    double cash_;
    std::vector<double> holdings_;
};
