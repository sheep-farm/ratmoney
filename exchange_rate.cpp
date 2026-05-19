#include "exchange_rate.h"

namespace ratmoney {

namespace {

using i128 = __int128;

static i128 gcd128(i128 a, i128 b) {
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  while (b) { i128 t = b; b = a % b; a = t; }
  return a == 0 ? 1 : a;
}

static Rational rational_op(i128 n, i128 d) {
  i128 g = gcd128(n, d);
  return Rational((int64_t)(n / g), (int64_t)(d / g));
}

} // namespace

bool ExchangeRate::isValid() const noexcept {
  return std::chrono::system_clock::now() <= timestamp + validity;
}

Rational ExchangeRate::midpoint() const {
  i128 n = (i128)bid.num * ask.den + (i128)ask.num * bid.den;
  i128 d = (i128)bid.den * ask.den * 2;
  return rational_op(n, d);
}

Rational ExchangeRate::effectiveBid() const {
  i128 n = (i128)bid.num * fee.den - (i128)fee.num * bid.den;
  i128 d = (i128)bid.den * fee.den;
  return rational_op(n, d);
}

Rational ExchangeRate::effectiveAsk() const {
  i128 n = (i128)ask.num * fee.den + (i128)fee.num * ask.den;
  i128 d = (i128)ask.den * fee.den;
  return rational_op(n, d);
}

std::expected<Currency, CurrencyError> ExchangeRate::apply(
    const Currency& amount,
    const CurrencyDescription& target,
    Side side,
    RoundingMode mode) const
{
  Rational rate = (side == Side::Ask) ? effectiveAsk() : effectiveBid();
  return convert(amount, rate, target, mode);
}

} // namespace ratmoney
