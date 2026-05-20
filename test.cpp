#include <chrono>
#include <limits>
#include <random>
#include <sstream>
#include <gtest/gtest.h>
#include "currency.h"
#include "exchange_rate.h"
#include "iso4217.h"

using namespace ratmoney;

// ────────────────────────────────────────────────────────────
// Main fixture
// ────────────────────────────────────────────────────────────

class CurrencyTest : public ::testing::Test {
protected:
  Currency dollar    { 100000, {1,  1}, {"US Dollar",      "USD"} };
  Currency brl       { 100000, {1,  5}, {"Brazilian Real",  "BRL"} };
  Currency uyu       { 100000, {1, 25}, {"Uruguayan Peso",  "UYU"} };
  Currency eur       { 100000, {1,  2}, {"Euro",            "EUR"} };
  Currency zero_rate { 100000, {0,  1}, {"Zero Rate",       "ZRO"} };
  Currency zero_value{      0, {1,  2}, {"Zero Value",      "ZVL"} };
};

// ────────────────────────────────────────────────────────────
// add
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, AddDollarPlusBrl) {
  auto r = dollar.add(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 120000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, AddBrlPlusDollar) {
  auto r = brl.add(dollar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 600000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, AddDollarPlusUyu) {
  auto r = dollar.add(uyu);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 104000);
}

TEST_F(CurrencyTest, AddBrlPlusUyu) {
  auto r = brl.add(uyu);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 120000);
}

TEST_F(CurrencyTest, AddEurPlusBrl) {
  auto r = eur.add(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 140000);
  EXPECT_EQ(r->rate(), (Rational{1, 2}));
}

TEST_F(CurrencyTest, AddBrlPlusEur) {
  auto r = brl.add(eur);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 350000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, AddZeroRateReturnsError) {
  auto r = zero_rate.add(dollar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, AddZeroValue) {
  auto r = dollar.add(zero_value);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 100000);
}

TEST_F(CurrencyTest, AddOverflowReturnsError) {
  Currency max_val(std::numeric_limits<int64_t>::max(), {1, 1}, {"Max", "MX"});
  auto r = max_val.add(dollar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// subtract
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, SubtractDollarMinusBrl) {
  auto r = dollar.subtract(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 80000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, SubtractBrlMinusDollar) {
  auto r = brl.subtract(dollar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), -400000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, SubtractEurMinusBrl) {
  auto r = eur.subtract(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 60000);
  EXPECT_EQ(r->rate(), (Rational{1, 2}));
}

TEST_F(CurrencyTest, SubtractBrlMinusEur) {
  auto r = brl.subtract(eur);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), -150000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, SubtractZeroRateReturnsError) {
  auto r = zero_rate.subtract(dollar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, SubtractOverflowReturnsError) {
  Currency min_val(std::numeric_limits<int64_t>::min(), {1, 1}, {"Min", "MN"});
  auto r = min_val.subtract(dollar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// scale
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, ScaleBy2) {
  auto r = dollar.scale({2, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 200000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, ScaleByHalf) {
  auto r = dollar.scale({1, 2});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 50000);
}

TEST_F(CurrencyTest, ScaleBy3Halves) {
  auto r = dollar.scale({3, 2});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 150000);
}

TEST_F(CurrencyTest, ScaleByZero) {
  auto r = dollar.scale({0, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 0);
}

TEST_F(CurrencyTest, ScaleInheritsRate) {
  auto r = brl.scale({2, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, ScaleOverflowReturnsError) {
  Currency large(std::numeric_limits<int64_t>::max(), {1, 1}, {"Max", "MX"});
  auto r = large.scale({2, 1});
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// ratio
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, RatioDollarBrlIs5To1) {
  auto r = dollar.ratio(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{5, 1}));
}

TEST_F(CurrencyTest, RatioBrlDollarIs1To5) {
  auto r = brl.ratio(dollar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{1, 5}));
}

TEST_F(CurrencyTest, RatioEurBrlIs5To2) {
  auto r = eur.ratio(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{5, 2}));
}

TEST_F(CurrencyTest, RatioBrlEurIs2To5) {
  auto r = brl.ratio(eur);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{2, 5}));
}

TEST_F(CurrencyTest, RatioZeroRateReturnsError) {
  auto r = zero_rate.ratio(dollar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, RatioZeroDivisorReturnsError) {
  auto r = dollar.ratio(zero_value);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::DivisionByZero);
}

// ────────────────────────────────────────────────────────────
// description
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, AddInheritsDescriptionFromLhs) {
  auto r = dollar.add(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), dollar.description());
  EXPECT_EQ(r->description().name,   "US Dollar");
  EXPECT_EQ(r->description().symbol, "USD");
}

