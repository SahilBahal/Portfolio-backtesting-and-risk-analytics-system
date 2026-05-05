// BacktestEngine runs the day-by-day simulation loop.
#include "backtesting/BacktestEngine.hpp"
// CsvReader loads the real price CSV created by main.py.
#include "backtesting/CsvReader.hpp"
// RebalanceStrategy.hpp gives us the concrete strategies used in this demo:
// BuyAndHoldStrategy and FixedWeightMonthlyRebalanceStrategy.
#include "backtesting/RebalanceStrategy.hpp"
// Types.hpp gives us BacktestResult, Trade, and PriceRow structs.
#include "backtesting/Types.hpp"
// OptimizedMonthlyRebalanceStrategy runs the inverse-volatility optimizer.
#include "portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.hpp"
// MonteCarloSimulator creates future portfolio outcome simulations from daily returns.
#include "risk/MonteCarloSimulator.hpp"
// ReportPrinter writes equity, trade, metric, Monte Carlo, and stress-test CSV outputs.
#include "risk/ReportPrinter.hpp"
// RiskMetrics computes total return, volatility, Sharpe, drawdown, VaR, and CVaR.
#include "risk/RiskMetrics.hpp"
// StressTestEngine applies deterministic scenario shocks to final portfolio weights.
#include "risk/StressTestEngine.hpp"

// Gives std::size_t, the unsigned integer type used for vector indexes.
#include <cstddef>
// Gives std::fixed and std::setprecision for clean number formatting.
#include <iomanip>
// Gives std::cout for printing results to the terminal.
#include <iostream>
// Gives std::string for asset names.
#include <string>
// Gives std::vector for storing weights and asset names.
#include <vector>

// Anonymous namespace:
// Anything inside this namespace is only visible inside this file.
// This is useful for helper functions that should not be used by other modules.
namespace {

// Print the important output from one completed backtest.
//
// Inputs:
// result = output object created by BacktestEngine::run()
// assets = asset names from the CSV header, used when printing final weights
void print_result(const BacktestResult& result, const std::vector<std::string>& assets) {
    // Print the strategy name.
    // Example: "Buy and Hold" or "Fixed Monthly Rebalance".
    std::cout << result.name << "\n";

    // Print the final portfolio value.
    // equity_curve stores portfolio value for every date in the backtest.
    // .back() means "give me the last value in the vector".
    // std::fixed + std::setprecision(2) prints money with two decimals.
    std::cout << "  final value: " << std::fixed << std::setprecision(2)
              << result.equity_curve.back() << "\n";

    // Print how many individual trades the strategy created.
    // Buy-and-hold should create zero trades after the initial investment.
    // Monthly rebalancing should create trades when the month changes.
    std::cout << "  trades: " << result.trades.size() << "\n";

    // Print total turnover as a percent.
    // In BacktestResult, turnover is stored as a decimal:
    // 0.25 means 25%, so we multiply by 100 for display.
    std::cout << "  total turnover: " << std::fixed << std::setprecision(2)
              << result.total_turnover * 100.0 << "%\n";

    // Print the final allocation weights by asset.
    // This helps us see whether the strategy ended close to target weights.
    std::cout << "  final weights: ";

    // Loop over every final weight.
    // i is the vector index: 0 for SPY, 1 for TLT, 2 for GLD, 3 for BTC-USD.
    for (std::size_t i = 0; i < result.final_weights.size(); ++i) {
        // Add a comma before every item except the first one.
        // This makes the output easier to read:
        // SPY=50.00%, TLT=25.00%, ...
        if (i > 0) {
            std::cout << ", ";
        }

        // Print asset name and its ending portfolio weight.
        // result.final_weights[i] is decimal form, so multiply by 100.
        std::cout << assets[i] << "=" << std::fixed << std::setprecision(2)
                  << result.final_weights[i] * 100.0 << "%";
    }

    // Print two new lines after each strategy result.
    std::cout << "\n\n";
}

// Run the risk/reporting module on one completed BacktestResult.
//
// This function proves the integration point:
// BacktestEngine produces BacktestResult, then the risk module consumes it.
void write_risk_outputs(const BacktestResult& result, const std::string& output_dir) {
    // RiskMetrics reads the equity curve and daily returns from BacktestResult.
    RiskMetrics risk_metrics;
    const RiskReport risk_report = risk_metrics.compute(result);

    // MonteCarloSimulator fits a simple normal distribution to historical daily
    // returns and simulates future terminal portfolio values.
    // The fixed seed makes the demo reproducible for class/testing.
    MonteCarloSimulator monte_carlo;
    const MonteCarloResult monte_carlo_result = monte_carlo.simulate(
        result,
        10000,
        252,
        42
    );

    // StressTestEngine uses result.asset_names to map shocks to the correct assets.
    StressTestEngine stress_test;
    stress_test.load_defaults(result.asset_names);
    const std::vector<StressResult> stress_results = stress_test.run(result);

    // ReportPrinter writes all outputs to a strategy-specific folder.
    ReportPrinter printer(output_dir);
    printer.print_all(result, risk_report, monte_carlo_result, stress_results);
}

} // namespace

