# Manuel Bossi - Backtesting and Portfolio Engine

Manuel owns the core simulation layer of the project.

## Main Responsibility

This module provides the reusable C++ engine that steps through historical prices, tracks portfolio accounting, and runs portfolio strategies through the Strategy Pattern.

## Files Owned by Manuel

| File | Purpose |
| --- | --- |
| `include/backtesting/Types.hpp` | Shared structs for prices, trades, and backtest results. |
| `include/backtesting/CsvReader.hpp` | Declares the CSV reader that converts generated price files into `PriceTable`. |
| `include/backtesting/PriceTable.hpp` | Stores historical asset prices by date. |
| `include/backtesting/Portfolio.hpp` | Tracks cash, holdings, portfolio value, weights, trades, and transaction costs. |
| `include/backtesting/RebalanceStrategy.hpp` | Defines the abstract strategy interface plus buy-and-hold and fixed monthly rebalance strategies. |
| `include/backtesting/BacktestEngine.hpp` | Defines the simulation engine interface and backtest configuration. |
| `src/backtesting/CsvReader.cpp` | Implements CSV parsing, column validation, and conversion into price rows. |
| `src/backtesting/PriceTable.cpp` | Implements basic price-table validation and lookup. |
| `src/backtesting/Portfolio.cpp` | Implements portfolio accounting and rebalancing logic. |
| `src/backtesting/RebalanceStrategy.cpp` | Implements buy-and-hold and monthly fixed-weight strategy behavior. |
| `src/backtesting/BacktestEngine.cpp` | Implements the day-by-day backtest loop. |
| `src/main_demo.cpp` | Standalone demo that reads `data/asset_prices.csv` and runs both baseline strategies. |

## How Other Team Members Plug In

Roberto's optimization strategy can implement or reuse the `RebalanceStrategy` interface to generate optimized target weights.

Sahil's risk and reporting modules can consume the `BacktestResult` object, especially the equity curve, daily returns, trade log, final weights, and turnover.