TEST_F(CurrencyTest, SubtractInheritsDescriptionFromLhs) {
  auto r = eur.subtract(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), eur.description());
  EXPECT_EQ(r->description().name,   "Euro");
  EXPECT_EQ(r->description().symbol, "EUR");
}

TEST_F(CurrencyTest, ScaleInheritsDescriptionFromOperand) {
  auto r = brl.scale({3, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), brl.description());
  EXPECT_EQ(r->description().name,   "Brazilian Real");
  EXPECT_EQ(r->description().symbol, "BRL");
}

TEST_F(CurrencyTest, AddDoesNotInheritDescriptionFromRhs) {
  auto r = dollar.add(brl);
  ASSERT_TRUE(r.has_value());
  EXPECT_NE(r->description(), brl.description());
}

// ────────────────────────────────────────────────────────────
// operator==
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, EqualitySameValues) {
  Currency other(100000, {1, 1}, {"US Dollar", "USD"});
  EXPECT_EQ(dollar, other);
}

TEST_F(CurrencyTest, InequalityDifferentUnits) {
  Currency other(200000, {1, 1}, {"US Dollar", "USD"});
  EXPECT_NE(dollar, other);
}

TEST_F(CurrencyTest, InequalityDifferentRate) {
  Currency other(100000, {1, 2}, {"US Dollar", "USD"});
  EXPECT_NE(dollar, other);
}

TEST_F(CurrencyTest, InequalityDifferentDescription) {
  Currency other(100000, {1, 1}, {"Euro", "EUR"});
  EXPECT_NE(dollar, other);
}

// ────────────────────────────────────────────────────────────
// invariants
// ────────────────────────────────────────────────────────────

TEST(InvariantTest, RationalZeroDenominatorThrows) {
  EXPECT_THROW((Rational{1, 0}), std::invalid_argument);
}

TEST(InvariantTest, RationalInt64MinDenominatorThrowsOverflow) {
  EXPECT_THROW((Rational{1, std::numeric_limits<int64_t>::min()}),
               std::overflow_error);
}

TEST(InvariantTest, RationalNormalizesFraction) {
  EXPECT_EQ((Rational{2, 10}), (Rational{1, 5}));
  EXPECT_EQ((Rational{6,  9}), (Rational{2, 3}));
}

TEST(InvariantTest, RationalNormalizesSign) {
  EXPECT_EQ((Rational{ 1, -2}), (Rational{-1, 2}));
  EXPECT_EQ((Rational{-1, -2}), (Rational{ 1, 2}));
}

TEST(InvariantTest, DescriptionStoredByValue) {
  CurrencyDescription desc { "Test", "T$" };
  Currency c(0, {1, 1}, desc);
  desc.name = "Modified";
  EXPECT_EQ(c.description().name, "Test");
}

// ────────────────────────────────────────────────────────────
// operator<<
// ────────────────────────────────────────────────────────────

TEST(StreamTest, Int64MinDoesNotCrash) {
  // Before the fix, -INT64_MIN was UB; now uses __int128
  Currency c(std::numeric_limits<int64_t>::min(), {1, 1}, {"T", "T$", 2});
  std::ostringstream oss;
  EXPECT_NO_THROW(oss << c);
  EXPECT_NE(oss.str().find("T$-"), std::string::npos);
}

TEST(StreamTest, LocaleThousandsSeparator) {
  // Imbue with pt_BR or de_DE locale to test thousands separator
  Currency c(100000, {1, 1}, {"T", "T$", 2});
  std::ostringstream oss;
  try {
    oss.imbue(std::locale("pt_BR.UTF-8"));
  } catch (...) {
    GTEST_SKIP() << "locale pt_BR.UTF-8 not available";
  }
  oss << c;
  // With pt_BR locale, 1000.00 may have a thousands separator (implementation-defined)
  // We only verify that it did not crash and that there is output
  EXPECT_FALSE(oss.str().empty());
}

