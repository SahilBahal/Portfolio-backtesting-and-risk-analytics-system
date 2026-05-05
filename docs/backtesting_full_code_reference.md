# Backtesting Module: Full Code Reference

This single Markdown file explains the architecture of the backtesting, optimizer, and risk modules and includes the full code for each relevant project file.

It is meant as a study/reference document. The actual runnable source code remains in the normal `.hpp`, `.cpp`, `CMakeLists.txt`, and Python support files.

## Architecture Summary

```text
main.py -> data/asset_prices.csv
        -> CsvReader
        -> PriceTable
        -> BacktestEngine
        -> RebalanceStrategy
           -> BuyAndHoldStrategy
           -> FixedWeightMonthlyRebalanceStrategy
           -> OptimizedMonthlyRebalanceStrategy -> OptimizationEngine
        -> Portfolio
        -> BacktestResult
        -> RiskMetrics / MonteCarloSimulator / StressTestEngine
        -> ReportPrinter -> output/*.csv
```

## Main Design Choices

- `BacktestEngine` owns the day-by-day simulation loop.
- `RebalanceStrategy` is the Strategy Pattern interface. The engine receives a `RebalanceStrategy&`, so it can run buy-and-hold, fixed rebalancing, or optimized rebalancing without changing the engine code.
- `OptimizedMonthlyRebalanceStrategy` proves Robert's optimizer can plug into the same interface as the baseline strategies.
- `OptimizationEngine` calculates inverse-volatility weights using a trailing lookback window.
- `Portfolio` owns accounting state: cash, holdings, trades, transaction costs, and final weights.
- `BacktestResult` is the shared contract consumed by the risk/reporting module.
- `Trade` includes `shares`, `price`, `notional`, `cost`, and `turnover` so reporting can write complete trade CSVs.
- `asset_names` maps final weights and stress-test shocks to the right assets.
- CMake builds separate `backtesting`, `portfolio_optimizer`, and `risk` libraries, then links them into `backtesting_demo`.

## File Map

| File | Purpose |
| --- | --- |
| `CMakeLists.txt` | Defines the C++17 build, builds the backtesting, optimizer, and risk libraries, and creates the backtesting_demo executable. |
| `src/main_demo.cpp` | Main demo program. It reads data/asset_prices.csv, defines target weights, runs buy-and-hold, fixed monthly rebalance, and optimized monthly rebalance, then writes report outputs. |
| `include/backtesting/Types.hpp` | Defines shared data objects used across modules: PriceRow, Trade, and BacktestResult. |
| `include/backtesting/CsvReader.hpp` | Declares the CSV reader that loads a wide price CSV into a PriceTable. |
| `src/backtesting/CsvReader.cpp` | Implements CSV parsing, validates columns, converts text prices into doubles, and returns a PriceTable. |
| `include/backtesting/PriceTable.hpp` | Declares the class that stores validated dates, asset names, and historical prices. |
| `src/backtesting/PriceTable.cpp` | Implements shape checks, positive-price checks, and safe lookup methods. |
| `include/backtesting/Portfolio.hpp` | Declares the portfolio accounting class for cash, holdings, weights, trades, and rebalancing. |
| `src/backtesting/Portfolio.cpp` | Implements initial investment, portfolio value, final weights, transaction costs, and rebalance accounting. |
| `include/backtesting/RebalanceStrategy.hpp` | Declares the abstract strategy interface plus the two baseline strategy classes. |
| `src/backtesting/RebalanceStrategy.cpp` | Implements buy-and-hold and fixed monthly rebalance behavior. |
| `include/backtesting/BacktestEngine.hpp` | Declares the simulation engine and backtest configuration. |
| `src/backtesting/BacktestEngine.cpp` | Implements the daily backtest loop, polymorphic strategy calls, trade collection, and daily return calculation. |
| `include/portfolio_optimizer/OptimizationEngine.hpp` | Declares portfolio optimization helper functions such as inverse-volatility weights and covariance calculations. |
| `src/portfolio_optimizer/OptimizationEngine.cpp` | Implements return, volatility, covariance, inverse-volatility weight, and variance calculations. |
| `include/portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.hpp` | Declares the optimized monthly rebalance strategy that plugs into RebalanceStrategy. |
| `src/portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.cpp` | Implements monthly inverse-volatility rebalancing through the same strategy interface as the baseline strategies. |
| `include/risk/RiskMetrics.hpp` | Declares risk metric outputs and the RiskMetrics calculator. |
| `src/risk/RiskMetrics.cpp` | Implements total return, annualized return, volatility, Sharpe ratio, max drawdown, VaR, CVaR, and rolling volatility. |
| `include/risk/MonteCarloSimulator.hpp` | Declares Monte Carlo simulation outputs and interface. |
| `src/risk/MonteCarloSimulator.cpp` | Implements normal-return Monte Carlo simulation of future terminal portfolio values. |
| `include/risk/StressTestEngine.hpp` | Declares deterministic stress-test scenarios and outputs. |
| `src/risk/StressTestEngine.cpp` | Implements default stress scenarios for SPY, TLT, GLD, and BTC-USD. |
| `include/risk/ReportPrinter.hpp` | Declares CSV report-writing functions. |
| `src/risk/ReportPrinter.cpp` | Writes equity curves, trades, metrics, rolling volatility, Monte Carlo, and stress-test CSV outputs. |
| `main.py` | Downloads real market data with yfinance and writes data/asset_prices.csv for the C++ demo. |
| `requirements-data.txt` | Lists the Python packages needed to regenerate the price CSV. |

## CSV Data File

The generated CSV is `data/asset_prices.csv`. It is not pasted in full here because it contains many rows of market data. Its format is:

```text
date,SPY,TLT,GLD,BTC-USD
2020-01-02,296.888245,114.166550,143.949997,6985.470215
...
```

## Full Code

### Build File: `CMakeLists.txt`

Defines the C++17 build, builds the backtesting, optimizer, and risk libraries, and creates the backtesting_demo executable.

```cmake
cmake_minimum_required(VERSION 3.16)

project(FE522_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json so VS Code / clangd can read the same include
# paths and compiler flags that CMake uses during the real build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(backtesting
    src/backtesting/BacktestEngine.cpp
    src/backtesting/CsvReader.cpp
    src/backtesting/Portfolio.cpp
    src/backtesting/PriceTable.cpp
    src/backtesting/RebalanceStrategy.cpp
)

target_include_directories(backtesting PUBLIC include)

if (MSVC)
    target_compile_options(backtesting PRIVATE /W4)
else()
    target_compile_options(backtesting PRIVATE -Wall -Wextra -pedantic)
endif()

add_library(risk
    src/risk/MonteCarloSimulator.cpp
    src/risk/ReportPrinter.cpp
    src/risk/RiskMetrics.cpp
    src/risk/StressTestEngine.cpp
)

target_include_directories(risk PUBLIC include)
target_link_libraries(risk PUBLIC backtesting)

if (MSVC)
    target_compile_options(risk PRIVATE /W4)
else()
    target_compile_options(risk PRIVATE -Wall -Wextra -pedantic)
endif()

add_library(portfolio_optimizer
    src/portfolio_optimizer/OptimizationEngine.cpp
    src/portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.cpp
)

target_include_directories(portfolio_optimizer PUBLIC include)
target_link_libraries(portfolio_optimizer PUBLIC backtesting)

if (MSVC)
    target_compile_options(portfolio_optimizer PRIVATE /W4)
else()
    target_compile_options(portfolio_optimizer PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(backtesting_demo
    src/main_demo.cpp
)

target_link_libraries(backtesting_demo PRIVATE risk portfolio_optimizer)
```

### C++ Demo Entry Point: `src/main_demo.cpp`

Main demo program. It reads data/asset_prices.csv, defines target weights, runs buy-and-hold, fixed monthly rebalance, and optimized monthly rebalance, then writes report outputs.

```cpp
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
```

### Shared Types Header: `include/backtesting/Types.hpp`

Defines shared data objects used across modules: PriceRow, Trade, and BacktestResult.

```cpp
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
// which strategy, which asset, how many shares, the execution price, how much
// notional $, $ transaction cost, and what turnover percentage.
// Positive shares/notional mean buy.
// Negative shares/notional mean sell.
struct Trade {
    std::string date;
    std::string strategy;
    std::string asset;
    double shares = 0.0;
    double price = 0.0;
    double notional = 0.0;
    double cost = 0.0;
    double turnover = 0.0;
};

// We store complete results from running one strategy through the backtest engine.
// The risk module can consume this object to compute Sharpe, drawdown,
// VaR, CVaR, Monte Carlo inputs, and reporting tables.
struct BacktestResult {
    std::string name;
    std::vector<std::string> asset_names;
    std::vector<std::string> dates;
    std::vector<double> equity_curve;
    std::vector<double> daily_returns;
    std::vector<Trade> trades;
    std::vector<double> final_weights;
    double total_turnover = 0.0;
};
```

