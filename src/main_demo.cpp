#include "backtesting/BacktestEngine.hpp"
#include "backtesting/RebalanceStrategy.hpp"
#include "backtesting/Types.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

namespace {
void print_result(const BacktestResult& result) {
    std::cout << result.name << "\n";
    std::cout << "  final value: " << std::fixed << std::setprecision(2)
              << result.equity_curve.back() << "\n";
    std::cout << "  trades: " << result.trades.size() << "\n";
    std::cout << "  total turnover: " << std::fixed << std::setprecision(2)
              << result.total_turnover * 100.0 << "%\n\n";
}
}

int main() {
    // Small hard-coded data set used only to prove the backtesting section compiles
    // and runs independently.
    std::vector<std::string> assets = {"SPY", "TLT"};
    std::vector<PriceRow> rows = {
        {"2024-01-02", {100.0, 100.0}},
        {"2024-01-31", {105.0, 99.0}},
        {"2024-02-01", {106.0, 98.0}},
        {"2024-02-29", {110.0, 101.0}},
        {"2024-03-01", {108.0, 103.0}},
        {"2024-03-29", {112.0, 104.0}},
    };

    PriceTable prices(assets, rows);
    std::vector<double> target_weights = {0.60, 0.40};

    BacktestConfig config;
    config.initial_capital = 100000.0;
    config.transaction_cost_bps = 5.0;

    BacktestEngine engine(prices, target_weights, config);

    BuyAndHoldStrategy buy_and_hold;
    FixedWeightMonthlyRebalanceStrategy monthly_rebalance;

    print_result(engine.run(buy_and_hold));
    print_result(engine.run(monthly_rebalance));

    return 0;
}