TEST(StreamTest, JpyNoDecimalPlaces) {
  Currency jpy(1500, {1, 1}, ratmoney::iso4217::JPY);
  std::ostringstream oss;
  oss << jpy;
  EXPECT_EQ(oss.str(), "JPY1500");
}

TEST(StreamTest, KwdThreeDecimalPlaces) {
  Currency kwd(1001, {1, 1}, ratmoney::iso4217::KWD);
  std::ostringstream oss;
  oss << kwd;
  EXPECT_EQ(oss.str(), "KWD1.001");
}

// ────────────────────────────────────────────────────────────
// banker's rounding (HalfEven default)
// ────────────────────────────────────────────────────────────

TEST(RoundingTest, HalfBelowRoundsDown) {
  Currency c(50, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 0);
}

TEST(RoundingTest, HalfAboveRoundsUp) {
  Currency c(150, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingTest, HalfOddRoundsToEven) {
  Currency c(250, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

// ────────────────────────────────────────────────────────────
// rounding modes
// ────────────────────────────────────────────────────────────

TEST(RoundingModeTest, HalfUpRoundsHalfUp) {
  Currency c(150, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::HalfUp);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingModeTest, HalfDownRoundsHalfDown) {
  Currency c(150, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::HalfDown);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 1);
}

TEST(RoundingModeTest, UpAlwaysRoundsUp) {
  Currency c(101, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::Up);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingModeTest, DownAlwaysTruncates) {
  Currency c(199, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::Down);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 1);
}

TEST(RoundingModeTest, HalfEvenRoundsToNearestEven) {
  Currency c50 (50,  {1, 1}, {"T", "T$"});
  Currency c150(150, {1, 1}, {"T", "T$"});

  auto r50  = c50 .scale({1, 100}, RoundingMode::HalfEven);
  auto r150 = c150.scale({1, 100}, RoundingMode::HalfEven);

  ASSERT_TRUE(r50.has_value());
  ASSERT_TRUE(r150.has_value());
  EXPECT_EQ(r50->units(),  0);  // 0.5 → 0 (even)
  EXPECT_EQ(r150->units(), 2);  // 1.5 → 2 (even)
}

// ────────────────────────────────────────────────────────────
// serialization
// ────────────────────────────────────────────────────────────

TEST(SerializationTest, RoundTripBasic) {
  Currency original(123456, {3, 7}, {"Test", "T$", 2});
  auto s = original.serialize();
  auto r = Currency::deserialize(s);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, original);
}

TEST(SerializationTest, RoundTripPrecisionThree) {
  Currency original(999, {1, 1}, {"Kuwaiti Dinar", "KWD", 3});
  auto s = original.serialize();
  auto r = Currency::deserialize(s);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description().precision, 3);
  EXPECT_EQ(*r, original);
}

TEST(SerializationTest, DeserializeZeroDenominatorReturnsError) {
  Currency::Serialized bad{0, 1, 0, "T", "T$", 2};
  auto r = Currency::deserialize(bad);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::InvalidData);
}

TEST(SerializationTest, DeserializePrecisionAboveMaxReturnsError) {
  Currency::Serialized bad{0, 1, 1, "T", "T$", 19};
  auto r = Currency::deserialize(bad);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::InvalidData);
}

TEST(SerializationTest, SerializedPreservesFields) {
  Currency c(42, {5, 3}, {"My Coin", "MC", 1});
  auto s = c.serialize();
  EXPECT_EQ(s.units,     42);
  EXPECT_EQ(s.rate_num,   5);
  EXPECT_EQ(s.rate_den,   3);
  EXPECT_EQ(s.name,      "My Coin");
  EXPECT_EQ(s.symbol,    "MC");
  EXPECT_EQ(s.precision,  1);
}

// ────────────────────────────────────────────────────────────
// ISO 4217
// ────────────────────────────────────────────────────────────

TEST(Iso4217Test, JpyPrecisionZero) {
  EXPECT_EQ(iso4217::JPY.precision, 0);
  EXPECT_EQ(iso4217::JPY.symbol, "JPY");
}

TEST(Iso4217Test, KwdPrecisionThree) {
  EXPECT_EQ(iso4217::KWD.precision, 3);
  EXPECT_EQ(iso4217::KWD.symbol, "KWD");
}

TEST(Iso4217Test, UsdPrecisionTwo) {
  EXPECT_EQ(iso4217::USD.precision, 2);
  EXPECT_EQ(iso4217::USD.symbol, "USD");
}

