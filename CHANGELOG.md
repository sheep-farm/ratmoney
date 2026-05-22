# Changelog

All notable changes to this project will be documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added

- `Currency::allocate(int64_t n)` — splits amount into n equal parts using floor division;
  parts differ by at most 1 unit and sum exactly to the original (correct for negative amounts)
- `Currency::allocate(span<const int64_t> ratios)` / `allocate({...})` — splits by integer
  ratios using the largest-remainder method; sum always equals the original regardless of
  rounding; all ratios must be ≥ 0 and not all zero
- `Currency::percent(Rational pct, RoundingMode)` — returns `amount × pct / 100`;
  e.g. `percent({15, 1})` = 15%, `percent({1, 2})` = 0.5%
- `Currency::proportion(Rational factor, RoundingMode)` — semantic alias for `scale()`;
  makes intent explicit when computing a fraction of a whole
- `Currency::to_double()` — returns the amount in major units as `double`
  (e.g. 12345 USD-cents → 123.45); intended for continuous math, not display or storage

### Changed

- `iso4217.h` — expanded from ~40 to ~180 entries with full ISO 4217 active-code coverage:
  Americas (North, Central/Caribbean, South), Europe (incl. UK territories FKP/GIP/SHP),
  Middle East, Africa, Asia, Pacific/Oceania, precious metals (XAU/XAG/XPD/XPT),
  fund codes (BOV/CHE/CHW/CLF/COU/MXV/USN/UYI/UYW/XDR/XSU/XUA), and
  non-currency codes (XTS/XXX). Precision for all 3-decimal currencies (BHD/IQD/JOD/KWD/LYD/OMR/TND),
  0-decimal currencies (BIF/CLP/DJF/GNF/ISK/JPY/KMF/KRW/PYG/RWF/UGX/VND/VUV/XAF/XOF/XPF),
  and 4-decimal fund codes (CLF/UYW) are correctly set.

---

## [0.1.0] — 2026-05-20

Initial public release.

### Added

- `Rational` — exact rational number as normalized `int64_t num/den` (GCD at construction)
- `Currency` — monetary amount as `int64_t` minor units + `Rational` exchange rate + `CurrencyDescription`
- `CurrencyDescription` — name, ISO symbol, decimal precision (0–18)
- `CurrencyError` — `ZeroRate`, `DivisionByZero`, `Overflow`, `InvalidData`
- `RoundingMode` — `HalfEven` (banker's, default), `HalfUp`, `HalfDown`, `Up`, `Down`
- `Currency::add`, `subtract`, `scale`, `ratio` — all return `std::expected<T, CurrencyError>`
- `Currency::serialize` / `Currency::deserialize` — plain-struct round-trip
- `CurrencyPair` — type-safe conversion direction; validated `convert()` overload
- Free `convert(from, exchange_rate, to)` — handles precision differences (USD↔JPY etc.)
- `ExchangeRate` — bid/ask/fee with `effectiveBid()`, `effectiveAsk()`, `midpoint()`, `isValid()`, `apply()`
- `Side` — `Bid` / `Ask` selector for `ExchangeRate::apply`
- `operator<<` for `Rational` and `Currency` — locale-aware decimal point and thousands separator
- ISO 4217 registry (`ratmoney::iso4217`) — 40+ currency constants across Americas, Europe, Asia/Pacific, Middle East/Africa, and informal crypto (BTC, ETH)
- pImpl (`std::unique_ptr<Impl>`) for ABI stability
- All intermediate arithmetic uses `__int128` — no floating point, no silent overflow
- CMake build: static (`RatMoney::Static`) and shared (`RatMoney::Shared`) targets with `find_package` support
- GNU Make build: `make`, `make static`, `make shared`, `make test`, `make test-sanitize`, `make install`
- CI: GCC 13/14 and Clang 18, Release + ASan/UBSan, libFuzzer (60 s), on ubuntu-24.04
- libFuzzer entry point (`fuzz_target.cpp`)
- 80+ Google Test cases across 10+ suites including stress test (50 000 random inputs)

### Platform

- Requires GCC 13+ or Clang 16+ (C++23, `__int128`)
- Linux x86\_64 and ARM64; macOS with GCC or Clang
- MSVC is not supported (`__int128` is unavailable)
