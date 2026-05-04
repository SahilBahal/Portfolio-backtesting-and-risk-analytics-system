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
