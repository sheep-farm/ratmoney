#include <iomanip>
#include <limits>
#include <locale>
#include <ostream>
#include <stdexcept>
#include "currency.h"

namespace ratmoney {

// --- Rational ---

static __int128 gcd128(__int128 a, __int128 b) {
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  while (b) { __int128 t = b; b = a % b; a = t; }
  return a == 0 ? 1 : a;
}

Rational::Rational(int64_t n, int64_t d) {
  if (d == 0)
    throw std::invalid_argument("Rational: denominator cannot be zero");
  __int128 g  = gcd128((__int128)n, (__int128)d);
  __int128 rn = (__int128)n / g;
  __int128 rd = (__int128)d / g;
  // sign normalization via __int128 — avoids overflow when d == INT64_MIN
  if (rd < 0) { rn = -rn; rd = -rd; }
  if (rd > (__int128)std::numeric_limits<int64_t>::max())
    throw std::overflow_error("Rational: normalized denominator overflows int64_t");
  num = (int64_t)rn;
  den = (int64_t)rd;
}

// --- arithmetic helpers ---

static __int128 roundedDiv(__int128 n, __int128 d, RoundingMode mode) {
  __int128 q = n / d;
  __int128 r = n % d;

  if (r == 0) return q;

  bool positive = (n >= 0) == (d >= 0);
  __int128 abs_r  = r < 0 ? -r : r;
  __int128 abs_d  = d < 0 ? -d : d;
  __int128 abs_r2 = abs_r * 2;

  switch (mode) {
    case RoundingMode::HalfEven: {
      bool over = abs_r2 > abs_d;
      bool tie  = abs_r2 == abs_d && (q & 1) != 0;
      if (over || tie) q += positive ? 1 : -1;
      break;
    }
    case RoundingMode::HalfUp:
      if (abs_r2 >= abs_d) q += positive ? 1 : -1;
      break;
    case RoundingMode::HalfDown:
      if (abs_r2 > abs_d) q += positive ? 1 : -1;
      break;
    case RoundingMode::Up:
      q += positive ? 1 : -1;
      break;
    case RoundingMode::Down:
      break;
  }
  return q;
}

static __int128 convertUnits128(int64_t units, const Rational& from,
                                const Rational& to, RoundingMode mode) {
  __int128 n = (__int128)units * from.num * to.den;
  __int128 d = (__int128)from.den * to.num;
  return roundedDiv(n, d, mode);
}

static bool fitsInt64(__int128 v) {
  return v >= (__int128)std::numeric_limits<int64_t>::min()
      && v <= (__int128)std::numeric_limits<int64_t>::max();
}

// --- Currency::Impl ---

struct Currency::Impl {
  int64_t            units;
  Rational           rate;
  CurrencyDescription description;
};

// --- Currency ---

Currency::Currency(int64_t units, Rational rate, CurrencyDescription description)
  : impl_(std::make_unique<Impl>(Impl{units, rate, std::move(description)})) {}

Currency::~Currency() = default;

Currency::Currency(const Currency& other)
  : impl_(std::make_unique<Impl>(*other.impl_)) {}

Currency& Currency::operator=(const Currency& other) {
  if (this != &other)
    impl_ = std::make_unique<Impl>(*other.impl_);
  return *this;
}

Currency::Currency(Currency&&) noexcept = default;
Currency& Currency::operator=(Currency&&) noexcept = default;

int64_t                    Currency::units()       const noexcept { return impl_->units; }
Rational                   Currency::rate()        const noexcept { return impl_->rate; }
const CurrencyDescription& Currency::description() const noexcept { return impl_->description; }

bool Currency::operator==(const Currency& other) const {
  return impl_->units       == other.impl_->units
      && impl_->rate        == other.impl_->rate
      && impl_->description == other.impl_->description;
}

std::expected<Currency, CurrencyError> Currency::add(
    const Currency& other, RoundingMode mode) const {
  if (impl_->rate.num == 0)
    return std::unexpected(CurrencyError::ZeroRate);
  __int128 converted = convertUnits128(other.impl_->units, other.impl_->rate,
                                       impl_->rate, mode);
  __int128 result = (__int128)impl_->units + converted;
  if (!fitsInt64(result))
    return std::unexpected(CurrencyError::Overflow);
  return Currency((int64_t)result, impl_->rate, impl_->description);
}

std::expected<Currency, CurrencyError> Currency::subtract(
    const Currency& other, RoundingMode mode) const {
  if (impl_->rate.num == 0)
    return std::unexpected(CurrencyError::ZeroRate);
  __int128 converted = convertUnits128(other.impl_->units, other.impl_->rate,
                                       impl_->rate, mode);
  __int128 result = (__int128)impl_->units - converted;
  if (!fitsInt64(result))
    return std::unexpected(CurrencyError::Overflow);
  return Currency((int64_t)result, impl_->rate, impl_->description);
}

std::expected<Currency, CurrencyError> Currency::scale(
    Rational factor, RoundingMode mode) const {
  __int128 result = roundedDiv((__int128)impl_->units * factor.num,
                               (__int128)factor.den, mode);
  if (!fitsInt64(result))
    return std::unexpected(CurrencyError::Overflow);
  return Currency((int64_t)result, impl_->rate, impl_->description);
}

std::expected<Rational, CurrencyError> Currency::ratio(
    const Currency& other) const {
  if (impl_->rate.num == 0)
    return std::unexpected(CurrencyError::ZeroRate);
  __int128 other_in_this = convertUnits128(other.impl_->units, other.impl_->rate,
                                           impl_->rate, RoundingMode::HalfEven);
  if (other_in_this == 0)
    return std::unexpected(CurrencyError::DivisionByZero);
  if (!fitsInt64(other_in_this))
    return std::unexpected(CurrencyError::Overflow);
  return Rational(impl_->units, (int64_t)other_in_this);
}

Currency::Serialized Currency::serialize() const {
  return {
    impl_->units,
    impl_->rate.num,
    impl_->rate.den,
    impl_->description.name,
    impl_->description.symbol,
    impl_->description.precision,
  };
}

std::expected<Currency, CurrencyError> Currency::deserialize(const Serialized& s) {
  if (s.rate_den == 0 || s.precision > 18)
    return std::unexpected(CurrencyError::InvalidData);
  try {
    return Currency(s.units, Rational(s.rate_num, s.rate_den),
                    CurrencyDescription{s.name, s.symbol, s.precision});
  } catch (...) {
    return std::unexpected(CurrencyError::InvalidData);
  }
}

std::expected<Currency, CurrencyError> convert(
    const Currency&            from,
    Rational                   exchange_rate,
    const CurrencyDescription& to,
    RoundingMode               mode)
{
  if (exchange_rate.num == 0)
    return std::unexpected(CurrencyError::ZeroRate);

  uint8_t src_prec = from.description().precision;
  uint8_t tgt_prec = to.precision;

  __int128 src_factor = 1, tgt_factor = 1;
  for (uint8_t i = 0; i < src_prec; ++i) src_factor *= 10;
  for (uint8_t i = 0; i < tgt_prec; ++i) tgt_factor *= 10;

  // new_units = from.units * rate.num * tgt_factor / (rate.den * src_factor)
  __int128 n = (__int128)from.units() * exchange_rate.num * tgt_factor;
  __int128 d = (__int128)exchange_rate.den * src_factor;
  __int128 new_units = roundedDiv(n, d, mode);

  if (!fitsInt64(new_units))
    return std::unexpected(CurrencyError::Overflow);

  // target_rate = src_factor * rate.den / (rate.num * tgt_factor)
  // (value of 1 target-smallest-unit in source-smallest-units)
  __int128 tr_n = src_factor * (__int128)exchange_rate.den;
  __int128 tr_d = (__int128)exchange_rate.num * tgt_factor;
  __int128 g = gcd128(tr_n < 0 ? -tr_n : tr_n, tr_d < 0 ? -tr_d : tr_d);
  if (g == 0) g = 1;
  tr_n /= g; tr_d /= g;

  if (!fitsInt64(tr_n) || !fitsInt64(tr_d))
    return std::unexpected(CurrencyError::Overflow);

  try {
    return Currency((int64_t)new_units,
                    Rational((int64_t)tr_n, (int64_t)tr_d),
                    to);
  } catch (...) {
    return std::unexpected(CurrencyError::InvalidData);
  }
}

std::expected<Currency, CurrencyError> convert(
    const Currency&     from,
    const CurrencyPair& pair,
    Rational            exchange_rate,
    RoundingMode        mode)
{
  if (from.description() != pair.base)
    return std::unexpected(CurrencyError::InvalidData);
  return convert(from, exchange_rate, pair.quote, mode);
}

// --- stream operators ---

std::ostream& operator<<(std::ostream& os, const Rational& r) {
  if (r.den == 1) return os << r.num;
  return os << r.num << '/' << r.den;
}

std::ostream& operator<<(std::ostream& os, const Currency& c) {
  int64_t raw = c.units();
  bool negative = raw < 0;
  // use __int128 to negate safely — -INT64_MIN is UB in int64_t
  uint64_t abs_units = static_cast<uint64_t>(
    negative ? -static_cast<__int128>(raw) : static_cast<__int128>(raw));

  uint8_t  prec    = c.description().precision;
  uint64_t divisor = 1;
  for (uint8_t i = 0; i < prec; ++i) divisor *= 10;

  uint64_t whole = abs_units / divisor;
  uint64_t frac  = abs_units % divisor;

  char decimal_point =
    std::use_facet<std::numpunct<char>>(os.getloc()).decimal_point();

  // whole part via operator<< uses the stream's locale (includes thousands separator)
  os << c.description().symbol << (negative ? "-" : "") << whole;

  if (prec > 0) {
    // frac: zero-padded manually — must not have a thousands separator
    std::string frac_str(prec, '0');
    uint64_t tmp = frac;
    for (int i = prec - 1; i >= 0; --i) {
      frac_str[i] = '0' + (tmp % 10);
      tmp /= 10;
    }
    os << decimal_point << frac_str;
  }
  return os;
}

} // namespace ratmoney
