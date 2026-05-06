"""Create presentation-ready plots from the C++ risk/reporting CSV outputs.

This Python file is intentionally a support layer only:
the C++ program still performs the backtest, risk metrics, Monte Carlo
simulation, stress tests, and CSV reporting.

Typical workflow from the repository root:

    cmake --build build
    ./build/backtesting_demo
    python3 plot_results.py

The script reads the strategy folders under output/ and writes PNG charts
under figures/.
"""

from __future__ import annotations

import os
import tempfile
from pathlib import Path

import pandas as pd

# Matplotlib sometimes tries to write cache files under the user's home folder.
# In some lab or sandbox environments that location is not writable, so we force
# it to use a temporary folder before importing pyplot.
os.environ.setdefault("MPLCONFIGDIR", str(Path(tempfile.gettempdir()) / "fe522_matplotlib"))

import matplotlib.pyplot as plt  # noqa: E402  (must import after MPLCONFIGDIR is set)


ROOT = Path(__file__).resolve().parent
OUTPUT_DIR = ROOT / "output"
FIGURES_DIR = ROOT / "figures"

STRATEGY_FOLDERS = [
    "buy_and_hold",
    "fixed_monthly_rebalance",
    "optimized_monthly_rebalance",
]

DISPLAY_NAMES = {
    "buy_and_hold": "Buy & Hold",
    "fixed_monthly_rebalance": "Fixed Monthly",
    "optimized_monthly_rebalance": "Optimized Monthly",
}


def require_file(path: Path) -> None:
    """Fail early with a clear message if the C++ outputs are missing."""
    if not path.exists():
        raise FileNotFoundError(
            f"Missing {path}. Run ./build/backtesting_demo before plot_results.py."
        )


def load_metric(strategy: str, metric_name: str) -> float:
    """Read one metric value from output/<strategy>/metrics.csv."""
    path = OUTPUT_DIR / strategy / "metrics.csv"
    require_file(path)

    metrics = pd.read_csv(path)
    row = metrics.loc[metrics["metric"] == metric_name]
    if row.empty:
        raise ValueError(f"Metric {metric_name} not found in {path}.")

    return float(row.iloc[0]["value"])


def plot_equity_curves() -> None:
    """Plot all strategy equity curves on one chart."""
    plt.figure(figsize=(11, 6))

    for strategy in STRATEGY_FOLDERS:
        path = OUTPUT_DIR / strategy / "equity_curve.csv"
        require_file(path)

        equity = pd.read_csv(path, parse_dates=["date"])
        plt.plot(
            equity["date"],
            equity["portfolio_value"],
            linewidth=2.0,
            label=DISPLAY_NAMES[strategy],
        )

    plt.title("Portfolio Equity Curves")
    plt.xlabel("Date")
    plt.ylabel("Portfolio Value ($)")
    plt.grid(True, alpha=0.25)
    plt.legend()
    plt.tight_layout()
    plt.savefig(FIGURES_DIR / "equity_curves.png", dpi=160)
    plt.close()


def plot_key_metrics() -> None:
    """Plot the main risk/performance metrics as grouped bars."""
    metric_names = [
        "annualized_return",
        "annualized_volatility",
        "max_drawdown",
        "sharpe_ratio",
    ]
    metric_labels = [
        "Ann. Return",
        "Ann. Volatility",
        "Max Drawdown",
        "Sharpe",
    ]

    rows = []
    for strategy in STRATEGY_FOLDERS:
        row = {"strategy": DISPLAY_NAMES[strategy]}
        for metric_name in metric_names:
            row[metric_name] = load_metric(strategy, metric_name)
        rows.append(row)

    metrics = pd.DataFrame(rows)

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    percent_metrics = ["annualized_return", "annualized_volatility", "max_drawdown"]
    percent_data = metrics.copy()
    for metric_name in percent_metrics:
        percent_data[metric_name] = percent_data[metric_name] * 100.0

    percent_data.plot(
        x="strategy",
        y=percent_metrics,
        kind="bar",
        ax=axes[0],
        width=0.72,
    )
    axes[0].set_title("Return and Risk")
    axes[0].set_xlabel("")
    axes[0].set_ylabel("Percent")
    axes[0].set_xticklabels(metrics["strategy"], rotation=0)
    axes[0].legend(["Ann. Return", "Ann. Volatility", "Max Drawdown"])
    axes[0].grid(True, axis="y", alpha=0.25)

    metrics.plot(
        x="strategy",
        y="sharpe_ratio",
        kind="bar",
        ax=axes[1],
        legend=False,
        color="#2c7fb8",
        width=0.55,
    )
    axes[1].set_title("Sharpe Ratio")
    axes[1].set_xlabel("")
    axes[1].set_ylabel("Ratio")
    axes[1].set_xticklabels(metrics["strategy"], rotation=0)
    axes[1].grid(True, axis="y", alpha=0.25)

    fig.suptitle("Key Strategy Metrics")
    fig.tight_layout()
    fig.savefig(FIGURES_DIR / "key_metrics.png", dpi=160)
    plt.close(fig)