### CSV Reader Header: `include/backtesting/CsvReader.hpp`

Declares the CSV reader that loads a wide price CSV into a PriceTable.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need PriceTable because this reader converts a CSV file into a PriceTable object.
#include "backtesting/PriceTable.hpp"

// Imports std::string to store the CSV file path.
#include <string>

// CsvReader is responsible for reading historical price data from a CSV file.
// It keeps file parsing separate from the backtest engine.
// This separation is important because BacktestEngine should only run simulations;
// it should not also know how to parse files.
class CsvReader {
public:
    // Read a wide price CSV and return a validated PriceTable.
    //
    // Expected CSV format:
    // date,SPY,TLT,GLD,BTC-USD
    // 2020-01-02,296.888245,114.166550,143.949997,6985.470215
    //
    // The first column must be the date.
    // Every other column is treated as one asset price series.
    static PriceTable read_price_table(const std::string& path);
};
```

### CSV Reader Source: `src/backtesting/CsvReader.cpp`

Implements CSV parsing, validates columns, converts text prices into doubles, and returns a PriceTable.

```cpp
#include "backtesting/CsvReader.hpp"

// Gives std::ifstream for reading files from disk.
#include <fstream>
// Gives std::ostringstream for building clear error messages.
#include <sstream>
// Gives std::runtime_error for throwing exceptions when the CSV is invalid.
#include <stdexcept>
// Gives std::move for efficiently moving vectors into PriceTable.
#include <utility>
// Imports std::vector because we build vectors of strings and PriceRow objects.
#include <vector>

namespace {

// Remove simple leading/trailing spaces from a CSV field.
// yfinance does not add spaces, but trimming makes the reader safer for hand-edited files.
std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Split one CSV line on commas.
// This simple parser is enough for our generated price file because the fields
// are plain dates, tickers, and numbers without quoted commas.
std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }

    return fields;
}

// Convert one price field from text to double and provide a helpful error if it fails.
double parse_price(const std::string& text, std::size_t line_number, const std::string& asset) {
    try {
        std::size_t parsed_characters = 0;
        const double price = std::stod(text, &parsed_characters);

        // parsed_characters must cover the whole field. Otherwise a value like
        // "123abc" would partially parse as 123, which is not a clean price.
        if (parsed_characters != text.size()) {
            throw std::invalid_argument("extra characters after price");
        }

        return price;
    } catch (const std::exception&) {
        std::ostringstream message;
        message << "Invalid price for " << asset << " on CSV line " << line_number << ".";
        throw std::runtime_error(message.str());
    }
}

} // namespace

// Read a CSV file and convert it into the same PriceTable object used by BacktestEngine.
PriceTable CsvReader::read_price_table(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open CSV file: " + path);
    }

    std::string header_line;
    if (!std::getline(file, header_line)) {
        throw std::runtime_error("CSV file is empty: " + path);
    }

    const std::vector<std::string> header_fields = split_csv_line(header_line);
    if (header_fields.size() < 2) {
        throw std::runtime_error("CSV file must have a date column and at least one asset column.");
    }
    if (header_fields[0] != "date") {
        throw std::runtime_error("First CSV column must be named date.");
    }

    // The header after the date column becomes the asset list.
    std::vector<std::string> assets;
    assets.reserve(header_fields.size() - 1);
    for (std::size_t i = 1; i < header_fields.size(); ++i) {
        if (header_fields[i].empty()) {
            throw std::runtime_error("CSV header contains an empty asset name.");
        }
        assets.push_back(header_fields[i]);
    }

    std::vector<PriceRow> rows;
    std::string line;
    std::size_t line_number = 1;

    while (std::getline(file, line)) {
        ++line_number;

        // Skip blank lines so one accidental empty line at the end does not break the demo.
        if (trim(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = split_csv_line(line);
        if (fields.size() != header_fields.size()) {
            std::ostringstream message;
            message << "CSV line " << line_number << " has " << fields.size()
                    << " columns, expected " << header_fields.size() << ".";
            throw std::runtime_error(message.str());
        }
        if (fields[0].empty()) {
            std::ostringstream message;
            message << "CSV line " << line_number << " has an empty date.";
            throw std::runtime_error(message.str());
        }

        PriceRow row;
        row.date = fields[0];
        row.prices.reserve(assets.size());

        for (std::size_t i = 1; i < fields.size(); ++i) {
            row.prices.push_back(parse_price(fields[i], line_number, assets[i - 1]));
        }

        rows.push_back(std::move(row));
    }

    // PriceTable performs the final validation: at least two rows, positive prices,
    // and the same number of prices in each row.
    return PriceTable(std::move(assets), std::move(rows));
}
```

### Price Table Header: `include/backtesting/PriceTable.hpp`

Declares the class that stores validated dates, asset names, and historical prices.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need PriceRow, which is declared in Types.hpp.
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for row and asset counts.
#include <cstddef>

// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Small shared dependency used by the backtest engine.
// It stores already-cleaned historical prices in a simple date-by-date table:
// rows are dates, columns are assets, and each cell is a price.
// The public methods below let other components read the data safely.
// The private member variables below prevent outside code from changing the table directly.
class PriceTable {
public:
    // This constructor validates the table's shape and price values.
    // It takes two inputs: a vector of asset tickers and a vector of PriceRow objects.
    // Here we only declare the constructor; we define it in PriceTable.cpp.
    PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows);

    // Asset tickers in the same order as every row's price vector.
    // We declare a function called assets() that returns a reference to the vector of asset tickers.
    // 1. std::vector<std::string>: a list of strings.
    // 2. &: return by reference, avoiding an expensive copy.
    // 3. const: the caller is not allowed to modify the returned vector.
    const std::vector<std::string>& assets() const;

    // Number of historical dates (rows)
    // We declare a function called row_count() that returns the number of rows in the price table.
    std::size_t row_count() const;

    // Number of assets (columns)
    std::size_t asset_count() const;

    // Date label for one row.
    // We declare a function called date() that takes a row index and returns the date string for that row.
    const std::string& date(std::size_t row) const;

    // Prices for all assets on one row.
    // We declare a function called prices() that takes a row index and returns a reference to the vector of prices for that row.
    const std::vector<double>& prices(std::size_t row) const;

private:
    // Outside code cannot modify this data directly because it is private,
    // but the class's own functions can access it. This is encapsulation.

    // This variable stores the asset tickers in the same order as every row's price vector.
    std::vector<std::string> assets_;
    // This variable stores the historical price data. Each PriceRow contains one date
    // and one vector of prices for all assets on that date.
    std::vector<PriceRow> rows_;
};
```

### Price Table Source: `src/backtesting/PriceTable.cpp`

Implements shape checks, positive-price checks, and safe lookup methods.

```cpp
#include "backtesting/PriceTable.hpp"

// Gives std::isfinite to validate price values.
#include <cmath>
// Gives std::runtime_error for throwing exceptions when the input data is invalid.
#include <stdexcept>
// Gives std::move for efficiently moving input vectors into the class without copying.
#include <utility>

// Here we define the constructor and member functions of the PriceTable
// class declared in PriceTable.hpp.
PriceTable::PriceTable(std::vector<std::string> assets, std::vector<PriceRow> rows)
    : assets_(std::move(assets)), rows_(std::move(rows)) {
    // Here we move incoming assets and rows into the class's private member variables,
    // avoiding copies.

    // The table must contain at least one asset and enough
    // dates (2) to calculate a return.
    if (assets_.empty()) {
        throw std::runtime_error("PriceTable must contain at least one asset.");
    }
    if (rows_.size() < 2) {
        throw std::runtime_error("PriceTable must contain at least two rows.");
    }


    // Validate all prices once during construction so the backtest engine can
    // assume the table is clean.
    for (const auto& row : rows_) {
        // C++ infers the type, uses a reference to avoid copying, and does not modify the row.

        // Each row must have one price per asset.
        if (row.prices.size() != assets_.size()) {
            throw std::runtime_error("Price row has the wrong number of columns.");
        }
        // every price must be finite and greater than 0.
        for (double price : row.prices) {
            if (!std::isfinite(price) || price <= 0.0) {
                throw std::runtime_error("PriceTable contains an invalid price.");
            }
        }
    }
}

const std::vector<std::string>& PriceTable::assets() const {
    // Return a reference to the vector of asset tickers.
    return assets_;
}

std::size_t PriceTable::row_count() const {
    // Return the number of rows in the price table, meaning how many dates exist.
    return rows_.size();
}

std::size_t PriceTable::asset_count() const {
    // Return the number of assets in the price table.
    return assets_.size();
}

const std::string& PriceTable::date(std::size_t row) const {
    // Return the date at index row; .at() checks bounds and throws if row is out of range.
    return rows_.at(row).date;
}

const std::vector<double>& PriceTable::prices(std::size_t row) const {
    // Return the prices at index row; .at() checks bounds and throws if row is out of range.
    return rows_.at(row).prices;
}
```

### Portfolio Header: `include/backtesting/Portfolio.hpp`

Declares the portfolio accounting class for cash, holdings, weights, trades, and rebalancing.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// We need Trade, which is declared in Types.hpp.
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for asset counts.
#include <cstddef>

// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Portfolio owns the accounting state during a backtest.
// This class is encapsulated: cash and holdings are private, and
// outside code must use public methods to inspect value or request trades.
class Portfolio {
public:
    // We create a portfolio with starting cash and a number of assets.
    Portfolio(double initial_capital, std::size_t asset_count);

    // We invest all starting capital into the target weights on day 0.
    // This function does not return a value (void).
    void invest_to_target(
        const std::vector<double>& prices,
        const std::vector<double>& target_weights
    );

    // We mark the portfolio to market using the latest prices.
    // Const because it reads state but does not change holdings.
    double value(const std::vector<double>& prices) const;

    // Return current portfolio weights using latest prices.
    // Const because it reads state but does not change values.
    // This is useful for reporting final allocations and stress tests.
    std::vector<double> weights(const std::vector<double>& prices) const;

    // This function rebalances current holdings back to target weights.
    // It also returns a Trade log so the reporting module can show exactly
    // what was bought or sold.
    std::vector<Trade> rebalance(
        const std::string& date,
        const std::string& strategy_name,
        const std::vector<std::string>& assets,
        const std::vector<double>& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    );

private:
    // Private state enforces encapsulation.
    // cash_ stores cash, and holdings_ stores the number of shares held for each asset.
    double cash_;
    std::vector<double> holdings_;
};
```

### Portfolio Source: `src/backtesting/Portfolio.cpp`

Implements initial investment, portfolio value, final weights, transaction costs, and rebalance accounting.

```cpp
#include "backtesting/Portfolio.hpp"

// Gives std::fabs to calculate absolute value.
#include <cmath>

// Gives std::runtime_error for throwing exceptions when the input data is invalid.
#include <stdexcept>

// Here we define the constructor and member functions of the Portfolio
// class declared in Portfolio.hpp.
Portfolio::Portfolio(double initial_capital, std::size_t asset_count)
    : cash_(initial_capital), holdings_(asset_count, 0.0) {
    // Sets cash = initial capital and creates one holding for each asset, initialized to 0.0 shares.

    // A portfolio must start with positive capital.
    if (initial_capital <= 0.0) {
        throw std::runtime_error("Initial capital must be positive.");
    }
}

// This function invests all starting capital into the target weights on day 0.
// It does not return a value (void).
void Portfolio::invest_to_target(
    const std::vector<double>& prices,
    const std::vector<double>& target_weights
) {
    // The number of prices and target weights must match the number of assets in the portfolio.
    if (prices.size() != holdings_.size() || target_weights.size() != holdings_.size()) {
        throw std::runtime_error("Initial investment dimensions do not match.");
    }

    // We calculate the starting value of the portfolio, which is just the initial cash.
    const double starting_value = cash_;

    // We loop through each asset, convert the target weight into dollars, then convert dollars into shares,
    // and update cash by subtracting the dollars invested.
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Convert target weight into dollars.
        const double dollars = starting_value * target_weights[i];

        // Convert dollars into shares.
        holdings_[i] = dollars / prices[i];

        // Buying the position uses cash.
        cash_ -= dollars;
    }
}

