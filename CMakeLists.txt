cmake_minimum_required(VERSION 3.16)

project(FE522_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json so VS Code / clangd can read the same include
# paths and compiler flags that CMake uses during the real build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(backtesting
    src/backtesting/BacktestEngine.cpp
    src/backtesting/CsvReader.cpp
    src/backtesting/Portfolio.cpp
    src/backtesting/PriceTable.cpp
    src/backtesting/RebalanceStrategy.cpp
)

target_include_directories(backtesting PUBLIC include)

if (MSVC)
    target_compile_options(backtesting PRIVATE /W4)
else()
    target_compile_options(backtesting PRIVATE -Wall -Wextra -pedantic)
endif()

add_library(risk
    src/risk/MonteCarloSimulator.cpp
    src/risk/ReportPrinter.cpp
    src/risk/RiskMetrics.cpp
    src/risk/StressTestEngine.cpp
)

target_include_directories(risk PUBLIC include)
target_link_libraries(risk PUBLIC backtesting)

if (MSVC)
    target_compile_options(risk PRIVATE /W4)
else()
    target_compile_options(risk PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(backtesting_demo
    src/main_demo.cpp
)

target_link_libraries(backtesting_demo PRIVATE risk)
