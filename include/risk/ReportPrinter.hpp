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