// We mark the portfolio to market using the latest prices.
// Const because it reads state but does not change holdings.
double Portfolio::value(const std::vector<double>& prices) const {
    if (prices.size() != holdings_.size()) {
        throw std::runtime_error("Portfolio value dimensions do not match.");
    }

    // Portfolio value equals cash plus marked-to-market position values.
    double total = cash_;
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        total += holdings_[i] * prices[i];
    }
    // Portfolio value = cash + sum of (shares held in each asset * price of each asset).
    return total;
}

// Return current portfolio weights using latest prices.
// Const because it reads state but does not change values.
// This is useful for reporting final allocations and stress tests.
std::vector<double> Portfolio::weights(const std::vector<double>& prices) const {
    const double total = value(prices);

    // We initialize the output vector with zeros in case the portfolio value is zero or negative.
    std::vector<double> output(holdings_.size(), 0.0);

    // If the portfolio value is zero or negative, we cannot calculate meaningful weights.
    if (total <= 0.0) {
        return output;
    }

    // Otherwise, we calculate weights as (shares * price) / total value.
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        output[i] = holdings_[i] * prices[i] / total;
    }

    // We return the vector of weights.
    return output;
}


// This function rebalances current holdings back to target weights.
// It also returns a Trade log so the reporting module can show exactly
// what was bought or sold.
std::vector<Trade> Portfolio::rebalance(
    const std::string& date,
    const std::string& strategy_name,
    const std::vector<std::string>& assets,
    const std::vector<double>& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) {
    // The number of prices, target weights, and asset names must match the number of assets in the portfolio.
    if (
        prices.size() != holdings_.size() ||
        target_weights.size() != holdings_.size() ||
        assets.size() != holdings_.size()
    ) {
        throw std::runtime_error("Rebalance dimensions do not match.");
    }

    // We calculate the current portfolio value before trading to use for calculating trade sizes and turnover.
    const double value_before = value(prices);
    if (value_before <= 0.0) {
        throw std::runtime_error("Portfolio value is non-positive.");
    }

    // Create a vector of current dollar values for each asset (shares held * price).
    std::vector<double> current_dollars(holdings_.size(), 0.0);
    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        current_dollars[i] = holdings_[i] * prices[i];
    }

    // We do NOT invest the full portfolio value during a rebalance.
    // To keep the accounting simple we reserve 1% of the
    // portfolio value as cash and only rebalance the remaining 99%.
    // Example:
    //   value_before      = 100,000
    //   cash_reserve_rate = 0.01
    //   investable_value  = 100,000 * (1 - 0.01) = 99,000
    // The reserved cash helps pay transaction costs and prevents the portfolio
    // from becoming over-invested.
    const double cash_reserve_rate = 0.01;
    const double investable_value = value_before * (1.0 - cash_reserve_rate);

    // We loop through each asset, calculate the desired dollar exposure based on target weights,
    // calculate the dollar difference from current exposure, convert that into shares to buy or sell,
    // update cash and holdings, and record the trade details in a Trade struct.
    std::vector<Trade> trades;
    // We reserve space in the trades vector to avoid multiple reallocations
    // in case we trade every asset. This is an optimization to improve performance.
    trades.reserve(holdings_.size());

    for (std::size_t i = 0; i < holdings_.size(); ++i) {
        // Desired exposure after reserving cash for transaction costs.
        const double target_dollars = investable_value * target_weights[i];

        // Dollar difference between current and target exposure.
        const double delta_dollars = target_dollars - current_dollars[i];

        // If the dollar difference is very small, we skip trading to avoid unnecessary transaction costs.
        if (std::fabs(delta_dollars) < 1e-8) {
            continue;
        }

        // Convert dollar difference into shares to buy or sell.
        const double trade_shares = delta_dollars / prices[i];
        // Calculate transaction cost for this trade.
        const double cost = std::fabs(delta_dollars) * transaction_cost_rate;

        // Update accounting state.
        holdings_[i] += trade_shares;
        cash_ -= delta_dollars;
        cash_ -= cost;

        // Record this trade for reporting.
        Trade trade;
        trade.date = date;
        trade.strategy = strategy_name;
        trade.asset = assets[i];
        trade.shares = trade_shares;
        trade.price = prices[i];
        trade.notional = delta_dollars;
        trade.cost = cost;
        trade.turnover = std::fabs(delta_dollars) / value_before;
        trades.push_back(trade);
    }

    // After processing all trades, we check if cash is negative beyond a small tolerance.
    // If so, we throw an exception because the rebalance logic produced an invalid state.
    // If cash is slightly negative due to rounding or transaction costs, we clamp it to zero.
    if (cash_ < -1e-6) {
        throw std::runtime_error("Rebalance produced negative cash.");
    }
    if (cash_ < 0.0) {
        cash_ = 0.0;
    }

    // We return the vector of trades executed during this rebalance.
    return trades;
}
```

### Strategy Header: `include/backtesting/RebalanceStrategy.hpp`

Declares the abstract strategy interface plus the two baseline strategy classes.

```cpp
#pragma once

