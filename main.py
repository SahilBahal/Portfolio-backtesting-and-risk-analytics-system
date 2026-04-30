"""
FE522 data downloader for the backtesting module.

This file is intentionally Python, not C++.
Python is only used as a support layer to create clean input data.
The actual backtesting, portfolio accounting, and strategy execution stay in C++.

What this script does:
1. Downloads daily adjusted market prices from Yahoo Finance using yfinance.
2. Keeps only one clean price column per asset.
3. Drops dates where any asset is missing a price, so the C++ backtest receives
   a rectangular table with the same assets on every date.
4. Saves the result as data/asset_prices.csv.

CSV format produced:

date,SPY,TLT,GLD,BTC-USD
2020-01-02,299.123456,120.123456,142.123456,6985.470000
...
"""

from pathlib import Path
from typing import Iterable

import sys


# These imports are inside a try/except so a beginner running the file gets a
# clear installation message instead of a long Python traceback.
try:
    import pandas as pd
    import yfinance as yf
except ModuleNotFoundError as error:
    missing_package = error.name
    print(f"Missing Python package: {missing_package}", file=sys.stderr)
    print("Install dependencies with:", file=sys.stderr)
    print("  python3 -m pip install pandas yfinance", file=sys.stderr)
    raise SystemExit(1) from error


# Macro-style asset universe:
# SPY     = US equities
# TLT     = long-duration US Treasury bonds
# GLD     = gold
# BTC-USD = Bitcoin in US dollars
#
# This is a useful set for a portfolio project because the assets have different
# risk drivers instead of all being very similar stocks.
TICKERS = ["SPY", "TLT", "GLD", "BTC-USD"]


# Keep the sample long enough to include multiple market regimes.
START_DATE = "2020-01-01"


# None means "download through the latest available Yahoo Finance date."
END_DATE = None


# The C++ project can treat this file as cleaned input data.
OUTPUT_PATH = Path("data") / "asset_prices.csv"


def extract_close_prices(raw_data: pd.DataFrame, tickers: Iterable[str]) -> pd.DataFrame:
    """Return a clean table containing one closing-price column per ticker.

    yfinance returns a slightly different DataFrame shape depending on whether
    we download one ticker or many tickers. This helper hides that detail from
    the rest of the script.
    """

    ticker_list = list(tickers)

    # Multiple tickers usually produce MultiIndex columns:
    # top level = price field, second level = ticker.
    # Example columns: ("Close", "SPY"), ("Close", "TLT"), ...
    if isinstance(raw_data.columns, pd.MultiIndex):
        if "Close" not in raw_data.columns.get_level_values(0):
            raise ValueError("Downloaded data does not contain a Close price field.")

        close_prices = raw_data["Close"].copy()

        # Keep columns in the exact order defined in TICKERS.
        return close_prices[ticker_list]

    # Single ticker fallback. We do not use it today, but keeping it here makes
    # the script safer if someone later changes TICKERS to only one asset.
    if "Close" not in raw_data.columns:
        raise ValueError("Downloaded data does not contain a Close price field.")

    close_prices = raw_data[["Close"]].copy()
    close_prices.columns = ticker_list
    return close_prices


def download_asset_prices() -> pd.DataFrame:
    """Download adjusted close prices and return a clean DataFrame."""

    raw_data = yf.download(
        TICKERS,
        start=START_DATE,
        end=END_DATE,
        auto_adjust=True,
        progress=False,
    )

    if raw_data.empty:
        raise RuntimeError("Yahoo Finance returned no data.")

    prices = extract_close_prices(raw_data, TICKERS)

    # Convert the index to a normal date string when writing CSV.
    prices.index.name = "date"

    # Keep only rows where every asset has a price.
    # This removes weekends and holidays because ETFs do not trade every day,
    # while BTC trades every day.
    prices = prices.dropna(how="any")

    # Sort by date so the C++ backtest can step forward in time.
    prices = prices.sort_index()

    return prices


def main() -> None:
    """Main entry point for running this file from the command line."""

    prices = download_asset_prices()

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    # Six decimals is enough precision for ETF prices and keeps the CSV readable.
    prices.to_csv(OUTPUT_PATH, float_format="%.6f")

    print(f"Saved {len(prices)} rows to {OUTPUT_PATH}")
    print(f"Assets: {', '.join(prices.columns)}")
    print(f"First date: {prices.index.min().date()}")
    print(f"Last date:  {prices.index.max().date()}")


if __name__ == "__main__":
    main()
