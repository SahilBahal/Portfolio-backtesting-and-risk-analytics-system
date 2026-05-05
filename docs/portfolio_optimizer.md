# Portfolio Optimizer Logic

## Overview

The portfolio optimizer implements **inverse-volatility weighting** with a rolling lookback window. This approach dynamically allocates portfolio weights based on recent asset volatility, rather than using fixed target weights.

## Core Optimization Approach

### Inverse-Volatility Weights

The optimizer calculates weights using the following formula:

```
w_i = (1/σ_i) / Σ(1/σ_j)
```

Where:
- `σ_i` is the volatility of asset `i` over the lookback window
- Lower volatility assets receive larger weights
- Higher volatility assets receive smaller weights
- Weights are normalized to sum to 1.0

### Monthly Rebalancing Trigger

- Only rebalances on the first valid date (after lookback window) and at month boundaries
- Avoids constant daily recalculation by computing optimization only when needed
- Uses a 60-day lookback window by default (configurable)

## Implementation Components

| Function | Purpose |
|----------|---------|
| `compute_returns()` | Calculate daily simple returns: `(P_t / P_{t-1}) - 1` |
| `compute_volatility()` | Standard deviation of returns over lookback window |
| `inverse_volatility_weights()` | Inverse-vol allocation (main optimizer) |
| `compute_covariance_matrix()` | Full covariance matrix of asset returns |
| `portfolio_variance()` | Portfolio variance: `w^T Σ w` (used for analysis) |

## Strategy Class

### OptimizedMonthlyRebalanceStrategy

- Extends `RebalanceStrategy` base class (Strategy Pattern)
- Parameters: 60-day lookback window (customizable)
- Ignores `target_weights` parameter—uses dynamically computed inverse-vol weights instead
- Rebalances monthly using the current market regime (recent volatility)

## Key Feature

The optimizer is **reactive and adaptive**:

- During volatile periods: reduces allocation to volatile assets
- During calm periods: can hold more of higher-volatility instruments

This contrasts with fixed allocation strategies that maintain static target weights regardless of market conditions.

## Mathematical Foundation

### Return Calculation
```
r_t = (P_t / P_{t-1}) - 1
```

### Volatility (Standard Deviation)
```
σ = √[ Σ(r_t - μ)^2 / (n-1) ]
```

### Weight Normalization
```
w_i_normalized = w_i_raw / Σ(w_j_raw)
```

## Integration

The optimizer integrates with the backtesting engine through the Strategy Pattern, allowing it to be used alongside buy-and-hold and fixed-weight rebalancing strategies for comparative analysis.