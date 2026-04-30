// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

#include "backtesting/PriceTable.hpp"
#include "backtesting/RebalanceStrategy.hpp"
#include "backtesting/Types.hpp"

// Imports std::vector, a dynamic array used to store target weights.
#include <vector>

// Configuration values for one backtest run.
struct BacktestConfig {
    double initial_capital = 100000.0;
    double transaction_cost_bps = 5.0;
};

// BacktestEngine runs the same day-by-day simulation loop for any strategy that implements
// the RebalanceStrategy interface.
class BacktestEngine {
public:
    BacktestEngine(
        const PriceTable& prices,
        std::vector<double> target_weights,
        BacktestConfig config
    );

    // Run one strategy through polymorphism:
    // BacktestEngine does not know the concrete strategy type. It only
    // calls the virtual rebalance() function.
    BacktestResult run(const RebalanceStrategy& strategy) const;

private:
    // The engine holds a reference to the price table,
    // a copy of the target weights, and a copy of the config.
    const PriceTable& prices_;
    std::vector<double> target_weights_;
    BacktestConfig config_;

    // This function fills the daily_returns vector in the BacktestResult by
    // calculating the percentage change in the equity curve from one day to the next.
    static void fill_daily_returns(BacktestResult& result);
};
