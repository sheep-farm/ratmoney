// Entry point for libFuzzer. Build with: CXX=clang++ make fuzz
// Run: ./fuzz_target corpus/ -max_total_time=60
#include <cstring>
#include <stdexcept>
#include "currency.h"
#include "exchange_rate.h"
#include "iso4217.h"

using namespace ratmoney;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 50) return 0;

  int64_t u1, rn1, rd1, u2, rn2, rd2;
  std::memcpy(&u1,  data,      8);
  std::memcpy(&rn1, data + 8,  8);
  std::memcpy(&rd1, data + 16, 8);
  std::memcpy(&u2,  data + 24, 8);
  std::memcpy(&rn2, data + 32, 8);
  std::memcpy(&rd2, data + 40, 8);

  auto mode = static_cast<RoundingMode>(data[48] % 5);
  int64_t n = (int64_t)(data[49] % 9) + 1;  // 1..9

  // ratios for allocate: clamp to [0, 200] to keep sums well within int64
  int64_t ra = (int64_t)(data[48] % 100);
  int64_t rb = (int64_t)(data[49] % 100);
  int64_t rc = (int64_t)(data[48] % 50) + 1;

  try {
    Rational r1(rn1, rd1 == 0 ? 1 : rd1);
    Rational r2(rn2, rd2 == 0 ? 1 : rd2);

    Currency c1(u1, r1, iso4217::USD);
    Currency c2(u2, r1, iso4217::BRL);

    // ── core arithmetic ───────────────────────────────────────
    (void)c1.add(c2, mode);
    (void)c1.subtract(c2, mode);
    (void)c1.scale(r1, mode);
    (void)c1.ratio(c2);

    // ── percent / proportion / to_double ──────────────────────
    (void)c1.percent(r1, mode);
    (void)c1.proportion(r1, mode);
    (void)c1.to_double();

    // ── allocate — equal split ────────────────────────────────
    (void)c1.allocate(n);

    // ── allocate — ratio split ────────────────────────────────
    (void)c1.allocate({ra, rb, rc});

    // ── serialize ─────────────────────────────────────────────
    auto s = c1.serialize();
    (void)Currency::deserialize(s);

    // ── convert ───────────────────────────────────────────────
    if (rn1 != 0)
      (void)convert(c1, r1, iso4217::BRL, mode);

    // ── ExchangeRate ──────────────────────────────────────────
    ExchangeRate er{
      .bid       = r1,
      .ask       = r2.num != 0 ? r2 : Rational{1, 1},
      .fee       = {0, 1},
      .timestamp = std::chrono::system_clock::now(),
    };
    (void)er.midpoint();
    (void)er.effectiveBid();
    (void)er.effectiveAsk();
    if (er.effectiveAsk().num != 0)
      (void)er.apply(c1, iso4217::BRL, Side::Ask, mode);

  } catch (const std::invalid_argument&) {
  } catch (const std::overflow_error&) {
  }

  return 0;
}
