# FE522 Final Project Report

## Architecture Overview

Our project is a modular C++17 portfolio optimization, backtesting, risk, and simulation system. Python is used only to download and clean historical market data into `data/asset_prices.csv` and to visualize C++ output CSV files; the computational finance logic runs in C++.

Data flows through the system as follows: `CsvReader` parses the CSV file into a validated `PriceTable`, which stores dates, asset names, and adjusted prices. `BacktestEngine` owns the daily simulation loop. It creates a `Portfolio`, invests the initial capital, then calls a strategy through the abstract `RebalanceStrategy` interface on each date. `Portfolio` owns cash, holdings, transaction-cost accounting, trade logs, current value, and final weights. Each completed run returns a `BacktestResult`, which is consumed by `RiskMetrics`, `MonteCarloSimulator`, `StressTestEngine`, and `ReportPrinter`.

The strategy layer uses the Strategy Pattern. `BuyAndHoldStrategy`, `FixedWeightMonthlyRebalanceStrategy`, and `OptimizedMonthlyRebalanceStrategy` all inherit from `RebalanceStrategy` and implement the same virtual `rebalance()` function. This lets the same `BacktestEngine` run static and optimized strategies without changing the engine code. The optimizer module computes rolling inverse-volatility weights, the risk module writes CSV outputs plus a compact terminal risk snapshot, and `plot_results.py` turns those CSV outputs into charts.

## Mathematical Methods

The main algorithms implemented are fixed-calendar monthly rebalancing, rolling inverse-volatility allocation, historical VaR/CVaR, normal-return Monte Carlo simulation, and deterministic scenario stress testing.

Daily asset returns are simple returns:

$$r_{i,t} = \frac{P_{i,t}}{P_{i,t-1}} - 1$$

Portfolio value is marked to market as cash plus shares times prices:

$$V_t = C_t + \sum_i q_{i,t}P_{i,t}$$

For rebalancing, the implementation reserves 1% of portfolio value as cash. With cash reserve rate $r_{\text{cash}} = 0.01$, the trade size is:

$$\Delta q_i = \frac{w_i(1-r_{\text{cash}})V_t - q_iP_{i,t}}{P_{i,t}}$$

It then charges transaction cost as:

$$\text{cost}_i = |\Delta q_i P_{i,t}| \times c$$

where `c` is the transaction-cost rate converted from basis points.

The optimized monthly strategy uses inverse-volatility allocation:

$$w_i = \frac{1/\sigma_i}{\sum_j 1/\sigma_j}$$

where $\sigma_i$ is the sample standard deviation of recent daily returns over a 60-day lookback window. This is a simple long-only risk-balancing method related to mean-variance portfolio ideas from Markowitz (1952, https://doi.org/10.1111/j.1540-6261.1952.tb01525.x).

Risk metrics include total return, annualized return, annualized volatility, Sharpe ratio, maximum drawdown, historical VaR, historical CVaR, and rolling volatility. Annualized volatility is:

$$\sigma_{\text{ann}} = \sigma_{\text{daily}}\sqrt{252}$$

Sharpe ratio is:

$$\text{Sharpe} = \frac{R_{\text{ann}} - R_f}{\sigma_{\text{ann}}}$$

following Sharpe's risk-adjusted performance concept (Sharpe, 1966, https://doi.org/10.1086/294846). Historical VaR sorts realized daily returns and takes the 5th percentile loss; CVaR averages the losses beyond that threshold, consistent with common market-risk practice such as RiskMetrics (1996, https://www.msci.com/research-and-insights/paper/1996-riskmetrics-technical-document).

Monte Carlo simulation estimates one-year future terminal values by fitting a normal distribution to historical daily portfolio returns, then compounding random daily returns for 252 days over 10,000 simulations. This uses the standard Monte Carlo idea of repeated random sampling (Metropolis and Ulam, 1949, https://doi.org/10.1080/01621459.1949.10483310). Stress testing applies deterministic asset shocks to final portfolio weights, for example equity shock, rate shock, Bitcoin crash, and inflation shock.

## Key Design Decisions and Trade-Offs

We used separate modules and headers instead of one large file because the project has distinct responsibilities: data loading, portfolio accounting, strategy execution, optimization, risk analysis, simulation, and reporting. This matches the course requirement for multiple interacting components and makes each class easier to test and explain.

We used inheritance and polymorphism for strategies because the backtest engine should not need `if` statements for every strategy type. It only calls `strategy.rebalance(...)`; C++ chooses the correct derived implementation at runtime. This is cleaner than hard-coding buy-and-hold, monthly rebalance, and optimized rebalance directly inside the engine.

We used `std::vector` and `std::string` rather than raw arrays or manual memory management. The number of assets and dates is data-dependent, so dynamic containers are appropriate. This also follows the Rule of Zero: most classes do not need custom destructors, copy constructors, or assignment operators.

CSV parsing is kept simple but defensive. The reader validates that the file has a header, at least one asset column, numeric positive prices, and consistent row dimensions before constructing the `PriceTable`. This avoids running a backtest on malformed input.

The optimizer uses inverse-volatility weights instead of a full quadratic-programming minimum-variance solver. This was a deliberate trade-off: inverse-volatility is transparent, long-only, easy to present, and robust for a small student project. The code still includes covariance and portfolio-variance functions, which provide a foundation for a more advanced optimizer later.

We did not use a root-finder such as Newton's method or bisection because this project does not solve nonlinear pricing equations. The optimization method has a closed-form normalization step, so a root-finder would add complexity without improving this implementation.

## Improvements Given More Time

The first improvement would be a true constrained minimum-variance optimizer with explicit maximum weights, cash constraints, and efficient-frontier output. Second, the Monte Carlo model could be improved by bootstrapping historical returns or using fat-tailed distributions instead of normal daily returns. Third, the CSV reader could support missing-date alignment across assets instead of assuming one clean wide CSV. Finally, transaction-cost modeling could be expanded to include bid-ask spread, slippage, and asset-specific costs.