#include "backtesting/Portfolio.hpp"
#include "backtesting/PriceTable.hpp"
#include "backtesting/Types.hpp"

// Gives std::size_t, an unsigned integer type used for row indexes.
#include <cstddef>
// Imports std::string to store text and std::vector to store dynamic arrays.
#include <string>
#include <vector>

// Abstract base class for all portfolio strategies.
// This is the Strategy Pattern from the course.
class RebalanceStrategy {
public:
    // A virtual destructor is required when a class has virtual functions.
    // It prevents undefined behavior if derived strategies are deleted through
    // a base-class pointer.
    virtual ~RebalanceStrategy() = default;

    // Strategy name used in reports.
    virtual std::string name() const = 0;

    // Pure virtual function.
    // Each derived strategy decides whether to trade on the current row.
    virtual std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const = 0;
};

// Base strategy 1:
// Buy-and-hold invests on day 0 through the BacktestEngine and then never
// rebalances.
class BuyAndHoldStrategy : public RebalanceStrategy {
public:
    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;
};

// Base strategy 2:
// Fixed monthly rebalancing restores target weights at the first trading day
// of each new calendar month.
class FixedWeightMonthlyRebalanceStrategy : public RebalanceStrategy {
public:
    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;

private:
    static bool is_new_month(
        const std::string& previous_date,
        const std::string& current_date
    );
};
```

### Strategy Source: `src/backtesting/RebalanceStrategy.cpp`

Implements buy-and-hold and fixed monthly rebalance behavior.

```cpp
#include "backtesting/RebalanceStrategy.hpp"

// Here we define the member functions of the strategy classes declared in RebalanceStrategy.hpp.
std::string BuyAndHoldStrategy::name() const {
    return "Buy and Hold";
}

std::vector<Trade> BuyAndHoldStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    // Buy-and-hold never trades after the initial investment.
    // These casts avoid compiler warnings for intentionally unused parameters.
    (void)row;
    (void)portfolio;
    (void)prices;
    (void)target_weights;
    (void)transaction_cost_rate;

    return {};
}

// Fixed monthly rebalancing restores target weights at the first trading day
// of each new calendar month. The strategy checks the date on every row and
// only trades when the month changes. The strategy calls Portfolio::rebalance()
// to do the actual trading and accounting.
std::string FixedWeightMonthlyRebalanceStrategy::name() const {
    return "Fixed Monthly Rebalance";
}

// This function checks if the date has moved into a new calendar month and only
// rebalances on the first trading day of each new month.
std::vector<Trade> FixedWeightMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    // Row 0 has no previous date. The engine normally starts calling strategies
    // at row 1, but this guard keeps the class safe if reused elsewhere.
    if (row == 0) {
        return {};
    }

    // Only rebalance when the date moves into a new month.
    if (!is_new_month(prices.date(row - 1), prices.date(row))) {
        return {};
    }

    // The strategy chooses when to trade.
    // Portfolio handles the actual accounting.
    return portfolio.rebalance(
        prices.date(row),
        name(),
        prices.assets(),
        prices.prices(row),
        target_weights,
        transaction_cost_rate
    );
}

// This function checks if the date has moved into a new calendar month.
bool FixedWeightMonthlyRebalanceStrategy::is_new_month(
    const std::string& previous_date,
    const std::string& current_date
) {
    if (previous_date.size() < 7 || current_date.size() < 7) {
        return false;
    }

    // Dates are formatted YYYY-MM-DD, so the first seven characters identify
    // the calendar month.
    return previous_date.substr(0, 7) != current_date.substr(0, 7);
}
```

### Backtest Engine Header: `include/backtesting/BacktestEngine.hpp`

Declares the simulation engine and backtest configuration.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

#include "backtesting/PriceTable.hpp"
#include "backtesting/RebalanceStrategy.hpp"
#include "backtesting/Types.hpp"

// Imports std::vector, a dynamic array used to store target weights.
#include <vector>

// Configuration values for one backtest run.
struct BacktestConfig {
    double initial_capital = 100000.0;
    double transaction_cost_bps = 5.0;
};

// BacktestEngine runs the same day-by-day simulation loop for any strategy that implements
// the RebalanceStrategy interface.
class BacktestEngine {
public:
    BacktestEngine(
        const PriceTable& prices,
        std::vector<double> target_weights,
        BacktestConfig config
    );

    // Run one strategy through polymorphism:
    // BacktestEngine does not know the concrete strategy type. It only
    // calls the virtual rebalance() function.
    BacktestResult run(const RebalanceStrategy& strategy) const;

private:
    // The engine holds a reference to the price table,
    // a copy of the target weights, and a copy of the config.
    const PriceTable& prices_;
    std::vector<double> target_weights_;
    BacktestConfig config_;

    // This function fills the daily_returns vector in the BacktestResult by
    // calculating the percentage change in the equity curve from one day to the next.
    static void fill_daily_returns(BacktestResult& result);
};
```

### Backtest Engine Source: `src/backtesting/BacktestEngine.cpp`

Implements the daily backtest loop, polymorphic strategy calls, trade collection, and daily return calculation.

```cpp
#include "backtesting/BacktestEngine.hpp"

// Gives std::runtime_error for throwing exceptions when the input data is invalid.
#include <stdexcept>

// Gives std::move for efficiently moving input vectors into the class without copying.
#include <utility>

// Here we define the constructor and member functions of the BacktestEngine.
BacktestEngine::BacktestEngine(
    const PriceTable& prices,
    std::vector<double> target_weights,
    BacktestConfig config
) : prices_(prices), target_weights_(std::move(target_weights)), config_(config) {
    // Class invariant: one target weight is required for each asset.
    if (target_weights_.size() != prices_.asset_count()) {
        throw std::runtime_error("Target weight count does not match asset count.");
    }
}

// This function runs one strategy through the backtest engine.
// The engine simulates a day-by-day loop, calling the strategy's rebalance()
// function on each row of the price table. The engine records the strategy's trades,
// equity curve, and final weights in a BacktestResult object, which is returned at the end.
BacktestResult BacktestEngine::run(const RebalanceStrategy& strategy) const {
    BacktestResult result;
    result.name = strategy.name();
    result.asset_names = prices_.assets();

    // Every strategy starts from the same initial portfolio.
    Portfolio portfolio(config_.initial_capital, prices_.asset_count());
    portfolio.invest_to_target(prices_.prices(0), target_weights_);

    // Record day 0 immediately after initial investment.
    result.dates.push_back(prices_.date(0));
    result.equity_curve.push_back(portfolio.value(prices_.prices(0)));
    result.final_weights = portfolio.weights(prices_.prices(0));

    // Convert basis points into decimal cost.
    // Example: 5 bps = 0.0005.
    const double cost_rate = config_.transaction_cost_bps / 10000.0;

    for (std::size_t row = 1; row < prices_.row_count(); ++row) {
        // polymorphism line:
        // BacktestEngine does not know the concrete strategy type. It only
        // calls the virtual rebalance() function.
        std::vector<Trade> trades = strategy.rebalance(
            row,
            portfolio,
            prices_,
            target_weights_,
            cost_rate
        );

        // Store trades and turnover generated by the strategy.
        for (const auto& trade : trades) {
            result.total_turnover += trade.turnover;
            result.trades.push_back(trade);
        }

        // Record portfolio state after any trades on this date.
        result.dates.push_back(prices_.date(row));
        result.equity_curve.push_back(portfolio.value(prices_.prices(row)));
        result.final_weights = portfolio.weights(prices_.prices(row));
    }

    fill_daily_returns(result);
    return result;
}

// This function fills the daily_returns vector in the BacktestResult by
// calculating the percentage change in the equity curve from one day to the next.
void BacktestEngine::fill_daily_returns(BacktestResult& result) {
    result.daily_returns.clear();

    // We need at least 2 days to calculate a return. If we don't have enough data,
    // we leave daily_returns empty.
    if (result.equity_curve.size() < 2) {
        return;
    }

    // We reserve space in the daily_returns vector to avoid multiple reallocations
    // as we push back returns. This is an optimization to improve performance.
    result.daily_returns.reserve(result.equity_curve.size() - 1);

    // We loop through the equity curve starting from the second day, calculate the
    // return as (today / yesterday - 1), and push it into the daily_returns vector.
    for (std::size_t i = 1; i < result.equity_curve.size(); ++i) {
        const double yesterday = result.equity_curve[i - 1];
        const double today = result.equity_curve[i];
        result.daily_returns.push_back(today / yesterday - 1.0);
    }
}
```

