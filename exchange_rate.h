#pragma once

#include <chrono>
#include "currency.h"

namespace ratmoney {

struct ExchangeRate {
  Rational  bid;
  Rational  ask;
  Rational  fee{0, 1};
  std::chrono::system_clock::time_point timestamp;
  std::chrono::seconds validity{60};

  bool     isValid()      const noexcept;
  Rational midpoint()     const;
  Rational effectiveBid() const;  ///< bid - fee
  Rational effectiveAsk() const;  ///< ask + fee

  /// Convert amount to target currency using effectiveBid or effectiveAsk.
  [[nodiscard]] std::expected<Currency, CurrencyError> apply(
      const Currency&            amount,
      const CurrencyDescription& target,
      Side                       side = Side::Ask,
      RoundingMode               mode = RoundingMode::HalfEven) const;
};

} // namespace ratmoney
