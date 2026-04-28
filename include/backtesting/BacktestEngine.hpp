#pragma once

#include "backtesting/PriceTable.hpp"
#include "backtesting/RebalanceStrategy.hpp"
#include "backtesting/Types.hpp"

#include <vector>

// Configuration values for one backtest run.
struct BacktestConfig {
    double initial_capital = 100000.0;
    double transaction_cost_bps = 5.0;
};

// BacktestEngine is Manuel's main system component.
// It runs the same day-by-day simulation loop for any strategy that implements
// the RebalanceStrategy interface.
class BacktestEngine {
public:
    BacktestEngine(
        const PriceTable& prices,
        std::vector<double> target_weights,
        BacktestConfig config
    );

    // Run one strategy through polymorphism.
    // Passing by reference avoids object slicing.
    BacktestResult run(const RebalanceStrategy& strategy) const;

private:
    const PriceTable& prices_;
    std::vector<double> target_weights_;
    BacktestConfig config_;

    static void fill_daily_returns(BacktestResult& result);
};