### Optimizer Header: `include/portfolio_optimizer/OptimizationEngine.hpp`

Declares portfolio optimization helper functions such as inverse-volatility weights and covariance calculations.

```cpp
#pragma once

#include "backtesting/PriceTable.hpp"

#include <cstddef>
#include <vector>

class OptimizationEngine {
public:
    // Computes daily simple returns for each asset using:
    // return = (price_today / price_yesterday) - 1
    static std::vector<std::vector<double>> compute_returns(
        const PriceTable& prices,
        std::size_t end_row,
        std::size_t lookback
    );

    // Computes the volatility of each asset over the lookback window.
    static std::vector<double> compute_volatility(
        const PriceTable& prices,
        std::size_t end_row,
        std::size_t lookback
    );

    // Computes inverse-volatility portfolio weights.
    //
    // Assets with lower volatility receive larger weights.
    // The output vector sums to 1.0.
    static std::vector<double> inverse_volatility_weights(
        const PriceTable& prices,
        std::size_t end_row,
        std::size_t lookback
    );

    // Computes the covariance matrix of asset returns.
    static std::vector<std::vector<double>> compute_covariance_matrix(
        const PriceTable& prices,
        std::size_t end_row,
        std::size_t lookback
    );

    // Computes portfolio variance:
    // w^T Sigma w
    static double portfolio_variance(
        const std::vector<double>& weights,
        const std::vector<std::vector<double>>& covariance_matrix
    );

private:
    // Normalizes weights so they sum to 1.0.
    static std::vector<double> normalize_weights(
        const std::vector<double>& raw_weights
    );
};
```

### Optimizer Source: `src/portfolio_optimizer/OptimizationEngine.cpp`

Implements return, volatility, covariance, inverse-volatility weight, and variance calculations.

```cpp
#include "portfolio_optimizer/OptimizationEngine.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

std::vector<std::vector<double>> OptimizationEngine::compute_returns(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback
) {
    if (prices.row_count() == 0 || prices.asset_count() == 0) {
        throw std::runtime_error("PriceTable is empty.");
    }

    if (end_row >= prices.row_count()) {
        throw std::out_of_range("end_row is outside PriceTable.");
    }

    if (end_row < lookback) {
        throw std::runtime_error("Not enough price history for lookback window.");
    }

    const std::size_t asset_count = prices.asset_count();

    std::vector<std::vector<double>> returns;
    returns.reserve(lookback);

    for (std::size_t row = end_row - lookback + 1; row <= end_row; ++row) {
        const auto& today_prices = prices.prices(row);
        const auto& yesterday_prices = prices.prices(row - 1);

        std::vector<double> row_returns;
        row_returns.reserve(asset_count);

        for (std::size_t asset = 0; asset < asset_count; ++asset) {
            if (yesterday_prices[asset] <= 0.0) {
                throw std::runtime_error("Invalid previous price found.");
            }

            double r = (today_prices[asset] / yesterday_prices[asset]) - 1.0;
            row_returns.push_back(r);
        }

        returns.push_back(row_returns);
    }

    return returns;
}

std::vector<double> OptimizationEngine::compute_volatility(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback
) {
    auto returns = compute_returns(prices, end_row, lookback);

    const std::size_t n = returns.size();
    const std::size_t asset_count = prices.asset_count();

    std::vector<double> means(asset_count, 0.0);
    std::vector<double> volatilities(asset_count, 0.0);

    for (const auto& row_returns : returns) {
        for (std::size_t asset = 0; asset < asset_count; ++asset) {
            means[asset] += row_returns[asset];
        }
    }

    for (double& mean : means) {
        mean /= static_cast<double>(n);
    }

    for (const auto& row_returns : returns) {
        for (std::size_t asset = 0; asset < asset_count; ++asset) {
            double diff = row_returns[asset] - means[asset];
            volatilities[asset] += diff * diff;
        }
    }

    for (double& vol : volatilities) {
        if (n > 1) {
            vol = std::sqrt(vol / static_cast<double>(n - 1));
        } else {
            vol = 0.0;
        }
    }

    return volatilities;
}

std::vector<double> OptimizationEngine::inverse_volatility_weights(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback
) {
    auto volatilities = compute_volatility(prices, end_row, lookback);

    std::vector<double> raw_weights;
    raw_weights.reserve(volatilities.size());

    for (double vol : volatilities) {
        if (vol <= 0.0) {
            raw_weights.push_back(0.0);
        } else {
            raw_weights.push_back(1.0 / vol);
        }
    }

    return normalize_weights(raw_weights);
}

std::vector<std::vector<double>> OptimizationEngine::compute_covariance_matrix(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback
) {
    auto returns = compute_returns(prices, end_row, lookback);

    const std::size_t n = returns.size();
    const std::size_t asset_count = prices.asset_count();

    std::vector<double> means(asset_count, 0.0);

    for (const auto& row_returns : returns) {
        for (std::size_t asset = 0; asset < asset_count; ++asset) {
            means[asset] += row_returns[asset];
        }
    }

    for (double& mean : means) {
        mean /= static_cast<double>(n);
    }

    std::vector<std::vector<double>> covariance_matrix(
        asset_count,
        std::vector<double>(asset_count, 0.0)
    );

    for (std::size_t i = 0; i < asset_count; ++i) {
        for (std::size_t j = 0; j < asset_count; ++j) {
            double covariance = 0.0;

            for (const auto& row_returns : returns) {
                covariance +=
                    (row_returns[i] - means[i]) *
                    (row_returns[j] - means[j]);
            }

            if (n > 1) {
                covariance_matrix[i][j] = covariance / static_cast<double>(n - 1);
            }
        }
    }

    return covariance_matrix;
}

double OptimizationEngine::portfolio_variance(
    const std::vector<double>& weights,
    const std::vector<std::vector<double>>& covariance_matrix
) {
    const std::size_t n = weights.size();

    if (covariance_matrix.size() != n) {
        throw std::runtime_error("Covariance matrix size does not match weights.");
    }

    double variance = 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        if (covariance_matrix[i].size() != n) {
            throw std::runtime_error("Covariance matrix is not square.");
        }

        for (std::size_t j = 0; j < n; ++j) {
            variance += weights[i] * covariance_matrix[i][j] * weights[j];
        }
    }

    return variance;
}

std::vector<double> OptimizationEngine::normalize_weights(
    const std::vector<double>& raw_weights
) {
    double total = 0.0;

    for (double weight : raw_weights) {
        total += weight;
    }

    if (total <= 0.0) {
        const double equal_weight = 1.0 / static_cast<double>(raw_weights.size());
        return std::vector<double>(raw_weights.size(), equal_weight);
    }

    std::vector<double> normalized;
    normalized.reserve(raw_weights.size());

    for (double weight : raw_weights) {
        normalized.push_back(weight / total);
    }

    return normalized;
}
```

### Optimized Strategy Header: `include/portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.hpp`

Declares the optimized monthly rebalance strategy that plugs into RebalanceStrategy.

```cpp
#pragma once

#include "backtesting/RebalanceStrategy.hpp"
#include "OptimizationEngine.hpp"

#include <cstddef>
#include <string>
#include <vector>

class OptimizedMonthlyRebalanceStrategy : public RebalanceStrategy {
public:
    explicit OptimizedMonthlyRebalanceStrategy(std::size_t lookback_days = 60);

    std::string name() const override;

    std::vector<Trade> rebalance(
        std::size_t row,
        Portfolio& portfolio,
        const PriceTable& prices,
        const std::vector<double>& target_weights,
        double transaction_cost_rate
    ) const override;

private:
    std::size_t lookback_days_;
};
```

### Optimized Strategy Source: `src/portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.cpp`

Implements monthly inverse-volatility rebalancing through the same strategy interface as the baseline strategies.

