// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// MonteCarloSimulator consumes BacktestResult from the official backtesting module.
#include "backtesting/Types.hpp"

// Imports std::vector for simulated terminal values.
#include <vector>

// MonteCarloResult stores the distribution of simulated future portfolio values.
struct MonteCarloResult {
    double mean_terminal = 0.0;
    double p5 = 0.0;
    double p25 = 0.0;
    double median = 0.0;
    double p75 = 0.0;
    double p95 = 0.0;
    std::vector<double> terminal_values;
};

// MonteCarloSimulator uses the historical daily return mean and volatility from
// a BacktestResult to simulate future terminal portfolio values.
class MonteCarloSimulator {
public:
    MonteCarloResult simulate(
        const BacktestResult& result,
        int num_simulations = 10000,
        int horizon_days = 252,
        unsigned int seed = 0
    ) const;

private:
    static double percentile(const std::vector<double>& sorted_values, double p);
};