TEST(Iso4217Test, CurrencyWithJpyAcceptsPrecisionZero) {
  Currency jpy(1500, {1, 1}, iso4217::JPY);
  EXPECT_EQ(jpy.description().precision, 0);
  EXPECT_EQ(jpy.units(), 1500);
}

// ────────────────────────────────────────────────────────────
// ExchangeRate
// ────────────────────────────────────────────────────────────

TEST(ExchangeRateTest, RateValidWithinTimeWindow) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now(),
    .validity  = std::chrono::seconds{60},
  };
  EXPECT_TRUE(er.isValid());
}

TEST(ExchangeRateTest, RateExpiredOutsideTimeWindow) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now() - std::chrono::seconds{120},
    .validity  = std::chrono::seconds{60},
  };
  EXPECT_FALSE(er.isValid());
}

TEST(ExchangeRateTest, MidpointSymmetricBidAsk) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now(),
  };
  // (99/100 + 101/100) / 2 = 200/200 = 1/1
  EXPECT_EQ(er.midpoint(), (Rational{1, 1}));
}

TEST(ExchangeRateTest, EffectiveBidAskWithFee) {
  ExchangeRate er{
    .bid       = {100, 100},  // 1.00
    .ask       = {102, 100},  // 1.02
    .fee       = {1,   100},  // 0.01
    .timestamp = std::chrono::system_clock::now(),
  };
  // bid - fee = 1.00 - 0.01 = 99/100
  EXPECT_EQ(er.effectiveBid(), (Rational{99, 100}));
  // ask + fee = 1.02 + 0.01 = 103/100
  EXPECT_EQ(er.effectiveAsk(), (Rational{103, 100}));
}

TEST(ExchangeRateTest, EffectiveBidAskNoFeeEqualsBidAsk) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .fee       = {0, 1},
    .timestamp = std::chrono::system_clock::now(),
  };
  EXPECT_EQ(er.effectiveBid(), er.bid);
  EXPECT_EQ(er.effectiveAsk(), er.ask);
}

TEST(ExchangeRateTest, MidpointFractional) {
  ExchangeRate er{
    .bid       = {1, 2},
    .ask       = {3, 4},
    .timestamp = std::chrono::system_clock::now(),
  };
  // (1/2 + 3/4) / 2 = (2/4 + 3/4) / 2 = (5/4) / 2 = 5/8
  EXPECT_EQ(er.midpoint(), (Rational{5, 8}));
}

// ────────────────────────────────────────────────────────────
// free convert + ExchangeRate::apply
// ────────────────────────────────────────────────────────────

TEST(ConvertTest, UsdToBrl) {
  Currency usd(10000, {1, 1}, iso4217::USD);   // 100.00 USD
  auto r = convert(usd, {5, 1}, iso4217::BRL); // 1 USD = 5 BRL
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 50000);                // 500.00 BRL
  EXPECT_EQ(r->description().symbol, "BRL");
  EXPECT_EQ(r->rate(), (Rational{1, 5}));      // 1 BRL cent = 1/5 USD cent
}

TEST(ConvertTest, UsdToJpy) {
  Currency usd(10000, {1, 1}, iso4217::USD);     // 100.00 USD
  auto r = convert(usd, {110, 1}, iso4217::JPY); // 1 USD = 110 JPY
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 11000);                  // 11000 JPY
  EXPECT_EQ(r->description().symbol, "JPY");
  EXPECT_EQ(r->rate(), (Rational{10, 11}));      // 10 USD cents = 11 JPY
}

TEST(ConvertTest, JpyToUsd) {
  Currency jpy(11000, {10, 11}, iso4217::JPY);   // 11000 JPY (rate from previous convert)
  auto r = convert(jpy, {1, 110}, iso4217::USD); // 1 JPY = 1/110 USD
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 10000);                  // 100.00 USD (back-conversion, exact)
}

TEST(ConvertTest, CurrencyPairValidatesSource) {
  Currency usd(10000, {1, 1}, iso4217::USD);
  CurrencyPair usd_brl{iso4217::USD, iso4217::BRL};

  auto ok = convert(usd, usd_brl, {5, 1});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->units(), 50000);

  Currency eur(10000, {1, 1}, iso4217::EUR);
  auto err = convert(eur, usd_brl, {5, 1});
  ASSERT_FALSE(err.has_value());
  EXPECT_EQ(err.error(), CurrencyError::InvalidData);
}

