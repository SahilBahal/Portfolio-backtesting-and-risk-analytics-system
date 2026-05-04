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
