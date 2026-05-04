// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// RiskMetrics consumes BacktestResult from the official backtesting module.
#include "backtesting/Types.hpp"

// Imports std::vector for return series and rolling volatility output.
#include <vector>

// RiskReport stores the performance and risk statistics computed from one backtest.
struct RiskReport {
    double total_return = 0.0;
    double annualized_return = 0.0;
    double annualized_volatility = 0.0;
    double sharpe_ratio = 0.0;
    double max_drawdown = 0.0;
    double var_95 = 0.0;
    double cvar_95 = 0.0;
    std::vector<double> rolling_volatility;
};

// RiskMetrics converts a BacktestResult into common portfolio risk statistics.
class RiskMetrics {
public:
    // Compute all risk metrics using the backtest equity curve and daily returns.
    RiskReport compute(const BacktestResult& result, double risk_free_rate = 0.0) const;

private:
    static double mean(const std::vector<double>& values);
    static double stddev(const std::vector<double>& values, double average);
    static double compute_max_drawdown(const std::vector<double>& equity_curve);
    static double compute_var(const std::vector<double>& returns, double alpha);
    static double compute_cvar(const std::vector<double>& returns, double var_threshold);
    static std::vector<double> compute_rolling_volatility(
        const std::vector<double>& returns,
        int window = 21
    );
};
