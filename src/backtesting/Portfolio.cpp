#include "backtesting/Portfolio.hpp"

// Gives std::fabs to calculate absolute value.
#include <cmath>

// Gives std::runtime_error for throwing exceptions when the input data is invalid.
#include <stdexcept>

// Here we define the constructor and member functions of the Portfolio
// class declared in Portfolio.hpp.
Portfolio::Portfolio(double initial_capital, std::size_t asset_count)
    : cash_(initial_capital), holdings_(asset_count, 0.0) {
    // Sets cash = initial capital and creates one holding for each asset, initialized to 0.0 shares.

    // A portfolio must start with positive capital.
    if (initial_capital <= 0.0) {
        throw std::runtime_error("Initial capital must be positive.");
    }
}

// This function invests all starting capital into the target weights on day 0.
// It does not return a value (void).
void Portfolio::invest_to_target(
    const std::vector<double>& prices,
    const std::vector<double>& target_weights
) {
    // The number of prices and target weights must match the number of assets in the portfolio.
    if (prices.size() != holdings_.size() || target_weights.size() != holdings_.size()) {
        throw std::runtime_error("Initial investment dimensions do not match.");
    }

    // We calculate the starting value of the portfolio, which is just the initial cash.
    const double starting_value = cash_;

    // We loop through each asset, convert the target weight into dollars, then convert dollars into shares,
    // and update cash by subtracting the dollars invested.
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Convert target weight into dollars.
        const double dollars = starting_value * target_weights[i];

        // Convert dollars into shares.
        holdings_[i] = dollars / prices[i];

        // Buying the position uses cash.
        cash_ -= dollars;
    }
}

// We mark the portfolio to market using the latest prices.
// Const because it reads state but does not change holdings.
double Portfolio::value(const std::vector<double>& prices) const {
    if (prices.size() != holdings_.size()) {
        throw std::runtime_error("Portfolio value dimensions do not match.");
    }

    // Portfolio value equals cash plus marked-to-market position values.
    double total = cash_;
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        total += holdings_[i] * prices[i];
    }
    // Portfolio value = cash + sum of (shares held in each asset * price of each asset).
    return total;
}

// Return current portfolio weights using latest prices.
// Const because it reads state but does not change values.
// This is useful for reporting final allocations and stress tests.
std::vector<double> Portfolio::weights(const std::vector<double>& prices) const {
    const double total = value(prices);

    // We initialize the output vector with zeros in case the portfolio value is zero or negative.
    std::vector<double> output(holdings_.size(), 0.0);

    // If the portfolio value is zero or negative, we cannot calculate meaningful weights.
    if (total <= 0.0) {
        return output;
    }

    // Otherwise, we calculate weights as (shares * price) / total value.
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        output[i] = holdings_[i] * prices[i] / total;
    }

    // We return the vector of weights.
    return output;
}


// This function rebalances current holdings back to target weights.
// It also returns a Trade log so the reporting module can show exactly
// what was bought or sold.
std::vector<Trade> Portfolio::rebalance(
    const std::string& date,
    const std::string& strategy_name,
    const std::vector<std::string>& assets,
    const std::vector<double>& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) {
    // The number of prices, target weights, and asset names must match the number of assets in the portfolio.
    if (
        prices.size() != holdings_.size() ||
        target_weights.size() != holdings_.size() ||
        assets.size() != holdings_.size()
    ) {
        throw std::runtime_error("Rebalance dimensions do not match.");
    }

    // We calculate the current portfolio value before trading to use for calculating trade sizes and turnover.
    const double value_before = value(prices);
    if (value_before <= 0.0) {
        throw std::runtime_error("Portfolio value is non-positive.");
    }

    // Create a vector of current dollar values for each asset (shares held * price).
    std::vector<double> current_dollars(holdings_.size(), 0.0);
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        current_dollars[i] = holdings_[i] * prices[i];
    }

    // We do NOT invest the full portfolio value during a rebalance.
    // To keep the accounting simple we reserve 1% of the
    // portfolio value as cash and only rebalance the remaining 99%.
    // Example:
    //   value_before      = 100,000
    //   cash_reserve_rate = 0.01
    //   investable_value  = 100,000 * (1 - 0.01) = 99,000
    // The reserved cash helps pay transaction costs and prevents the portfolio
    // from becoming over-invested.
    const double cash_reserve_rate = 0.01;
    const double investable_value = value_before * (1.0 - cash_reserve_rate);

    // We loop through each asset, calculate the desired dollar exposure based on target weights,
    // calculate the dollar difference from current exposure, convert that into shares to buy or sell,
    // update cash and holdings, and record the trade details in a Trade struct.
    std::vector<Trade> trades;
    // We reserve space in the trades vector to avoid multiple reallocations
    // in case we trade every asset. This is an optimization to improve performance.
    trades.reserve(holdings_.size());

    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Desired exposure after reserving cash for transaction costs.
        const double target_dollars = investable_value * target_weights[i];

        // Dollar difference between current and target exposure.
        const double delta_dollars = target_dollars - current_dollars[i];

        // If the dollar difference is very small, we skip trading to avoid unnecessary transaction costs.
        if (std::fabs(delta_dollars) < 1e-8) {
            continue;
        }

        // Convert dollar difference into shares to buy or sell.
        const double trade_shares = delta_dollars / prices[i];
        // Calculate transaction cost for this trade.
        const double cost = std::fabs(delta_dollars) * transaction_cost_rate;

        // Update accounting state.
        holdings_[i] += trade_shares;
        cash_ -= delta_dollars;
        cash_ -= cost;

        // Record this trade for reporting.
        Trade trade;
        trade.date = date;
        trade.strategy = strategy_name;
        trade.asset = assets[i];
        trade.shares = trade_shares;
        trade.notional = delta_dollars;
        trade.cost = cost;
        trade.turnover = std::fabs(delta_dollars) / value_before;
        trades.push_back(trade);
    }

    // After processing all trades, we check if cash is negative beyond a small tolerance.
    // If so, we throw an exception because the rebalance logic produced an invalid state.
    // If cash is slightly negative due to rounding or transaction costs, we clamp it to zero.
    if (cash_ < -1e-6) {
        throw std::runtime_error("Rebalance produced negative cash.");
    }
    if (cash_ < 0.0) {
        cash_ = 0.0;
    }

    // We return the vector of trades executed during this rebalance.
    return trades;
}
