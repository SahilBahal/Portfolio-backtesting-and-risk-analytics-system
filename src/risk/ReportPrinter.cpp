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
