// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// Imports std::string to store dates, strategy names, and asset tickers.
// Imports std::vector, a dynamic array used to store prices, returns, trades, and weights.
#include <string>
#include <vector>

// One row of historical price data using dynamic arrays.
// Example:
//   date   = "2024-01-02"
//   prices = [TLT price, IEF price, SPY price, ...]
// A struct groups related data together and lets us pass one object instead of separate variables.
struct PriceRow {
    std::string date;
    std::vector<double> prices;
};

// We record one trade created during a rebalancing: which date,
// which strategy, which asset, how many shares, how much notional $, $ transaction cost,
// and what turnover percentage.
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

// We store complete results from running one strategy through the backtest engine.
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