def plot_monte_carlo_ranges() -> None:
    """Plot one-year Monte Carlo p5/median/p95 terminal value ranges."""
    rows = []
    for strategy in STRATEGY_FOLDERS:
        path = OUTPUT_DIR / strategy / "monte_carlo_summary.csv"
        require_file(path)

        summary = pd.read_csv(path)
        values = dict(zip(summary["statistic"], summary["value"]))
        rows.append(
            {
                "strategy": DISPLAY_NAMES[strategy],
                "p5": float(values["p5"]),
                "median": float(values["median"]),
                "p95": float(values["p95"]),
            }
        )

    mc = pd.DataFrame(rows)

    plt.figure(figsize=(10, 5))
    y_positions = range(len(mc))

    for y_pos, row in zip(y_positions, mc.itertuples(index=False)):
        plt.hlines(y=y_pos, xmin=row.p5, xmax=row.p95, linewidth=5, color="#9ecae1")
        plt.scatter(row.median, y_pos, color="#08519c", s=70, zorder=3)
        plt.text(row.p5, y_pos + 0.16, f"p5 ${row.p5:,.0f}", ha="left", fontsize=8)
        plt.text(row.p95, y_pos + 0.16, f"p95 ${row.p95:,.0f}", ha="right", fontsize=8)

    plt.yticks(list(y_positions), mc["strategy"])
    plt.title("Monte Carlo One-Year Terminal Value Range")
    plt.xlabel("Terminal Portfolio Value ($)")
    plt.grid(True, axis="x", alpha=0.25)
    plt.tight_layout()
    plt.savefig(FIGURES_DIR / "monte_carlo_ranges.png", dpi=160)
    plt.close()


def plot_stress_tests() -> None:
    """Plot scenario portfolio impacts for each strategy."""
    rows = []
    for strategy in STRATEGY_FOLDERS:
        path = OUTPUT_DIR / strategy / "stress_tests.csv"
        require_file(path)

        stress = pd.read_csv(path)
        for _, row in stress.iterrows():
            rows.append(
                {
                    "strategy": DISPLAY_NAMES[strategy],
                    "scenario": row["scenario"],
                    "portfolio_impact": float(row["portfolio_impact"]) * 100.0,
                }
            )

    stress_data = pd.DataFrame(rows)
    pivot = stress_data.pivot(
        index="scenario",
        columns="strategy",
        values="portfolio_impact",
    )

    ax = pivot.plot(kind="bar", figsize=(11, 5), width=0.75)
    ax.set_title("Stress Test Portfolio Impact")
    ax.set_xlabel("")
    ax.set_ylabel("Portfolio Impact (%)")
    ax.axhline(0.0, color="black", linewidth=0.8)
    ax.grid(True, axis="y", alpha=0.25)
    ax.legend(title="")
    plt.xticks(rotation=0)
    plt.tight_layout()
    plt.savefig(FIGURES_DIR / "stress_tests.png", dpi=160)
    plt.close()


def main() -> None:
    """Create all plots."""
    FIGURES_DIR.mkdir(exist_ok=True)

    plot_equity_curves()
    plot_key_metrics()
    plot_monte_carlo_ranges()
    plot_stress_tests()

    print(f"Plots written to: {FIGURES_DIR}")


if __name__ == "__main__":
    main()
