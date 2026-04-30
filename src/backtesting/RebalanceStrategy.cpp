#include "backtesting/RebalanceStrategy.hpp"

// Here we define the member functions of the strategy classes declared in RebalanceStrategy.hpp.
std::string BuyAndHoldStrategy::name() const {
    return "Buy and Hold";
}

std::vector<Trade> BuyAndHoldStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    // Buy-and-hold never trades after the initial investment.
    // These casts avoid compiler warnings for intentionally unused parameters.
    (void)row;
    (void)portfolio;
    (void)prices;
    (void)target_weights;
    (void)transaction_cost_rate;

    return {};
}

// Fixed monthly rebalancing restores target weights at the first trading day
// of each new calendar month. The strategy checks the date on every row and
// only trades when the month changes. The strategy calls Portfolio::rebalance()
// to do the actual trading and accounting.
std::string FixedWeightMonthlyRebalanceStrategy::name() const {
    return "Fixed Monthly Rebalance";
}

// This function checks if the date has moved into a new calendar month and only
// rebalances on the first trading day of each new month.
std::vector<Trade> FixedWeightMonthlyRebalanceStrategy::rebalance(
    std::size_t row,
    Portfolio& portfolio,
    const PriceTable& prices,
    const std::vector<double>& target_weights,
    double transaction_cost_rate
) const {
    // Row 0 has no previous date. The engine normally starts calling strategies
    // at row 1, but this guard keeps the class safe if reused elsewhere.
    if (row == 0) {
        return {};
    }

    // Only rebalance when the date moves into a new month.
    if (!is_new_month(prices.date(row - 1), prices.date(row))) {
        return {};
    }

    // The strategy chooses when to trade.
    // Portfolio handles the actual accounting.
    return portfolio.rebalance(
        prices.date(row),
        name(),
        prices.assets(),
        prices.prices(row),
        target_weights,
        transaction_cost_rate
    );
}

// This function checks if the date has moved into a new calendar month.
bool FixedWeightMonthlyRebalanceStrategy::is_new_month(
    const std::string& previous_date,
    const std::string& current_date
) {
    if (previous_date.size() < 7 || current_date.size() < 7) {
        return false;
    }

    // Dates are formatted YYYY-MM-DD, so the first seven characters identify
    // the calendar month.
    return previous_date.substr(0, 7) != current_date.substr(0, 7);
}
