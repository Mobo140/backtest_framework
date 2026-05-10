# Backtest Framework

Event-driven C++ backtesting framework for market-making strategies on historical LOB data.

## Architecture

```
LobReader ──┐
            ├─► EventMerger ─► ReplayEngine ─► PnlTracker
TradesReader┘                        │
                                     ├─► Strategy (AS-2008 / AS-2018)
                                     ├─► ExecutionModel
                                     └─► EquitySnapshot[]
```

| Component | Role |
|---|---|
| `LobReader` / `TradesReader` | Stream-parse CSV tick files |
| `EventMerger` | Merge LOB and trade events by timestamp |
| `ReplayEngine` | Drive the event loop; manage orders and fills |
| `AsStrategy` | Avellaneda-Stoikov market-maker (2008 + 2018 variants) |
| `ExecutionModel` | Fill orders when market crosses order price |
| `PnlTracker` | Track cash, position, mark-to-market equity |

## Strategies

### Avellaneda-Stoikov 2008
Reference: Avellaneda & Stoikov, *"High-frequency trading in a limit order book"*, QF 2008. DOI: `10.1080/14697680701381228`

Posts bid/ask quotes around a risk-adjusted reservation price:
```
r = mid − q · γ · σ² · (T−t)
δ = (γ · σ² · (T−t)) / 2  +  (1/γ) · ln(1 + γ/κ)
```

### Microprice (Stoikov 2018)
Reference: Stoikov, *"The Micro-Price: A High-Frequency Estimator of Future Prices"*, QF 2018. DOI: `10.1080/14697688.2018.1489139`

Replaces naive mid with a queue-imbalance-weighted fair price:
```
microprice = (ask · bid_qty + bid · ask_qty) / (bid_qty + ask_qty)
```

### A-S 2018 Spread (same paper)
Exponential fill-intensity model gives tighter optimal spreads:
```
δ = (1/κ) · ln(1 + κ/γ)  +  (γ · σ² · (T−t)) / 2
```

## Build

```bash
make build
```

## Test

```bash
make test
```

## Run Experiments

Three strategy variants × four event horizons × five `max_position` values:

```bash
make run-sweep     # raw runs → results/sweeps/max_pos_*/{10k,50k,100k,1b}/run_{A,B,C}/
make analyze-sweep # updates results/report.md, results/report_ru.md, results/max_position_summary.csv
make experiments   # run-sweep + analyze-sweep
```

| Run | Strategy | Flags |
|---|---|---|
| A | AS-2008, naive mid | `--no-as-microprice` |
| B | AS-2008 + Microprice | `--as-microprice` |
| C | AS-2018 + Microprice | `--as-microprice --as-2018-spread` |

## Visualise

```bash
make visualize
# same as make analyze-sweep
```

Or directly:
```bash
./build/bt_runner \
  --lob MD/lob.csv \
  --trades MD/trades.csv \
  --max-events 100000 \
  --report-dir results/my_run \
  --as-microprice \
  --as-2018-spread
```

## CLI Flags

| Flag | Default | Description |
|---|---|---|
| `--lob <path>` | — | LOB CSV path |
| `--trades <path>` | — | Trades CSV path |
| `--max-events <n>` | 5000 | 0 = unlimited |
| `--report-dir <path>` | results | Output directory |
| `--order-qty <x>` | 1.0 | Order size |
| `--as-gamma <x>` | 0.1 | Risk-aversion γ |
| `--as-sigma <x>` | 1e-5 | Volatility σ |
| `--as-kappa <x>` | 1e7 | Arrival rate κ |
| `--as-T <x>` | 3600 | Session duration T |
| `--as-max-position <x>` | 50 | Inventory cap |
| `--as-microprice` | on | Use microprice |
| `--no-as-microprice` | — | Use naive mid |
| `--as-2018-spread` | off | Use 2018 spread formula |

## Output

Each run writes to its `report-dir`:

| File | Contents |
|---|---|
| `summary.txt` | Human-readable stats |
| `summary.csv` | Machine-readable stats |
| `fills.csv` | Per-fill records |
| `equity_curve.csv` | Equity + position at every LOB tick |

## Data

Place market data in `MD/`:
```
MD/lob.csv
MD/trades.csv
```

## Toolchain

- C++20, CMake, clang++ / g++
# backtest_framework
