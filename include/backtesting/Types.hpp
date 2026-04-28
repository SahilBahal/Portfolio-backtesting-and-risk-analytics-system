#pragma once

#include <string>
#include <vector>

// One row of historical price data.
// Example:
//   date   = "2024-01-02"
//   prices = [TLT price, IEF price, SPY price, ...]
struct PriceRow {
    std::string date;
    std::vector<double> prices;
};

// One trade created by a rebalancing strategy.
// Positive shares/notional mean buy.
// Negative shares/notional mean sell.
struct Trade {
    std::string date;
    std::string strategy;
    std::string asset;
    double shares = 0.0;
    double notional = 0.0;
    double cost = 0.0;
    double turnover = 0.0;
};

// Complete result from running one strategy through the backtest engine.
// Sahil's risk module can consume this object to compute Sharpe, drawdown,
// VaR, CVaR, Monte Carlo inputs, and reporting tables.
struct BacktestResult {
    std::string name;
    std::vector<std::string> dates;
    std::vector<double> equity_curve;
    std::vector<double> daily_returns;
    std::vector<Trade> trades;
    std::vector<double> final_weights;
    double total_turnover = 0.0;
};

