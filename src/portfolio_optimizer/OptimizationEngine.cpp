#include "portfolio_optimizer/OptimizationEngine.hpp"

#include <algorithm>
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

std::vector<double> OptimizationEngine::minimum_variance_weights(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback
) {
    auto covariance_matrix = compute_covariance_matrix(prices, end_row, lookback);
    const std::size_t n = covariance_matrix.size();

    if (n == 0) {
        return {};
    }

    std::vector<double> ones(n, 1.0);
    std::vector<double> raw_weights = solve_linear_system(covariance_matrix, ones);

    for (double& weight : raw_weights) {
        if (weight < 0.0) {
            weight = 0.0;
        }
    }

    return normalize_weights(raw_weights);
}

std::vector<double> OptimizationEngine::risk_parity_weights(
    const PriceTable& prices,
    std::size_t end_row,
    std::size_t lookback,
    std::size_t max_iterations,
    double tolerance
) {
    auto covariance_matrix = compute_covariance_matrix(prices, end_row, lookback);
    const std::size_t n = covariance_matrix.size();

    if (n == 0) {
        return {};
    }

    std::vector<double> weights(n, 1.0 / static_cast<double>(n));
    std::vector<double> risk_contributions(n);

    for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
        std::vector<double> marginal_risk = multiply_matrix_vector(covariance_matrix, weights);
        double target_rc = 1.0 / static_cast<double>(n);

        bool valid = true;
        for (std::size_t i = 0; i < n; ++i) {
            if (weights[i] <= 0.0 || marginal_risk[i] <= 0.0) {
                valid = false;
                break;
            }
            risk_contributions[i] = weights[i] * marginal_risk[i];
        }

        if (!valid) {
            weights.assign(n, 1.0 / static_cast<double>(n));
            continue;
        }

        double max_diff = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double desired_ratio = target_rc / risk_contributions[i];
            double alpha = std::sqrt(desired_ratio);
            weights[i] *= alpha;
        }

        weights = normalize_weights(weights);

        for (std::size_t i = 0; i < n; ++i) {
            double current_rc = weights[i] * marginal_risk[i];
            max_diff = std::max(max_diff, std::fabs(current_rc - target_rc));
        }

        if (max_diff < tolerance) {
            break;
        }
    }

    return normalize_weights(weights);
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

std::vector<double> OptimizationEngine::multiply_matrix_vector(
    const std::vector<std::vector<double>>& matrix,
    const std::vector<double>& vector
) {
    const std::size_t n = matrix.size();
    std::vector<double> result(n, 0.0);

    for (std::size_t i = 0; i < n; ++i) {
        if (matrix[i].size() != n) {
            throw std::runtime_error("Covariance matrix is not square.");
        }

        for (std::size_t j = 0; j < n; ++j) {
            result[i] += matrix[i][j] * vector[j];
        }
    }

    return result;
}

std::vector<double> OptimizationEngine::solve_linear_system(
    std::vector<std::vector<double>> matrix,
    std::vector<double> rhs
) {
    const std::size_t n = matrix.size();

    if (rhs.size() != n) {
        throw std::runtime_error("RHS size does not match matrix dimension.");
    }

    for (std::size_t i = 0; i < n; ++i) {
        if (matrix[i].size() != n) {
            throw std::runtime_error("Covariance matrix is not square.");
        }
    }

    for (std::size_t pivot = 0; pivot < n; ++pivot) {
        std::size_t max_row = pivot;
        double max_value = std::fabs(matrix[pivot][pivot]);

        for (std::size_t row = pivot + 1; row < n; ++row) {
            double value = std::fabs(matrix[row][pivot]);
            if (value > max_value) {
                max_value = value;
                max_row = row;
            }
        }

        if (max_value <= 0.0) {
            return std::vector<double>(n, 1.0 / static_cast<double>(n));
        }

        if (max_row != pivot) {
            std::swap(matrix[pivot], matrix[max_row]);
            std::swap(rhs[pivot], rhs[max_row]);
        }

        double pivot_value = matrix[pivot][pivot];
        for (std::size_t col = pivot; col < n; ++col) {
            matrix[pivot][col] /= pivot_value;
        }
        rhs[pivot] /= pivot_value;

        for (std::size_t row = pivot + 1; row < n; ++row) {
            double factor = matrix[row][pivot];
            for (std::size_t col = pivot; col < n; ++col) {
                matrix[row][col] -= factor * matrix[pivot][col];
            }
            rhs[row] -= factor * rhs[pivot];
        }
    }

    std::vector<double> solution(n, 0.0);
    for (std::size_t i = n; i-- > 0;) {
        double sum = rhs[i];
        for (std::size_t j = i + 1; j < n; ++j) {
            sum -= matrix[i][j] * solution[j];
        }
        solution[i] = sum;
    }

    return solution;
}