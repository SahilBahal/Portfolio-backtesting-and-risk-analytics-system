# FE522 Project - Manuel Backtesting Module

This branch contains Manuel Bossi's fresh C++ setup for the portfolio backtesting and portfolio-accounting layer.

## What This Branch Adds

- C++17 CMake build
- `Portfolio` accounting class
- `BacktestEngine` day-by-day simulator
- `RebalanceStrategy` interface using inheritance and polymorphism
- Buy-and-hold strategy
- Fixed monthly rebalance strategy
- VS Code / clangd configuration
- Small demo program to verify the module independently

## Build and Run

From the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/manuel_backtesting_demo
```

## Module Layout

```text
include/backtesting/   Public headers for Manuel's module
src/backtesting/       C++ implementations
src/main_demo.cpp      Small test/demo executable
docs/                  Ownership notes
```

This branch is meant to be merged through a pull request into `main`, then integrated with Roberto's optimization module and Sahil's risk/reporting module.
