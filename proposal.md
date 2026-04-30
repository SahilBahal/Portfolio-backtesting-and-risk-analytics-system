# FE522 Project Proposal

## Group members
- Robert Rodriguez [rrodrigu1@stevens.edu]
- Manuel Bossi [mbossi@stevens.edu]
- Sahil Bahal [sbahal1@stevens.edu]

## Topic

Portfolio Optimization, Backtesting, Risk, and Simulation System

## Summary

We build a modular C++ portfolio management system that reads historical multi-asset market data, simulates investment strategies through time, and compares performance across buy-and-hold, fixed monthly rebalancing, macro-aware allocation, and optimized allocation methods.

The system includes portfolio accounting, strategy execution, optimization, risk metrics, deterministic stress testing, and Monte Carlo simulation. Python is used only as a support layer to generate input data and plot output files produced by the C++ program.

## Scope
- CSV data/input layer for historical prices and target weights
- C++ `PriceTable` and `Portfolio` classes for prices, cash, holdings, trades, transaction costs, equity curve, and final weights
- Strategy Pattern using `RebalanceStrategy`, with buy-and-hold, fixed monthly rebalancing, macro-aware allocation, and optimized monthly rebalancing
- `BacktestEngine` for day-by-day portfolio simulation over historical dates
- Optimization layer with inverse-volatility weights, long-only minimum-variance allocation, efficient-frontier candidates, and allocation constraints
- Risk layer with total return, annualized return, volatility, Sharpe ratio, maximum drawdown, VaR, CVaR, and rolling volatility
- Monte Carlo simulation and stress testing under equity shock, rate shock, Bitcoin crash, and inflation shock scenarios
- C++ CSV outputs and Python plotting wrapper for equity curves, Monte Carlo bands, macro regimes, efficient frontier, and stress-test plots

## Presentation
- Attending via Zoom on May 7th: [yes] - 3 presenters (Robert, Manuel, Sahil)

