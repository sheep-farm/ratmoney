#pragma once

#include <cstdint>
#include <expected>
#include <iosfwd>
#include <memory>
#include <string>

namespace ratmoney {

/// Exact rational number stored as normalized num/den in int64_t.
/// Throws std::invalid_argument if den == 0; std::overflow_error if normalization overflows.
struct Rational {
  int64_t num;
  int64_t den;
  Rational(int64_t num, int64_t den);
  bool operator==(const Rational&) const = default;
};

/// Identifies a currency denomination (name, symbol, decimal places).
/// precision must be <= 18; 0 = JPY, 2 = USD/EUR/BRL, 3 = KWD.
struct CurrencyDescription {
  std::string name;
  std::string symbol;
  uint8_t     precision = 2;
  bool operator==(const CurrencyDescription&) const = default;
};

enum class CurrencyError {
  ZeroRate,
  DivisionByZero,
  Overflow,
  InvalidData,
};

enum class RoundingMode {
  HalfEven,   ///< banker's rounding (default)
  HalfUp,     ///< half away from zero
  HalfDown,   ///< half toward zero
  Up,         ///< always away from zero
  Down,       ///< truncate toward zero
};

/// Which side of a spread to use when converting.
enum class Side { Bid, Ask };

/// Type-safe pair identifying a conversion direction.
/// Pass to the validated convert() overload to prevent mismatched exchange rates.
struct CurrencyPair {
  CurrencyDescription base;   ///< source currency
  CurrencyDescription quote;  ///< target currency
  bool operator==(const CurrencyPair&) const = default;
};

/// Monetary amount: integer units in smallest denomination + exchange rate + metadata.
/// All arithmetic uses __int128 intermediate values — no floating point, no silent overflow.
class Currency {
public:
  Currency(int64_t units, Rational rate, CurrencyDescription description);
  ~Currency();
  Currency(const Currency&);
  Currency& operator=(const Currency&);
  Currency(Currency&&) noexcept;
  Currency& operator=(Currency&&) noexcept;

  int64_t                    units()       const noexcept;
  Rational                   rate()        const noexcept;
  const CurrencyDescription& description() const noexcept;

  bool operator==(const Currency&) const;

  /// Returns this + other, converting other to this currency's rate.
  [[nodiscard]] std::expected<Currency, CurrencyError> add(
    const Currency& other,
    RoundingMode mode = RoundingMode::HalfEven) const;

  [[nodiscard]] std::expected<Currency, CurrencyError> subtract(
    const Currency& other,
    RoundingMode mode = RoundingMode::HalfEven) const;

  /// Scales units by factor; rate and description are preserved.
  [[nodiscard]] std::expected<Currency, CurrencyError> scale(
    Rational factor,
    RoundingMode mode = RoundingMode::HalfEven) const;

  /// Returns this/other as a dimensionless Rational (same rate basis).
  [[nodiscard]] std::expected<Rational, CurrencyError> ratio(
    const Currency& other) const;

  struct Serialized {
    int64_t     units;
    int64_t     rate_num;
    int64_t     rate_den;
    std::string name;
    std::string symbol;
    uint8_t     precision;
  };

  Serialized serialize() const;
  static std::expected<Currency, CurrencyError> deserialize(const Serialized&);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/// Convert from to a different currency.
/// exchange_rate: X target-major-units per 1 source-major-unit (e.g. {5,1} for 1 USD = 5 BRL).
/// Precision differences (USD=2, JPY=0) are handled automatically.
[[nodiscard]] std::expected<Currency, CurrencyError> convert(
    const Currency&            from,
    Rational                   exchange_rate,
    const CurrencyDescription& to,
    RoundingMode               mode = RoundingMode::HalfEven);

/// Validated convert: returns InvalidData if from.description() != pair.base.
[[nodiscard]] std::expected<Currency, CurrencyError> convert(
    const Currency&     from,
    const CurrencyPair& pair,
    Rational            exchange_rate,
    RoundingMode        mode = RoundingMode::HalfEven);

// Thread safety: each Currency owns its data exclusively (pImpl).
// Concurrent access to *different* objects is safe.
// Concurrent access to the *same* object without external synchronization is not.

std::ostream& operator<<(std::ostream&, const Rational&);
std::ostream& operator<<(std::ostream&, const Currency&);

} // namespace ratmoney
