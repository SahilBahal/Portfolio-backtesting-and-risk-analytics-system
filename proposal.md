# Final Project Topic Proposal

**Group Members:** Manuel Bossi, Roberto Rodriguez

**Topic:** Portfolio Rebalancing Backtester and Risk Analyzer

**Summary:**
We build a C++ program that reads historical price data for a small group of assets and simulates how a portfolio would perform over time. The program will compare a buy-and-hold portfolio against a simple rebalancing strategy, such as monthly rebalancing back to fixed target weights. It will report final value, total return, volatility, Sharpe ratio, maximum drawdown, and a log of the rebalancing trades.

**Scope:**
The project will be written entirely in C++ and organized into multiple source files with a CMake build. The main technical components will include:

- **CSV data import:** Read date and adjusted close price data for 2-4 assets from CSV files.
- **Price and return storage:** Store price histories and calculate daily asset returns.
- **Portfolio model:** Track holdings, cash, target weights, daily portfolio value, and realized rebalancing trades.
- **Backtest engine:** Step through the historical dates, update portfolio value, trigger scheduled rebalancing, and record the equity curve.
- **Strategy logic:** Implement at least two strategies: buy-and-hold and fixed-weight monthly rebalancing.
- **Risk metrics:** Compute total return, annualized return, volatility, Sharpe ratio, maximum drawdown, and turnover.
- **Output/reporting:** Print formatted summary tables and write CSV output files for the portfolio equity curve and rebalance log.

The planned class structure will include components such as `CsvReader`, `PriceTable`, `Portfolio`, `RebalanceStrategy`, `BacktestEngine`, `RiskMetrics`, and `ReportPrinter`.

If time allows, we may add transaction costs or a small Python wrapper for plotting results, but the core project will compile and run as a standalone C++17 program.
