#pragma once

#include "backtesting/Types.hpp"

#include <cstddef>
#include <string>
#include <vector>

// Portfolio owns the accounting state during a backtest.
// This class is intentionally encapsulated: cash and holdings are private, and
// outside code must use public methods to inspect value or request trades.
class Portfolio {
public:
    // Start with cash only and zero shares in every asset.
    Portfolio(double initial_capital, std::size_t asset_count);

    // Invest all starting capital into the target weights on day 0.
    // Fractional shares are allowed to keep the project focused on portfolio
    // logic rather than broker-specific order-size rules.
    void invest_to_target(
        const std::vector<double>& prices,
        const std::vector<double>& target_weights
    );

    // Mark the portfolio to market using the latest prices.
    // This method is const because it reads state but does not change holdings.
    double value(const std::vector<double>& prices) const;

    // Return current portfolio weights using latest prices.
    // This is useful for reporting final allocations and stress tests.
    std::vector<double> weights(const std::vector<double>& prices) const;

    // Rebalance current holdings back to target weights.
    // The method returns a trade log so the reporting module can show exactly
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
    double cash_;
    std::vector<double> holdings_;
};