TEST(ConvertTest, ZeroRateReturnsError) {
  Currency usd(10000, {1, 1}, iso4217::USD);
  auto r = convert(usd, {0, 1}, iso4217::BRL);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST(ConvertTest, ExchangeRateApplyWithFee) {
  Currency usd(10000, {1, 1}, iso4217::USD);  // 100.00 USD
  ExchangeRate er{
    .bid       = {49, 10},   // 4.90 BRL/USD
    .ask       = {51, 10},   // 5.10 BRL/USD
    .fee       = {1, 100},   // 0.01 BRL/USD fee
    .timestamp = std::chrono::system_clock::now(),
  };
  // effectiveAsk = 5.10 + 0.01 = 5.11 BRL/USD
  // 100.00 USD × 5.11 = 511.00 BRL = 51100 cents
  auto r = er.apply(usd, iso4217::BRL);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 51100);
}

TEST(ConvertTest, ExchangeRateApplyBid) {
  Currency brl(50000, {1, 5}, iso4217::BRL);  // 500.00 BRL
  ExchangeRate er{
    .bid       = {19, 100},  // 0.19 USD/BRL
    .ask       = {21, 100},  // 0.21 USD/BRL
    .fee       = {0, 1},
    .timestamp = std::chrono::system_clock::now(),
  };
  // effectiveBid = 0.19 USD/BRL
  // 500.00 BRL × 0.19 = 95.00 USD = 9500 cents
  auto r = er.apply(brl, iso4217::USD, Side::Bid);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 9500);
}

// ────────────────────────────────────────────────────────────
// stress — 50 000 random inputs, invariants verified
// ────────────────────────────────────────────────────────────

TEST(StressTest, InvariantsWithRandomInputs) {
  std::mt19937_64 rng(0xdeadbeefcafe);
  std::uniform_int_distribution<int64_t> udist(-1'000'000LL, 1'000'000LL);
  std::uniform_int_distribution<int64_t> rdist(1, 10'000LL);

  int checked = 0;
  for (int i = 0; i < 50'000; ++i) {
    int64_t u  = udist(rng);
    int64_t rn = udist(rng);  // can be negative
    int64_t rd = rdist(rng);  // always positive

    try {
      Rational rate(rn, rd);
      Currency c(u, rate, {"T", "T$"});

      // scale({1,1}) == identity
      auto id = c.scale({1, 1});
      ASSERT_TRUE(id.has_value());
      EXPECT_EQ(*id, c);

      // add(zero) == identity
      Currency zero(0, rate, {"T", "T$"});
      auto az = c.add(zero);
      ASSERT_TRUE(az.has_value());
      EXPECT_EQ(*az, c);

      // ratio(self) == {1,1} when u != 0 and rate != 0
      if (u != 0 && rn != 0) {
        auto rs = c.ratio(c);
        ASSERT_TRUE(rs.has_value());
        EXPECT_EQ(*rs, (Rational{1, 1}));
      }

      ++checked;
    } catch (const std::exception&) {
      // invalid Rational — expected for edge inputs
    }
  }
  EXPECT_GT(checked, 45'000);
}

// ────────────────────────────────────────────────────────────
// properties
// ────────────────────────────────────────────────────────────

TEST(PropertyTest, ScaleIdentity) {
  Currency c(100000, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, c);
}

TEST(PropertyTest, RatioWithSelf) {
  Currency c(100000, {1, 1}, {"T", "T$"});
  auto r = c.ratio(c);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{1, 1}));
}

TEST(PropertyTest, AddSubtractInverse) {
  Currency usd(100000, {1, 1}, {"USD", "$", 2});
  Currency brl( 50000, {1, 5}, {"BRL", "R$", 2});
  auto added = usd.add(brl);
  ASSERT_TRUE(added.has_value());
  auto restored = added->subtract(brl);
  ASSERT_TRUE(restored.has_value());
  EXPECT_EQ(*restored, usd);
}

TEST(PropertyTest, ScaleAssociative) {
  Currency c(120000, {1, 1}, {"T", "T$"});
  auto r1 = c.scale({2, 1}).and_then([](Currency x){ return x.scale({3, 1}); });
  auto r2 = c.scale({6, 1});
  ASSERT_TRUE(r1.has_value());
  ASSERT_TRUE(r2.has_value());
  EXPECT_EQ(*r1, *r2);
}