```cpp
#include "portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.hpp"

#include <vector>

OptimizedMonthlyRebalanceStrategy::OptimizedMonthlyRebalanceStrategy(
    std::size_t lookback_days
)
    : lookback_days_(lookback_days) {}

std::string OptimizedMonthlyRebalanceStrategy::name() const {
    return "Optimized Monthly Rebalance";
}

std::vector<Trade> OptimizedMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    (void)target_weights;

    if (row == 0) {
        return {};
    }

    const std::string& today = prices.date(row);
    const std::string& yesterday = prices.date(row - 1);

    const std::string today_month = today.substr(0, 7);
    const std::string yesterday_month = yesterday.substr(0, 7);

    const bool is_first_day = row == lookback_days_;
    const bool is_new_month = today_month != yesterday_month;

    if (row < lookback_days_) {
        return {};
    }

    if (!is_first_day && !is_new_month) {
        return {};
    }

    std::vector<double> optimized_weights =
        OptimizationEngine::inverse_volatility_weights(
            prices,
            row,
            lookback_days_
        );

    return portfolio.rebalance(
        today,
        name(),
        prices.assets(),
        prices.prices(row),
        optimized_weights,
        transaction_cost_rate
    );
}
```

### Risk Metrics Header: `include/risk/RiskMetrics.hpp`

Declares risk metric outputs and the RiskMetrics calculator.

```cpp
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
```

### Risk Metrics Source: `src/risk/RiskMetrics.cpp`

Implements total return, annualized return, volatility, Sharpe ratio, max drawdown, VaR, CVaR, and rolling volatility.

```cpp
#include "risk/RiskMetrics.hpp"

// Gives std::sort for historical VaR calculation.
#include <algorithm>
// Gives std::sqrt and std::pow for volatility and annualized return.
#include <cmath>
// Gives std::size_t for vector indexes.
#include <cstddef>
// Gives std::accumulate for summing vectors.
#include <numeric>
// Gives std::runtime_error for invalid inputs.
#include <stdexcept>

double RiskMetrics::mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    const double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / static_cast<double>(values.size());
}

double RiskMetrics::stddev(const std::vector<double>& values, double average) {
    if (values.size() < 2) {
        return 0.0;
    }

    double squared_sum = 0.0;
    for (double value : values) {
        const double difference = value - average;
        squared_sum += difference * difference;
    }

    // Sample standard deviation divides by n - 1.
    return std::sqrt(squared_sum / static_cast<double>(values.size() - 1));
}

double RiskMetrics::compute_max_drawdown(const std::vector<double>& equity_curve) {
    if (equity_curve.size() < 2) {
        return 0.0;
    }

    double peak = equity_curve.front();
    double max_drawdown = 0.0;

    for (std::size_t i = 1; i < equity_curve.size(); ++i) {
        if (equity_curve[i] > peak) {
            peak = equity_curve[i];
        }

        const double drawdown = (peak - equity_curve[i]) / peak;
        if (drawdown > max_drawdown) {
            max_drawdown = drawdown;
        }
    }

    return max_drawdown;
}

double RiskMetrics::compute_var(const std::vector<double>& returns, double alpha) {
    if (returns.empty()) {
        return 0.0;
    }

    // Historical VaR: sort returns from worst to best and take the left-tail percentile.
    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());

    std::size_t index = static_cast<std::size_t>(
        std::floor(alpha * static_cast<double>(sorted_returns.size()))
    );
    if (index >= sorted_returns.size()) {
        index = sorted_returns.size() - 1;
    }

    // Return VaR as a positive loss number.
    return -sorted_returns[index];
}

double RiskMetrics::compute_cvar(const std::vector<double>& returns, double var_threshold) {
    double sum = 0.0;
    int count = 0;

    for (double daily_return : returns) {
        if (daily_return <= -var_threshold) {
            sum += daily_return;
            ++count;
        }
    }

    // If no return falls through the threshold, use VaR as the conservative fallback.
    if (count == 0) {
        return var_threshold;
    }

    return -(sum / static_cast<double>(count));
}

std::vector<double> RiskMetrics::compute_rolling_volatility(
    const std::vector<double>& returns,
    int window
) {
    std::vector<double> rolling_volatility;
    if (window <= 1 || static_cast<int>(returns.size()) < window) {
        return rolling_volatility;
    }

    rolling_volatility.reserve(returns.size() - static_cast<std::size_t>(window) + 1);

    for (std::size_t end = static_cast<std::size_t>(window); end <= returns.size(); ++end) {
        double window_sum = 0.0;
        for (std::size_t i = end - static_cast<std::size_t>(window); i < end; ++i) {
            window_sum += returns[i];
        }

        const double window_mean = window_sum / static_cast<double>(window);

        double squared_sum = 0.0;
        for (std::size_t i = end - static_cast<std::size_t>(window); i < end; ++i) {
            const double difference = returns[i] - window_mean;
            squared_sum += difference * difference;
        }

        const double daily_volatility = std::sqrt(squared_sum / static_cast<double>(window - 1));
        rolling_volatility.push_back(daily_volatility * std::sqrt(252.0));
    }

    return rolling_volatility;
}

RiskReport RiskMetrics::compute(const BacktestResult& result, double risk_free_rate) const {
    if (result.equity_curve.size() < 2) {
        throw std::runtime_error("RiskMetrics: equity curve is too short.");
    }
    if (result.daily_returns.empty()) {
        throw std::runtime_error("RiskMetrics: daily returns are empty.");
    }

    RiskReport report;

    report.total_return = result.equity_curve.back() / result.equity_curve.front() - 1.0;

    const double num_days = static_cast<double>(result.equity_curve.size() - 1);
    report.annualized_return = std::pow(1.0 + report.total_return, 252.0 / num_days) - 1.0;

    const double average_daily_return = mean(result.daily_returns);
    const double daily_volatility = stddev(result.daily_returns, average_daily_return);
    report.annualized_volatility = daily_volatility * std::sqrt(252.0);

    if (report.annualized_volatility > 1e-12) {
        report.sharpe_ratio =
            (report.annualized_return - risk_free_rate) / report.annualized_volatility;
    }

    report.max_drawdown = compute_max_drawdown(result.equity_curve);
    report.var_95 = compute_var(result.daily_returns, 0.05);
    report.cvar_95 = compute_cvar(result.daily_returns, report.var_95);
    report.rolling_volatility = compute_rolling_volatility(result.daily_returns, 21);

    return report;
}
```

### Monte Carlo Header: `include/risk/MonteCarloSimulator.hpp`

Declares Monte Carlo simulation outputs and interface.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// MonteCarloSimulator consumes BacktestResult from the official backtesting module.
#include "backtesting/Types.hpp"

// Imports std::vector for simulated terminal values.
#include <vector>

// MonteCarloResult stores the distribution of simulated future portfolio values.
struct MonteCarloResult {
    double mean_terminal = 0.0;
    double p5 = 0.0;
    double p25 = 0.0;
    double median = 0.0;
    double p75 = 0.0;
    double p95 = 0.0;
    std::vector<double> terminal_values;
};

// MonteCarloSimulator uses the historical daily return mean and volatility from
// a BacktestResult to simulate future terminal portfolio values.
class MonteCarloSimulator {
public:
    MonteCarloResult simulate(
        const BacktestResult& result,
        int num_simulations = 10000,
        int horizon_days = 252,
        unsigned int seed = 0
    ) const;

private:
    static double percentile(const std::vector<double>& sorted_values, double p);
};
```

### Monte Carlo Source: `src/risk/MonteCarloSimulator.cpp`

Implements normal-return Monte Carlo simulation of future terminal portfolio values.

```cpp
#include "risk/MonteCarloSimulator.hpp"

// Gives std::sort for percentile calculations.
#include <algorithm>
// Gives std::floor and std::sqrt.
#include <cmath>
// Gives std::accumulate for mean return calculations.
#include <numeric>
// Gives random number generators and normal distribution.
#include <random>
// Gives std::runtime_error for invalid inputs.
#include <stdexcept>
#include <utility>

double MonteCarloSimulator::percentile(const std::vector<double>& sorted_values, double p) {
    if (sorted_values.empty()) {
        return 0.0;
    }

    std::size_t index = static_cast<std::size_t>(
        std::floor(p * static_cast<double>(sorted_values.size()))
    );
    if (index >= sorted_values.size()) {
        index = sorted_values.size() - 1;
    }

    return sorted_values[index];
}

