cmake_minimum_required(VERSION 3.24)

project(source VERSION 0.1.0 LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_library(backtest_core
  app/app.cc
  src/data/lob_reader.cc
  src/data/trades_reader.cc
  src/replay/event_merger.cc
  src/replay/replay_engine.cc
  src/execution/execution_model.cc
  src/strategy/simple_strategy.cc
  src/strategy/as_strategy.cc 
  src/metrics/pnl_tracker.cc
)

target_include_directories(backtest_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(backtest_core
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(bt_runner
  cmd/main.cc
)

target_link_libraries(bt_runner
  PRIVATE
    backtest_core
)

add_executable(bt_tests
  tests/test_main.cc
)

target_link_libraries(bt_tests
  PRIVATE
    backtest_core
)

add_test(NAME bt_tests COMMAND bt_tests)
