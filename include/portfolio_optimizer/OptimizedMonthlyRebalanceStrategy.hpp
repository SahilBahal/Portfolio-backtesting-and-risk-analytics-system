#pragma once

#include "backtesting/RebalanceStrategy.hpp"
#include "OptimizationEngine.hpp"

#include <cstddef>
#include <string>
#include <vector>

class OptimizedMonthlyRebalanceStrategy : public RebalanceStrategy {
public:
    explicit OptimizedMonthlyRebalanceStrategy(std::size_t lookback_days = 60);

    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;

private:
    std::size_t lookback_days_;
};