MonteCarloResult MonteCarloSimulator::simulate(
    const BacktestResult& result,
    int num_simulations,
    int horizon_days,
    unsigned int seed
) const {
    if (result.daily_returns.size() < 2) {
        throw std::runtime_error("MonteCarloSimulator: not enough daily return data.");
    }
    if (num_simulations <= 0 || horizon_days <= 0) {
        throw std::runtime_error("MonteCarloSimulator: simulation counts must be positive.");
    }

    const double sum = std::accumulate(result.daily_returns.begin(), result.daily_returns.end(), 0.0);
    const double mean_return = sum / static_cast<double>(result.daily_returns.size());

    double squared_sum = 0.0;
    for (double daily_return : result.daily_returns) {
        const double difference = daily_return - mean_return;
        squared_sum += difference * difference;
    }
    const double volatility =
        std::sqrt(squared_sum / static_cast<double>(result.daily_returns.size() - 1));

    std::mt19937 generator;
    if (seed != 0) {
        generator.seed(seed);
    } else {
        std::random_device random_device;
        generator.seed(random_device());
    }

    std::normal_distribution<double> distribution(mean_return, volatility);

    const double start_value = result.equity_curve.back();
    std::vector<double> terminal_values(static_cast<std::size_t>(num_simulations), 0.0);

    for (int simulation = 0; simulation < num_simulations; ++simulation) {
        double value = start_value;
        for (int day = 0; day < horizon_days; ++day) {
            value *= (1.0 + distribution(generator));
        }
        terminal_values[static_cast<std::size_t>(simulation)] = value;
    }

    std::vector<double> sorted_values = terminal_values;
    std::sort(sorted_values.begin(), sorted_values.end());

    MonteCarloResult output;
    output.terminal_values = std::move(terminal_values);
    output.mean_terminal =
        std::accumulate(sorted_values.begin(), sorted_values.end(), 0.0)
        / static_cast<double>(sorted_values.size());
    output.p5 = percentile(sorted_values, 0.05);
    output.p25 = percentile(sorted_values, 0.25);
    output.median = percentile(sorted_values, 0.50);
    output.p75 = percentile(sorted_values, 0.75);
    output.p95 = percentile(sorted_values, 0.95);

    return output;
}
```

### Stress Test Header: `include/risk/StressTestEngine.hpp`

Declares deterministic stress-test scenarios and outputs.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// StressTestEngine consumes BacktestResult from the official backtesting module.
#include "backtesting/Types.hpp"

// Imports std::string for scenario names and std::vector for shock vectors.
#include <string>
#include <vector>

// One deterministic stress scenario.
// shocks[i] must correspond to result.asset_names[i] and result.final_weights[i].
struct StressScenario {
    std::string name;
    std::vector<double> shocks;
};

// Output from applying one scenario to the final portfolio weights.
struct StressResult {
    std::string scenario_name;
    double portfolio_impact = 0.0;
    std::vector<double> asset_impacts;
};

// StressTestEngine applies simple scenario shocks to final portfolio weights.
class StressTestEngine {
public:
    void add_scenario(const std::string& name, const std::vector<double>& shocks);

    // Load default scenarios for the current asset universe.
    // Current backtesting CSV assets: SPY, TLT, GLD, BTC-USD.
    void load_defaults(const std::vector<std::string>& asset_names);

    std::vector<StressResult> run(const BacktestResult& result) const;

private:
    std::vector<StressScenario> scenarios_;
};
```

### Stress Test Source: `src/risk/StressTestEngine.cpp`

Implements default stress scenarios for SPY, TLT, GLD, and BTC-USD.

```cpp
#include "risk/StressTestEngine.hpp"

// Gives std::runtime_error for invalid scenario dimensions.
#include <stdexcept>
#include <utility>

void StressTestEngine::add_scenario(const std::string& name, const std::vector<double>& shocks) {
    scenarios_.push_back({name, shocks});
}

void StressTestEngine::load_defaults(const std::vector<std::string>& asset_names) {
    const std::size_t asset_count = asset_names.size();

    // Helper function:
    // Build a shock vector by matching shock assumptions to actual asset names.
    auto make_shock = [&](double spy_shock, double tlt_shock, double gld_shock, double btc_shock) {
        std::vector<double> shocks(asset_count, 0.0);

        for (std::size_t i = 0; i < asset_count; ++i) {
            const std::string& asset = asset_names[i];
            if (asset == "SPY") {
                shocks[i] = spy_shock;
            } else if (asset == "TLT") {
                shocks[i] = tlt_shock;
            } else if (asset == "GLD") {
                shocks[i] = gld_shock;
            } else if (asset == "BTC-USD") {
                shocks[i] = btc_shock;
            }
        }

        return shocks;
    };

    scenarios_.clear();

    // These are simple deterministic classroom scenarios, not forecasts.
    add_scenario("Equity Shock", make_shock(-0.30, 0.05, 0.08, -0.20));
    add_scenario("Rate Shock", make_shock(-0.05, -0.10, -0.03, -0.05));
    add_scenario("Bitcoin Crash", make_shock(-0.03, 0.02, 0.00, -0.50));
    add_scenario("Inflation Shock", make_shock(-0.10, -0.08, 0.10, -0.05));
}

std::vector<StressResult> StressTestEngine::run(const BacktestResult& result) const {
    std::vector<StressResult> results;
    results.reserve(scenarios_.size());

    for (const auto& scenario : scenarios_) {
        if (scenario.shocks.size() != result.final_weights.size()) {
            throw std::runtime_error("StressTestEngine: shock vector size does not match final weights.");
        }

        StressResult stress_result;
        stress_result.scenario_name = scenario.name;
        stress_result.asset_impacts.resize(scenario.shocks.size(), 0.0);

        for (std::size_t i = 0; i < scenario.shocks.size(); ++i) {
            const double impact = result.final_weights[i] * scenario.shocks[i];
            stress_result.asset_impacts[i] = impact;
            stress_result.portfolio_impact += impact;
        }

        results.push_back(std::move(stress_result));
    }

    return results;
}
```

### Report Printer Header: `include/risk/ReportPrinter.hpp`

Declares CSV report-writing functions.

```cpp
// This tells the compiler to include this header only once, avoiding duplicate definitions.
#pragma once

// ReportPrinter writes outputs from the risk module.
#include "risk/MonteCarloSimulator.hpp"
#include "risk/RiskMetrics.hpp"
#include "risk/StressTestEngine.hpp"

// Imports std::string for output folder paths.
#include <string>
// Imports std::vector for stress-test result lists.
#include <vector>

// ReportPrinter writes CSV files that Python plotting scripts can read later.
class ReportPrinter {
public:
    explicit ReportPrinter(const std::string& output_dir);

    void print_equity_curve(const BacktestResult& result) const;
    void print_trades(const BacktestResult& result) const;
    void print_metrics(const RiskReport& report) const;
    void print_rolling_volatility(const BacktestResult& result, const RiskReport& report) const;
    void print_monte_carlo(const MonteCarloResult& monte_carlo) const;
    void print_stress_tests(
        const std::vector<StressResult>& stress_results,
        const BacktestResult& result
    ) const;

    void print_all(
        const BacktestResult& result,
        const RiskReport& report,
        const MonteCarloResult& monte_carlo,
        const std::vector<StressResult>& stress_results
    ) const;

private:
    std::string output_dir_;

    std::string path(const std::string& filename) const;
};
```

### Report Printer Source: `src/risk/ReportPrinter.cpp`

Writes equity curves, trades, metrics, rolling volatility, Monte Carlo, and stress-test CSV outputs.

