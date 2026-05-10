# Project Structure

## Layout

```text
.
├── CMakeLists.txt
├── Makefile
├── README.md
├── MD/                      # historical input data
├── results/                 # generated run reports
├── docs/
│   └── PROJECT_STRUCTURE.md
├── cmd/
│   └── main.cc              # CLI entry point
├── app/
│   └── app.cc               # top-level wiring and report output
├── include/
│   └── btfw/
│       ├── app/
│       │   └── application.hpp
│       ├── core/
│       │   ├── config.hpp
│       │   ├── event.hpp
│       │   ├── market_state.hpp
│       │   └── types.hpp
│       ├── data/
│       │   ├── lob_reader.hpp
│       │   └── trades_reader.hpp
│       ├── execution/
│       │   ├── execution_model.hpp
│       │   ├── fill.hpp
│       │   └── order.hpp
│       ├── metrics/
│       │   └── pnl_tracker.hpp
│       ├── replay/
│       │   ├── event_merger.hpp
│       │   └── replay_engine.hpp
│       └── strategy/
│           ├── simple_strategy.hpp
│           └── strategy.hpp
├── src/
│   ├── data/
│   │   ├── lob_reader.cc
│   │   └── trades_reader.cc
│   ├── execution/
│   │   └── execution_model.cc
│   ├── metrics/
│   │   └── pnl_tracker.cc
│   ├── replay/
│   │   ├── event_merger.cc
│   │   └── replay_engine.cc
│   └── strategy/
│       └── simple_strategy.cc
└── tests/
    └── test_main.cc
```

## Module Boundaries

- `cmd/` contains only the executable entry point and CLI argument parsing.
- `app/` wires the modules together and writes the final report.
- `include/btfw/core/` contains shared types used across the project.
- `include/btfw/data/` and `src/data/` handle streaming input from historical CSV files.
- `include/btfw/replay/` and `src/replay/` drive market replay and event ordering.
- `include/btfw/execution/` and `src/execution/` define order/fill types and the fill rule.
- `include/btfw/strategy/` and `src/strategy/` contain strategy interfaces and implementations.
- `include/btfw/metrics/` and `src/metrics/` track portfolio state and PnL.
- `tests/` holds small unit tests for core logic.

## Dependency Direction

`data -> replay -> strategy/execution -> metrics -> app`

`cmd` should depend only on `app`.