int main() {
    // Main demo program for the backtesting module.
    //
    // This file proves the C++ engine can:
    // 1. read real historical asset prices from CSV,
    // 2. construct a PriceTable,
    // 3. run multiple strategies through the same BacktestEngine, and
    // 4. print comparable results.

    // Read real historical prices generated by main.py.
    // Run this executable from the repository root so this relative path works:
    //   ./build/backtesting_demo
    //
    // CsvReader::read_price_table() returns a PriceTable object.
    // PriceTable validates the data shape and price values before the backtest starts.
    const PriceTable prices = CsvReader::read_price_table("data/asset_prices.csv");

    // Print a quick confirmation that the CSV was loaded correctly.
    // row_count() means number of dates.
    // asset_count() means number of assets.
    std::cout << "Loaded " << prices.row_count() << " price rows for "
              << prices.asset_count() << " assets.\n";

    // Print the first and last dates in the CSV.
    // date(0) is the first row.
    // date(row_count - 1) is the last row because C++ indexes start at 0.
    std::cout << "Date range: " << prices.date(0) << " to "
              << prices.date(prices.row_count() - 1) << "\n\n";

    // Target weights must be in the same order as the CSV header:
    // SPY, TLT, GLD, BTC-USD.
    //
    // This is a simple diversified macro allocation:
    // 50% US equities, 25% Treasury bonds, 15% gold, 10% Bitcoin.
    //
    // Important:
    // target_weights[0] belongs to SPY,
    // target_weights[1] belongs to TLT,
    // target_weights[2] belongs to GLD,
    // target_weights[3] belongs to BTC-USD.
    std::vector<double> target_weights = {0.50, 0.25, 0.15, 0.10};

    // BacktestConfig stores settings used by the engine.
    // Keeping settings in a struct makes the function calls cleaner.
    BacktestConfig config;

    // Starting portfolio value: $100,000.
    config.initial_capital = 100000.0;

    // Transaction cost in basis points.
    // 5 basis points = 0.05% = 0.0005 as a decimal.
    // BacktestEngine converts this value from bps to decimal before trading.
    config.transaction_cost_bps = 5.0;

    // Create the engine.
    //
    // The engine receives:
    // prices         = validated historical price table
    // target_weights = portfolio weights used by the strategies
    // config         = initial capital and transaction cost settings
    BacktestEngine engine(prices, target_weights, config);

    // Create strategy objects.
    //
    // These are concrete derived classes from the RebalanceStrategy base class.
    // This demonstrates inheritance and polymorphism from the course material.
    BuyAndHoldStrategy buy_and_hold;
    FixedWeightMonthlyRebalanceStrategy monthly_rebalance;
    OptimizedMonthlyRebalanceStrategy optimized_rebalance;

    // Run strategy 1: buy-and-hold.
    //
    // engine.run() accepts a RebalanceStrategy reference.
    // Because buy_and_hold is derived from RebalanceStrategy, C++ calls the
    // correct rebalance() method using dynamic dispatch.
    const BacktestResult buy_and_hold_result = engine.run(buy_and_hold);
    print_result(buy_and_hold_result, prices.assets());
    write_risk_outputs(buy_and_hold_result, "output/buy_and_hold");

    // Run strategy 2: fixed monthly rebalance.
    //
    // Same engine, same price data, same target weights, different strategy.
    // This is the main benefit of the Strategy Pattern.
    const BacktestResult monthly_rebalance_result = engine.run(monthly_rebalance);
    print_result(monthly_rebalance_result, prices.assets());
    write_risk_outputs(monthly_rebalance_result, "output/fixed_monthly_rebalance");

    // Run strategy 3: optimized monthly rebalance.
    //
    // This strategy still plugs into the same RebalanceStrategy interface.
    // The difference is that it computes dynamic inverse-volatility weights
    // instead of using the fixed target weights defined above.
    const BacktestResult optimized_rebalance_result = engine.run(optimized_rebalance);
    print_result(optimized_rebalance_result, prices.assets());
    write_risk_outputs(optimized_rebalance_result, "output/optimized_monthly_rebalance");

    // Returning 0 tells the operating system the program finished successfully.
    return 0;
}
