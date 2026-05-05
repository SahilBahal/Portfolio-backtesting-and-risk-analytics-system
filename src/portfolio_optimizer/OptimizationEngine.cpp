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