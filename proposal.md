# FE522 Final Project Proposal

## Group Members
- Roberto Rodriguez
- Manuel Bossi
- Sahil Bahal

## Topic
Portfolio Optimization, Backtesting, Risk, and Simulation System

## Summary
This project builds a modular C++ portfolio management system for testing and comparing investment strategies over historical market data. The system simulates portfolio performance across buy-and-hold, fixed monthly rebalancing, macro-aware allocation, and optimized minimum-variance allocation. It evaluates each strategy using performance metrics, risk analytics, deterministic stress tests, and Monte Carlo simulation.

Python is used only as a support layer for generating input data and plotting output results. The core portfolio accounting, strategy execution, optimization, risk metrics, stress testing, and Monte Carlo simulation are implemented in C++.

## Scope

### 1. Backtesting and Portfolio Engine
- `BacktestEngine` simulates portfolio evolution through time.
- `Portfolio` tracks cash, holdings, trades, transaction costs, equity curve, and final weights.
- Uses the Strategy Pattern through `RebalanceStrategy`.
- Base strategies include buy-and-hold and fixed monthly rebalancing.

### 2. Strategy Layer
- Buy-and-hold strategy.
- Fixed monthly rebalancing strategy.
- Macro-aware tactical allocation strategy.
- Optimized monthly rebalancing strategy using C++ optimization output.

### 3. Optimization and Allocation Layer
- `OptimizationEngine` computes portfolio weights using trailing historical data.
- Implements inverse-volatility allocation.
- Implements long-only minimum-variance allocation.
- Generates efficient-frontier candidate portfolios.
- Applies constraints such as long-only weights and maximum asset limits.

### 4. Risk and Simulation Layer
- `RiskMetrics` computes total return, annualized return, volatility, Sharpe ratio, maximum drawdown, VaR, CVaR, and rolling volatility.
- Monte Carlo simulation estimates future portfolio outcome distributions.
- Stress testing evaluates strategies under equity shock, rate shock, Bitcoin crash, and inflation shock scenarios.

### 5. Output and Reporting
- C++ writes CSV outputs for equity curves, trades, metrics, optimization logs, stress tests, and Monte Carlo results.
- Python plotting wrapper visualizes equity curves, Monte Carlo bands, macro regimes, efficient frontier, and stress tests.

## Team Split

| Member | Main Responsibility |
| --- | --- |
| Manuel Bossi | Backtest engine, portfolio accounting, base strategies, Strategy Pattern integration |
| Roberto Rodriguez | Optimization engine, allocation logic, efficient frontier, optimized strategy |
| Sahil Bahal | Risk metrics, Monte Carlo simulation, stress testing, reporting, visualization |

## Goal
Build a complete C++ portfolio system that combines backtesting, optimization, risk management, stress testing, and simulation to evaluate investment strategies in a realistic multi-asset setting.