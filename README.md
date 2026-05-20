# ratmoney

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)](#requirements)

> C++23 library for **exact** monetary arithmetic — no floating point, no silent overflow.

---

## The problem

```cpp
double price  = 0.10;
double tax    = 0.20;
double result = price + tax;          // 0.30000000000000004
```

Floating-point arithmetic is not suitable for money. Rounding errors accumulate silently across sums, tax calculations, currency conversions, and exchange-rate applications. The damage is invisible until reconciliation fails.

**ratmoney** solves this by storing amounts as `int64_t` integer minor units, performing all intermediate arithmetic in `__int128`, and propagating every possible failure through `std::expected<T, CurrencyError>` — so errors are explicit, not silent.

---

## How it works

A `Currency` holds three fields:

| Field | Type | Meaning |
|---|---|---|
| `units` | `int64_t` | Amount in smallest denomination (cents for USD, pence for GBP, satoshis for BTC) |
| `rate` | `Rational{num, den}` | Exchange rate relative to a base: lets `add()` reconcile across currencies without a separate conversion step |
| `description` | `CurrencyDescription` | Name, ISO symbol, decimal precision |

`Rational` stores `num`/`den` as normalized `int64_t` via GCD. All cross-currency multiplications use `__int128` intermediates, then round back to `int64_t` via one of five deterministic rounding modes.

---

## Features

| Feature | Detail |
|---|---|
| Exact arithmetic | `int64_t` units + `Rational{num,den}` rates, `__int128` intermediates — zero floating point |
| Overflow-safe | Every operation returns `std::expected<T, CurrencyError>`, never throws |
| 5 rounding modes | `HalfEven` (banker's, default), `HalfUp`, `HalfDown`, `Up`, `Down` |
| Precision-aware | `precision` per description: JPY=0, USD/EUR/BRL=2, KWD=3 |
| Exchange rates | `ExchangeRate` with bid/ask/fee, `effectiveBid/Ask()`, `apply()` |
| Type-safe convert | `CurrencyPair` validates source currency at call site |
| ISO 4217 registry | 40+ currencies in `ratmoney::iso4217` |
| Serialization | `serialize()` / `deserialize()` round-trip |
| ABI stable | pImpl (`std::unique_ptr<Impl>`) — changing internals doesn't break consumers |
| Locale output | `operator<<` uses stream locale for decimal point and thousands separator |
| Fuzzing | libFuzzer entry point in `fuzz_target.cpp` |

---

## Quick start

```cpp
#include <ratmoney/currency.h>
#include <ratmoney/exchange_rate.h>
#include <ratmoney/iso4217.h>

using namespace ratmoney;

// ── Construction ─────────────────────────────────────────────────
// units are in smallest denomination; rate {1,1} means 1:1 with itself
Currency usd(10000, {1, 1}, iso4217::USD);   // 100.00 USD
Currency brl(50000, {1, 5}, iso4217::BRL);   // 500.00 BRL  (rate: 1 USD = 5 BRL)
Currency jpy( 1100, {1, 1}, iso4217::JPY);   // 1100 JPY    (precision=0, no decimals)

// ── Arithmetic ────────────────────────────────────────────────────
// All operations return std::expected — never throw
auto sum = usd.add(brl);
if (sum) std::cout << *sum << '\n';          // USD120.00

auto tax = usd.scale({1, 10});               // 10 % of 100 USD → USD10.00

// ── Raw conversion ────────────────────────────────────────────────
// {5, 1}: 1 USD = 5 BRL (major-unit ratio)
auto in_brl = convert(usd, {5, 1}, iso4217::BRL);

// ── ExchangeRate (bid / ask / fee) ────────────────────────────────
ExchangeRate er{
    .bid       = {49, 10},                        // 4.90
    .ask       = {51, 10},                        // 5.10
    .fee       = { 1, 100},                       // 0.01 per major unit
    .timestamp = std::chrono::system_clock::now(),
    .validity  = std::chrono::seconds{60},
};
auto result = er.apply(usd, iso4217::BRL);        // effectiveAsk() = 5.11 → BRL511.00

// ── Type-safe pair ────────────────────────────────────────────────
// Returns CurrencyError::InvalidData if usd.description() != pair.base
CurrencyPair usd_brl{iso4217::USD, iso4217::BRL};
auto safe = convert(usd, usd_brl, {5, 1});

// ── Ratio ─────────────────────────────────────────────────────────
Currency a(200, {1, 1}, iso4217::USD);  // 2.00 USD
Currency b(100, {1, 1}, iso4217::USD);  // 1.00 USD
auto r = a.ratio(b);                    // Rational{2, 1}

// ── Serialize / deserialize ───────────────────────────────────────
auto s       = usd.serialize();
auto restored = Currency::deserialize(s);

// ── Error handling ────────────────────────────────────────────────
auto bad = convert(usd, {0, 1}, iso4217::BRL);   // ZeroRate
if (!bad) {
    switch (bad.error()) {
        case CurrencyError::ZeroRate:      /* ... */ break;
        case CurrencyError::Overflow:      /* ... */ break;
        case CurrencyError::InvalidData:   /* ... */ break;
        case CurrencyError::DivisionByZero:/* ... */ break;
    }
}
```

---

## Build

### Requirements

| Dependency | Version | Notes |
|---|---|---|
| GCC | 13+ | C++23, `__int128` |
| Clang | 16+ | required only for fuzzing |
| CMake | 3.20+ | recommended build system |
| GNU Make | any | alternative build system |
| Google Test | any | `libgtest-dev` on Debian/Ubuntu |

> **Platform note:** `__int128` is a GCC/Clang extension. Supported on Linux x86\_64, ARM64, and macOS. MSVC is not supported.

---

### Building with CMake (recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Output in `build/`:

| File | Type |
|---|---|
| `libratmoney.a` | Static library |
| `libratmoney.so` | Shared library (+ versioned symlinks) |

#### CMake options

| Option | Default | Effect |
|---|---|---|
| `SANITIZE` | OFF | Enables AddressSanitizer + UBSan on all targets |
| `FUZZ` | OFF | Builds `fuzz_target` with libFuzzer (requires clang) |

---

### Building with GNU Make

```bash
make            # static + shared
make static     # libratmoney_currency.a only
make shared     # libratmoney_currency.so only
```

---

### Installing system-wide

#### Via CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
sudo cmake --install build                           # → /usr/local
sudo cmake --install build --prefix /opt/ratmoney    # or any custom prefix
```

#### Via GNU Make

```bash
sudo make install                        # → /usr/local
sudo make install PREFIX=/opt/ratmoney   # or any custom prefix
sudo make uninstall
```

Installed layout:

| Path | Contents |
|---|---|
| `<prefix>/include/ratmoney/` | `currency.h`, `exchange_rate.h`, `iso4217.h` |
| `<prefix>/lib/` | `libratmoney.a`, `libratmoney.so` (+ versioned symlinks) |
| `<prefix>/lib/cmake/RatMoney/` | CMake package config (`find_package` support) |

---

### Using in your project

#### CMake — via `find_package`

```cmake
find_package(RatMoney REQUIRED)

target_link_libraries(myapp PRIVATE RatMoney::Static)
# or
target_link_libraries(myapp PRIVATE RatMoney::Shared)
```

If installed to a custom prefix:

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/opt/ratmoney
```

#### Manual — static

```bash
g++ -std=c++23 myapp.cpp \
    -I/usr/local/include/ratmoney \
    /usr/local/lib/libratmoney.a \
    -o myapp
```

#### Manual — shared

```bash
g++ -std=c++23 myapp.cpp \
    -I/usr/local/include/ratmoney \
    -L/usr/local/lib -lratmoney \
    -o myapp
```

---

### Tests, sanitizers, and fuzzing

```bash
# CMake
cmake -B build && cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure

# CMake + ASan/UBSan
cmake -B build -DSANITIZE=ON && cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure

# GNU Make shortcuts
make test            # build + run all tests
make test-sanitize   # build + run with ASan/UBSan
make run             # build + run demo

# Fuzzing (clang required)
CXX=clang++ cmake -B build -DFUZZ=ON && cmake --build build -j$(nproc)
mkdir -p corpus
./build/fuzz_target corpus/ -max_total_time=60
```

---

## API reference

### `Rational`

```cpp
struct Rational { int64_t num; int64_t den; };
```

Normalized by GCD at construction. `den == 0` throws `std::invalid_argument`.

---

### `CurrencyDescription`

```cpp
struct CurrencyDescription { std::string name; std::string symbol; uint8_t precision; };
```

Pre-defined constants in `ratmoney::iso4217` (40+ currencies). `precision` is the number of decimal places: 0 for JPY, 2 for USD/EUR/BRL, 3 for KWD.

---

### `Currency`

```cpp
Currency(int64_t units, Rational rate, CurrencyDescription description);
```

Key methods — all return `std::expected<T, CurrencyError>`:

| Method | Returns | Notes |
|---|---|---|
| `add(other, mode)` | `Currency` | Converts `other` to `this` rate basis |
| `subtract(other, mode)` | `Currency` | Same rate reconciliation |
| `scale(factor, mode)` | `Currency` | Multiplies units by `factor`; rate/description preserved |
| `ratio(other)` | `Rational` | Dimensionless ratio on same rate basis |
| `serialize()` | `Serialized` | Plain-old-data struct for storage/transport |
| `Currency::deserialize(s)` | `Currency` | Validates on reconstruction |

---

### `ExchangeRate`

```cpp
struct ExchangeRate {
    Rational  bid;
    Rational  ask;
    Rational  fee{0, 1};
    std::chrono::system_clock::time_point timestamp;
    std::chrono::seconds validity{60};
};
```

| Method | Notes |
|---|---|
| `isValid()` | Returns false if `now > timestamp + validity` |
| `midpoint()` | `(bid + ask) / 2` |
| `effectiveBid()` | `bid - fee` |
| `effectiveAsk()` | `ask + fee` |
| `apply(amount, target, side, mode)` | Converts using bid or ask (default: ask) |

---

### `CurrencyError`

| Value | Cause |
|---|---|
| `ZeroRate` | Source or conversion rate has zero numerator |
| `DivisionByZero` | `ratio()` divisor is zero |
| `Overflow` | Result doesn't fit in `int64_t` |
| `InvalidData` | Bad serialized data, `precision > 18`, or wrong `CurrencyPair` source |

---

### `RoundingMode`

| Value | Behaviour |
|---|---|
| `HalfEven` | Banker's rounding — round to even on exact half (default) |
| `HalfUp` | Round half away from zero |
| `HalfDown` | Round half toward zero |
| `Up` | Always away from zero |
| `Down` | Truncate toward zero |

---

## Thread safety

Each `Currency` owns its data exclusively via pImpl. Concurrent operations on **different** objects are safe without synchronization. Concurrent access to the **same** object requires external synchronization.

---

## Source layout

| Path | Purpose |
|---|---|
| `currency.h / .cpp` | Core types: `Rational`, `Currency`, `CurrencyPair`, `convert` |
| `exchange_rate.h / .cpp` | `ExchangeRate` with bid/ask/fee/apply |
| `iso4217.h` | 40+ `CurrencyDescription` constants |
| `fuzz_target.cpp` | libFuzzer entry point |
| `test.cpp` | 80+ Google Test cases across 10+ suites |
| `main.cpp` | Demo / manual smoke test |
| `CMakeLists.txt` | CMake build (options: `SANITIZE`, `FUZZ`) |
| `cmake/RatMoneyConfig.cmake.in` | Package config template for `find_package` |
| `docs/DESIGN.md` | Architecture decisions and design rationale |
| `docs/FOUNDATIONS.md` | Mathematical foundations |

---

## License

[GNU General Public License v3.0](LICENSE)
