# FE522 Project

Portfolio Rebalancing Backtester and Risk Analyzer.

Group members: Manuel Bossi, Roberto Rodriguez, Sahil Bahal

This repository is for the FE522 C++ final project. The planned program is a C++17 backtester that reads historical adjusted close prices for a small group of assets, compares buy-and-hold against scheduled portfolio rebalancing, and reports return and risk metrics.

## Planned Scope

- Read asset price data from CSV files.
- Store price histories and compute daily returns.
- Track portfolio holdings, cash, target weights, and daily value.
- Compare buy-and-hold with fixed-weight monthly rebalancing.
- Compute total return, annualized return, volatility, Sharpe ratio, maximum drawdown, and turnover.
- Write portfolio equity curves and rebalance logs to CSV.

The project will use multiple C++ source files and a CMake build.
