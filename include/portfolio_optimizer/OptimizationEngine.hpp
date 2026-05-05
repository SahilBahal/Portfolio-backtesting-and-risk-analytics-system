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