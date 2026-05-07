#include "portfolio_optimizer/OptimizedMonthlyRebalanceStrategy.hpp"

#include <vector>

OptimizedMonthlyRebalanceStrategy::OptimizedMonthlyRebalanceStrategy(
    std::size_t lookback_days
)
    : lookback_days_(lookback_days) {}

std::string OptimizedMonthlyRebalanceStrategy::name() const {
    return "Optimized Monthly Rebalance";
}

std::vector<Trade> OptimizedMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    (void)target_weights;

    if (row == 0) {
        return {};
    }

    const std::string& today = prices.date(row);
    const std::string& yesterday = prices.date(row - 1);

    const std::string today_month = today.substr(0, 7);
    const std::string yesterday_month = yesterday.substr(0, 7);

    const bool is_first_day = row == lookback_days_;
    const bool is_new_month = today_month != yesterday_month;

    if (row < lookback_days_) {
        return {};
    }

    if (!is_first_day && !is_new_month) {
        return {};
    }

    std::vector<double> optimized_weights =
        OptimizationEngine::inverse_volatility_weights(
            prices,
            row,
            lookback_days_
        );

    return portfolio.rebalance(
        today,
        name(),
        prices.assets(),
        prices.prices(row),
        optimized_weights,
        transaction_cost_rate
    );
}

MarkowitzMonthlyRebalanceStrategy::MarkowitzMonthlyRebalanceStrategy(
    std::size_t lookback_days
)
    : lookback_days_(lookback_days) {}

std::string MarkowitzMonthlyRebalanceStrategy::name() const {
    return "Markowitz Minimum-Variance Monthly Rebalance";
}

std::vector<Trade> MarkowitzMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    (void)target_weights;

    if (row == 0) {
        return {};
    }

    const std::string& today = prices.date(row);
    const std::string& yesterday = prices.date(row - 1);

    const std::string today_month = today.substr(0, 7);
    const std::string yesterday_month = yesterday.substr(0, 7);

    const bool is_first_day = row == lookback_days_;
    const bool is_new_month = today_month != yesterday_month;

    if (row < lookback_days_) {
        return {};
    }

    if (!is_first_day && !is_new_month) {
        return {};
    }

    std::vector<double> optimized_weights =
        OptimizationEngine::minimum_variance_weights(
            prices,
            row,
            lookback_days_
        );

    return portfolio.rebalance(
        today,
        name(),
        prices.assets(),
        prices.prices(row),
        optimized_weights,
        transaction_cost_rate
    );
}

RiskParityMonthlyRebalanceStrategy::RiskParityMonthlyRebalanceStrategy(
    std::size_t lookback_days
)
    : lookback_days_(lookback_days) {}

std::string RiskParityMonthlyRebalanceStrategy::name() const {
    return "Risk-Parity Monthly Rebalance";
}

std::vector<Trade> RiskParityMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    (void)target_weights;

    if (row == 0) {
        return {};
    }

    const std::string& today = prices.date(row);
    const std::string& yesterday = prices.date(row - 1);

    const std::string today_month = today.substr(0, 7);
    const std::string yesterday_month = yesterday.substr(0, 7);

    const bool is_first_day = row == lookback_days_;
    const bool is_new_month = today_month != yesterday_month;

    if (row < lookback_days_) {
        return {};
    }

    if (!is_first_day && !is_new_month) {
        return {};
    }

    std::vector<double> optimized_weights =
        OptimizationEngine::risk_parity_weights(
            prices,
            row,
            lookback_days_
        );

    return portfolio.rebalance(
        today,
        name(),
        prices.assets(),
        prices.prices(row),
        optimized_weights,
        transaction_cost_rate
    );
}
