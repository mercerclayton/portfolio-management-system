# Portfolio Management System (PMS-Lite)

This C++20 project provides a compact toolkit for portfolio accounting and valuation. It includes an instrument catalog, a market-data pub/sub feed with mid-price marking, and a FIFO position model that computes realized and unrealized PnL. The system uses only the C++ standard library and builds as a single command-line app with CMake.

## Features

* **Core Finance Types:** `Money`, `Price`, `Qty`, and `Side`, plus helpers for notional and unrealized PnL.
* **Instrument Repository:** Load instrument metadata (id, symbol, asset class, currency, tick size) from CSV into an in-memory store for fast lookup.
* **Market-Data Feed (pub/sub):** Push `Quote` updates (bid/ask/last) and subscribe via callbacks; cache the latest quote per instrument with a `mid()` helper.
* **FIFO Position Accounting:** Track open lots, realize PnL on closes, maintain VWAP average cost of the open side, and compute mark-to-market unrealized PnL.
* **Simple CLI Demo:** Runs end-to-end (instruments → quotes → trades → PnL) with clear console output.

## How to Use

**Installation:** Clone or download this repository.
**Dependencies:** C++20 compiler and CMake (no third-party libraries).

**Build (terminal):**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/pms               # Windows: .\build\Release\pms.exe
```

**Data:** The demo looks for `data/instruments.csv`. If it’s missing, it falls back to a small in-memory CSV so it still runs.

## Code Overview

* **`types.h`** – Core value types (`Money`, `Price`, `Qty`, `Side`) + helpers (`notional`, `unrealized_pnl`, `to_string`).
* **`instrument.h`** – `Instrument` and `InstrumentRepo` with CSV loading from a file path or any `std::istream`.
* **`marketdata.h`** – `Quote` (bid/ask/last), `mid(Quote)`, and `MarketDataFeed` (subscribe with callbacks, `last(id)` cache).
* **`position.h`** – `Position` with FIFO lots; `onFill(...)` for executed trades; `unrealized(mark)` for mark-to-market P\&L.

## Example Output

```
Instruments loaded from: data/instruments.csv
Number of instruments loaded: 3
AAPL: Equity USD (tick=0.01)

[QUOTE AAPL] bid=189.700000 ask=189.760000 last=189.730000 mid=189.730000 | notional @ 189.730000=USD 1897.30 | uPnL(long 10.000000 @ 100.000000)=USD 897.30
[QUOTE AAPL] bid=189.720000 ask=189.780000 last=189.750000 mid=189.750000 | notional @ 189.750000=USD 1897.50 | uPnL(long 10.000000 @ 100.000000)=USD 897.50
[QUOTE AAPL] bid=189.900000 ask=189.960000 last=189.940000 mid=189.930000 | notional @ 189.930000=USD 1899.30 | uPnL(long 10.000000 @ 100.000000)=USD 899.30
Last cached mid: 189.930000

---- POSITION AAPL ----
Currency : USD
Net Qty  : 5
Avg Px   : 110.000000
Realized : USD 250.00
Unrealized @189.930000 : USD 399.65

---- POSITION AAPL ----
Currency : USD
Net Qty  : 0
Avg Px   : 0.000000
Realized : USD 225.00
Unrealized @189.930000 : USD 0.00

---- POSITION AAPL ----
Currency : USD
Net Qty  : -8
Avg Px   : 119.000000
Realized : USD 225.00
Unrealized @189.930000 : USD -575.44

---- POSITION AAPL ----
Currency : USD
Net Qty  : -5
Avg Px   : 119.000000
Realized : USD 246.00
Unrealized @189.930000 : USD -359.65

Demo complete.
```

*(Exact numbers may differ slightly depending on the last mark and formatting.)*

## Roadmap

* Cash ledger (trade cashflows, fees) and portfolio aggregation (positions + cash → equity/NAV)
* Order/execution simulator and pre-trade risk checks
