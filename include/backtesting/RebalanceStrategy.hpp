#pragma once

#include "backtesting/Portfolio.hpp"
#include "backtesting/PriceTable.hpp"
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for row indexes.
#include <cstddef>
// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Abstract base class for all portfolio strategies.
// This is the Strategy Pattern from the course.
class RebalanceStrategy {
public:
    // A virtual destructor is required when a class has virtual functions.
    // It prevents undefined behavior if derived strategies are deleted through
    // a base-class pointer.
    virtual ~RebalanceStrategy() = default;

    // Strategy name used in reports.
    virtual std::string name() const = 0;

    // Pure virtual function.
    // Each derived strategy decides whether to trade on the current row.
    virtual std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const = 0;
};

// Base strategy 1:
// Buy-and-hold invests on day 0 through the BacktestEngine and then never
// rebalances.
class BuyAndHoldStrategy : public RebalanceStrategy {
public:
    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;
};

// Base strategy 2:
// Fixed monthly rebalancing restores target weights at the first trading day
// of each new calendar month.
class FixedWeightMonthlyRebalanceStrategy : public RebalanceStrategy {
public:
    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;

private:
    static bool is_new_month(
        const std::string& previous_date,
        const std::string& current_date
    );
};
