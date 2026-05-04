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
