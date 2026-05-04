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