```cpp
#include "risk/ReportPrinter.hpp"

// Gives std::filesystem::create_directories for output folder creation.
#include <filesystem>
// Gives std::ofstream for writing CSV files.
#include <fstream>
// Gives std::fixed and std::setprecision for numeric CSV formatting.
#include <iomanip>
// Gives std::cout for a simple completion message.
#include <iostream>
// Gives std::runtime_error for file-writing failures.
#include <stdexcept>

ReportPrinter::ReportPrinter(const std::string& output_dir) : output_dir_(output_dir) {
    if (!output_dir_.empty()) {
        std::filesystem::create_directories(output_dir_);
    }
}

std::string ReportPrinter::path(const std::string& filename) const {
    if (output_dir_.empty()) {
        return filename;
    }
    if (output_dir_.back() == '/') {
        return output_dir_ + filename;
    }
    return output_dir_ + "/" + filename;
}

void ReportPrinter::print_equity_curve(const BacktestResult& result) const {
    std::ofstream file(path("equity_curve.csv"));
    if (!file) {
        throw std::runtime_error("Cannot open equity_curve.csv for writing.");
    }

    file << "date,portfolio_value\n";
    file << std::fixed << std::setprecision(6);
    for (std::size_t i = 0; i < result.dates.size(); ++i) {
        file << result.dates[i] << "," << result.equity_curve[i] << "\n";
    }
}

void ReportPrinter::print_trades(const BacktestResult& result) const {
    std::ofstream file(path("trades.csv"));
    if (!file) {
        throw std::runtime_error("Cannot open trades.csv for writing.");
    }

    file << "date,strategy,asset,shares,price,notional,cost,turnover\n";
    file << std::fixed << std::setprecision(8);
    for (const auto& trade : result.trades) {
        file << trade.date << ","
             << trade.strategy << ","
             << trade.asset << ","
             << trade.shares << ","
             << trade.price << ","
             << trade.notional << ","
             << trade.cost << ","
             << trade.turnover << "\n";
    }
}

void ReportPrinter::print_metrics(const RiskReport& report) const {
    std::ofstream file(path("metrics.csv"));
    if (!file) {
        throw std::runtime_error("Cannot open metrics.csv for writing.");
    }

    file << "metric,value\n";
    file << std::fixed << std::setprecision(8);
    file << "total_return," << report.total_return << "\n";
    file << "annualized_return," << report.annualized_return << "\n";
    file << "annualized_volatility," << report.annualized_volatility << "\n";
    file << "sharpe_ratio," << report.sharpe_ratio << "\n";
    file << "max_drawdown," << report.max_drawdown << "\n";
    file << "var_95," << report.var_95 << "\n";
    file << "cvar_95," << report.cvar_95 << "\n";
}

void ReportPrinter::print_rolling_volatility(
    const BacktestResult& result,
    const RiskReport& report
) const {
    std::ofstream file(path("rolling_volatility.csv"));
    if (!file) {
        throw std::runtime_error("Cannot open rolling_volatility.csv for writing.");
    }

    file << "date,rolling_volatility\n";
    file << std::fixed << std::setprecision(8);

    const std::size_t offset = result.dates.size() - report.rolling_volatility.size();
    for (std::size_t i = 0; i < report.rolling_volatility.size(); ++i) {
        file << result.dates[i + offset] << "," << report.rolling_volatility[i] << "\n";
    }
}

void ReportPrinter::print_monte_carlo(const MonteCarloResult& monte_carlo) const {
    {
        std::ofstream file(path("monte_carlo_summary.csv"));
        if (!file) {
            throw std::runtime_error("Cannot open monte_carlo_summary.csv for writing.");
        }

        file << "statistic,value\n";
        file << std::fixed << std::setprecision(6);
        file << "mean," << monte_carlo.mean_terminal << "\n";
        file << "p5," << monte_carlo.p5 << "\n";
        file << "p25," << monte_carlo.p25 << "\n";
        file << "median," << monte_carlo.median << "\n";
        file << "p75," << monte_carlo.p75 << "\n";
        file << "p95," << monte_carlo.p95 << "\n";
    }

    {
        std::ofstream file(path("monte_carlo_distribution.csv"));
        if (!file) {
            throw std::runtime_error("Cannot open monte_carlo_distribution.csv for writing.");
        }

        file << "terminal_value\n";
        file << std::fixed << std::setprecision(4);
        for (double terminal_value : monte_carlo.terminal_values) {
            file << terminal_value << "\n";
        }
    }
}

void ReportPrinter::print_stress_tests(
    const std::vector<StressResult>& stress_results,
    const BacktestResult& result
) const {
    std::ofstream file(path("stress_tests.csv"));
    if (!file) {
        throw std::runtime_error("Cannot open stress_tests.csv for writing.");
    }

    file << "scenario,portfolio_impact";
    for (const auto& asset : result.asset_names) {
        file << "," << asset << "_impact";
    }
    file << "\n";

    file << std::fixed << std::setprecision(8);
    for (const auto& stress_result : stress_results) {
        file << stress_result.scenario_name << "," << stress_result.portfolio_impact;
        for (double asset_impact : stress_result.asset_impacts) {
            file << "," << asset_impact;
        }
        file << "\n";
    }
}

void ReportPrinter::print_all(
    const BacktestResult& result,
    const RiskReport& report,
    const MonteCarloResult& monte_carlo,
    const std::vector<StressResult>& stress_results
) const {
    print_equity_curve(result);
    print_trades(result);
    print_metrics(report);
    print_rolling_volatility(result, report);
    print_monte_carlo(monte_carlo);
    print_stress_tests(stress_results, result);

    std::cout << "Reports written to: " << output_dir_ << "\n";
}
```

### Python Data Downloader: `main.py`

Downloads real market data with yfinance and writes data/asset_prices.csv for the C++ demo.

```python
"""
FE522 data downloader for the backtesting module.

This file is intentionally Python, not C++.
Python is only used as a support layer to create clean input data.
The actual backtesting, portfolio accounting, and strategy execution stay in C++.

What this script does:
1. Downloads daily adjusted market prices from Yahoo Finance using yfinance.
2. Keeps only one clean price column per asset.
3. Drops dates where any asset is missing a price, so the C++ backtest receives
   a rectangular table with the same assets on every date.
4. Saves the result as data/asset_prices.csv.

CSV format produced:

date,SPY,TLT,GLD,BTC-USD
2020-01-02,299.123456,120.123456,142.123456,6985.470000
...
"""

from pathlib import Path
from typing import Iterable

import sys


# These imports are inside a try/except so a beginner running the file gets a
# clear installation message instead of a long Python traceback.
try:
    import pandas as pd
    import yfinance as yf
except ModuleNotFoundError as error:
    missing_package = error.name
    print(f"Missing Python package: {missing_package}", file=sys.stderr)
    print("Install dependencies with:", file=sys.stderr)
    print("  python3 -m pip install pandas yfinance", file=sys.stderr)
    raise SystemExit(1) from error


# Macro-style asset universe:
# SPY     = US equities
# TLT     = long-duration US Treasury bonds
# GLD     = gold
# BTC-USD = Bitcoin in US dollars
#
# This is a useful set for a portfolio project because the assets have different
# risk drivers instead of all being very similar stocks.
TICKERS = ["SPY", "TLT", "GLD", "BTC-USD"]


# Keep the sample long enough to include multiple market regimes.
START_DATE = "2020-01-01"


# None means "download through the latest available Yahoo Finance date."
END_DATE = None


# The C++ project can treat this file as cleaned input data.
OUTPUT_PATH = Path("data") / "asset_prices.csv"


def extract_close_prices(raw_data: pd.DataFrame, tickers: Iterable[str]) -> pd.DataFrame:
    """Return a clean table containing one closing-price column per ticker.

    yfinance returns a slightly different DataFrame shape depending on whether
    we download one ticker or many tickers. This helper hides that detail from
    the rest of the script.
    """

    ticker_list = list(tickers)

    # Multiple tickers usually produce MultiIndex columns:
    # top level = price field, second level = ticker.
    # Example columns: ("Close", "SPY"), ("Close", "TLT"), ...
    if isinstance(raw_data.columns, pd.MultiIndex):
        if "Close" not in raw_data.columns.get_level_values(0):
            raise ValueError("Downloaded data does not contain a Close price field.")

        close_prices = raw_data["Close"].copy()

        # Keep columns in the exact order defined in TICKERS.
        return close_prices[ticker_list]

    # Single ticker fallback. We do not use it today, but keeping it here makes
    # the script safer if someone later changes TICKERS to only one asset.
    if "Close" not in raw_data.columns:
        raise ValueError("Downloaded data does not contain a Close price field.")

    close_prices = raw_data[["Close"]].copy()
    close_prices.columns = ticker_list
    return close_prices


def download_asset_prices() -> pd.DataFrame:
    """Download adjusted close prices and return a clean DataFrame."""

    raw_data = yf.download(
        TICKERS,
        start=START_DATE,
        end=END_DATE,
        auto_adjust=True,
        progress=False,
    )

    if raw_data.empty:
        raise RuntimeError("Yahoo Finance returned no data.")

    prices = extract_close_prices(raw_data, TICKERS)

    # Convert the index to a normal date string when writing CSV.
    prices.index.name = "date"

    # Keep only rows where every asset has a price.
    # This removes weekends and holidays because ETFs do not trade every day,
    # while BTC trades every day.
    prices = prices.dropna(how="any")

    # Sort by date so the C++ backtest can step forward in time.
    prices = prices.sort_index()

    return prices


def main() -> None:
    """Main entry point for running this file from the command line."""

    prices = download_asset_prices()

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    # Six decimals is enough precision for ETF prices and keeps the CSV readable.
    prices.to_csv(OUTPUT_PATH, float_format="%.6f")

    print(f"Saved {len(prices)} rows to {OUTPUT_PATH}")
    print(f"Assets: {', '.join(prices.columns)}")
    print(f"First date: {prices.index.min().date()}")
    print(f"Last date:  {prices.index.max().date()}")


if __name__ == "__main__":
    main()
```

### Python Data Requirements: `requirements-data.txt`

Lists the Python packages needed to regenerate the price CSV.

```text
pandas
yfinance
```

