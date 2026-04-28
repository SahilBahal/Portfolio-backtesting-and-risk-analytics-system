#include "backtesting/Portfolio.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

Portfolio::Portfolio(double initial_capital, std::size_t asset_count)
    : cash_(initial_capital), holdings_(asset_count, 0.0) {
    // Class invariant: a portfolio must start with positive capital.
    if (initial_capital <= 0.0) {
        throw std::runtime_error("Initial capital must be positive.");
    }
}

void Portfolio::invest_to_target(
    const std::vector<double>& prices,
    const std::vector<double>& target_weights
) {
    if (prices.size() != holdings_.size() || target_weights.size() != holdings_.size()) {
        throw std::runtime_error("Initial investment dimensions do not match.");
    }

    const double starting_value = cash_;
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Convert target weight into dollars.
        const double dollars = starting_value * target_weights[i];

        // Convert dollars into shares.
        holdings_[i] = dollars / prices[i];

        // Buying the position uses cash.
        cash_ -= dollars;
    }
}

double Portfolio::value(const std::vector<double>& prices) const {
    if (prices.size() != holdings_.size()) {
        throw std::runtime_error("Portfolio value dimensions do not match.");
    }

    // Portfolio value equals cash plus marked-to-market position values.
    double total = cash_;
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        total += holdings_[i] * prices[i];
    }
    return total;
}

std::vector<double> Portfolio::weights(const std::vector<double>& prices) const {
    const double total = value(prices);
    std::vector<double> output(holdings_.size(), 0.0);

    if (total <= 0.0) {
        return output;
    }

    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        output[i] = holdings_[i] * prices[i] / total;
    }

    return output;
}

std::vector<Trade> Portfolio::rebalance(
    const std::string& date,
    const std::string& strategy_name,
    const std::vector<std::string>& assets,
    const std::vector<double>& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) {
    if (prices.size() != holdings_.size() || target_weights.size() != holdings_.size()) {
        throw std::runtime_error("Rebalance dimensions do not match.");
    }

    const double value_before = value(prices);
    if (value_before <= 0.0) {
        throw std::runtime_error("Portfolio value is non-positive.");
    }

    // Current dollar exposure for each asset before trading.
    std::vector<double> current_dollars(holdings_.size(), 0.0);
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        current_dollars[i] = holdings_[i] * prices[i];
    }

    // Estimate transaction costs before trading.
    // This protects the class invariant that cash should not become negative.
    double investable_value = value_before;
    for (int estimate = 0; estimate < 5; ++estimate) {
        double estimated_notional = 0.0;

        for (std::size_t i = 0; i < holdings_.size(); ++i) {
            const double target_dollars = investable_value * target_weights[i];
            estimated_notional += std::fabs(target_dollars - current_dollars[i]);
        }

        investable_value = std::max(0.0, value_before - estimated_notional * transaction_cost_rate);
    }

    std::vector<Trade> trades;
    trades.reserve(holdings_.size());

    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Desired exposure after reserving cash for transaction costs.
        const double target_dollars = investable_value * target_weights[i];

        // Positive means buy. Negative means sell.
        const double delta_dollars = target_dollars - current_dollars[i];

        if (std::fabs(delta_dollars) < 1e-8) {
            continue;
        }

        const double trade_shares = delta_dollars / prices[i];
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

    // Enforce class invariant after trading.
    if (cash_ < -1e-6) {
        throw std::runtime_error("Rebalance produced negative cash.");
    }
    if (cash_ < 0.0) {
        cash_ = 0.0;
    }

    return trades;
}

