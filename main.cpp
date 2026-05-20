#include <iostream>
#include <cstdlib>
#include "currency.h"
#include "exchange_rate.h"
#include "iso4217.h"

using namespace ratmoney;

int main() {
  Currency usd(100000, {1,  1}, iso4217::USD);
  Currency brl(100000, {1,  5}, iso4217::BRL);
  Currency eur(100000, {1,  2}, iso4217::EUR);
  Currency jpy( 10000, {1,  1}, iso4217::JPY);  // precision=0: no decimal places

  // basic operations
  auto r_add = usd.add(brl);
  auto r_sub = usd.subtract(brl);
  auto r_eur_add = eur.add(brl);
  auto r_eur_sub = eur.subtract(brl);

  if (!r_add || !r_sub || !r_eur_add || !r_eur_sub)
    return EXIT_FAILURE;

  std::cout << usd << " + " << brl << " = " << *r_add     << '\n';
  std::cout << usd << " - " << brl << " = " << *r_sub     << '\n';
  std::cout << eur << " + " << brl << " = " << *r_eur_add << '\n';
  std::cout << eur << " - " << brl << " = " << *r_eur_sub << '\n';
  std::cout << jpy << " (no decimal places)\n";

  // scale with different rounding modes
  Currency c(150, {1, 1}, iso4217::USD);
  auto half_even = c.scale({1, 100}, RoundingMode::HalfEven);
  auto half_up   = c.scale({1, 100}, RoundingMode::HalfUp);
  auto half_down = c.scale({1, 100}, RoundingMode::HalfDown);

  if (!half_even || !half_up || !half_down)
    return EXIT_FAILURE;

  std::cout << "\n150 × 0.01:\n";
  std::cout << "  HalfEven = " << half_even->units() << '\n';
  std::cout << "  HalfUp   = " << half_up->units()   << '\n';
  std::cout << "  HalfDown = " << half_down->units()  << '\n';

  // ratio
  auto ratio = usd.ratio(brl);
  if (!ratio) return EXIT_FAILURE;
  std::cout << "\nUSD / BRL = " << *ratio << '\n';

  // serialization round-trip
  auto s = usd.serialize();
  auto r = Currency::deserialize(s);
  if (!r) return EXIT_FAILURE;
  std::cout << "deserialize(" << usd << ") = " << *r << '\n';

  // exchange rate + convert
  ExchangeRate er{
    .bid       = {49, 10},   // 4.90 BRL/USD
    .ask       = {51, 10},   // 5.10 BRL/USD
    .fee       = {1, 100},   // 0.01 BRL/USD fee
    .timestamp = std::chrono::system_clock::now(),
    .validity  = std::chrono::seconds{300},
  };

  std::cout << "\nExchangeRate USD/BRL:\n";
  std::cout << "  bid=" << er.bid << "  ask=" << er.ask << "  fee=" << er.fee << '\n';
  std::cout << "  effectiveBid=" << er.effectiveBid()
            << "  effectiveAsk=" << er.effectiveAsk()
            << "  midpoint="     << er.midpoint()     << '\n';
  std::cout << "  valid: " << (er.isValid() ? "yes" : "no") << '\n';

  // convert via free function
  auto r_conv = convert(usd, er.effectiveAsk(), iso4217::BRL);
  if (!r_conv) return EXIT_FAILURE;
  std::cout << "\nconvert(" << usd << ", ask=" << er.effectiveAsk()
            << ", BRL) = " << *r_conv << '\n';

  // convert via ExchangeRate::apply
  auto r_apply = er.apply(usd, iso4217::BRL);
  if (!r_apply) return EXIT_FAILURE;
  std::cout << "er.apply (" << usd << ", BRL, Ask) = " << *r_apply << '\n';

  return EXIT_SUCCESS;
}
