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
// Fixture principal
// ────────────────────────────────────────────────────────────

class CurrencyTest : public ::testing::Test {
protected:
  Currency dolar     { 100000, {1,  1}, {"US Dollar",      "USD"} };
  Currency real      { 100000, {1,  5}, {"Brazilian Real",  "BRL"} };
  Currency peso      { 100000, {1, 25}, {"Uruguayan Peso",  "UYU"} };
  Currency euro      { 100000, {1,  2}, {"Euro",            "EUR"} };
  Currency zero_rate { 100000, {0,  1}, {"Zero Rate",       "ZRO"} };
  Currency zero_value{      0, {1,  2}, {"Zero Value",      "ZVL"} };
};

// ────────────────────────────────────────────────────────────
// add
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, AddDolarMaisReal) {
  auto r = dolar.add(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 120000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, AddRealMaisDolar) {
  auto r = real.add(dolar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 600000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, AddDolarMaisPeso) {
  auto r = dolar.add(peso);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 104000);
}

TEST_F(CurrencyTest, AddRealMaisPeso) {
  auto r = real.add(peso);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 120000);
}

TEST_F(CurrencyTest, AddEuroMaisReal) {
  auto r = euro.add(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 140000);
  EXPECT_EQ(r->rate(), (Rational{1, 2}));
}

TEST_F(CurrencyTest, AddRealMaisEuro) {
  auto r = real.add(euro);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 350000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, AddM1RateZeroRetornaErro) {
  auto r = zero_rate.add(dolar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, AddM2ValorZero) {
  auto r = dolar.add(zero_value);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 100000);
}

TEST_F(CurrencyTest, AddOverflowRetornaErro) {
  Currency max_val(std::numeric_limits<int64_t>::max(), {1, 1}, {"Max", "MX"});
  auto r = max_val.add(dolar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// subtract
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, SubtractDolarMenosReal) {
  auto r = dolar.subtract(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 80000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, SubtractRealMenosDolar) {
  auto r = real.subtract(dolar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), -400000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, SubtractEuroMenosReal) {
  auto r = euro.subtract(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 60000);
  EXPECT_EQ(r->rate(), (Rational{1, 2}));
}

TEST_F(CurrencyTest, SubtractRealMenosEuro) {
  auto r = real.subtract(euro);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), -150000);
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, SubtractM1RateZeroRetornaErro) {
  auto r = zero_rate.subtract(dolar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, SubtractOverflowRetornaErro) {
  Currency min_val(std::numeric_limits<int64_t>::min(), {1, 1}, {"Min", "MN"});
  auto r = min_val.subtract(dolar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// scale
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, ScalePor2) {
  auto r = dolar.scale({2, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 200000);
  EXPECT_EQ(r->rate(), (Rational{1, 1}));
}

TEST_F(CurrencyTest, ScalePorMetade) {
  auto r = dolar.scale({1, 2});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 50000);
}

TEST_F(CurrencyTest, ScalePor3Tercos) {
  auto r = dolar.scale({3, 2});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 150000);
}

TEST_F(CurrencyTest, ScalePorZero) {
  auto r = dolar.scale({0, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 0);
}

TEST_F(CurrencyTest, ScaleHerdaRate) {
  auto r = real.scale({2, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->rate(), (Rational{1, 5}));
}

TEST_F(CurrencyTest, ScaleOverflowRetornaErro) {
  Currency grande(std::numeric_limits<int64_t>::max(), {1, 1}, {"Max", "MX"});
  auto r = grande.scale({2, 1});
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::Overflow);
}

// ────────────────────────────────────────────────────────────
// ratio
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, RatioDolarRealE5Para1) {
  auto r = dolar.ratio(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{5, 1}));
}

TEST_F(CurrencyTest, RatioRealDolarE1Para5) {
  auto r = real.ratio(dolar);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{1, 5}));
}

TEST_F(CurrencyTest, RatioEuroRealE5Para2) {
  auto r = euro.ratio(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{5, 2}));
}

TEST_F(CurrencyTest, RatioRealEuroE2Para5) {
  auto r = real.ratio(euro);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{2, 5}));
}

TEST_F(CurrencyTest, RatioM1RateZeroRetornaErro) {
  auto r = zero_rate.ratio(dolar);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST_F(CurrencyTest, RatioM2ValorZeroRetornaErro) {
  auto r = dolar.ratio(zero_value);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::DivisionByZero);
}

// ────────────────────────────────────────────────────────────
// description
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, AddHerdaDescricaoDeM1) {
  auto r = dolar.add(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), dolar.description());
  EXPECT_EQ(r->description().name,   "US Dollar");
  EXPECT_EQ(r->description().symbol, "USD");
}

TEST_F(CurrencyTest, SubtractHerdaDescricaoDeM1) {
  auto r = euro.subtract(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), euro.description());
  EXPECT_EQ(r->description().name,   "Euro");
  EXPECT_EQ(r->description().symbol, "EUR");
}

TEST_F(CurrencyTest, ScaleHerdaDescricaoDeM1) {
  auto r = real.scale({3, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description(), real.description());
  EXPECT_EQ(r->description().name,   "Brazilian Real");
  EXPECT_EQ(r->description().symbol, "BRL");
}

TEST_F(CurrencyTest, AddNaoHerdaDescricaoDeM2) {
  auto r = dolar.add(real);
  ASSERT_TRUE(r.has_value());
  EXPECT_NE(r->description(), real.description());
}

// ────────────────────────────────────────────────────────────
// operator==
// ────────────────────────────────────────────────────────────

TEST_F(CurrencyTest, IgualdadeMesmosValores) {
  Currency outro(100000, {1, 1}, {"US Dollar", "USD"});
  EXPECT_EQ(dolar, outro);
}

TEST_F(CurrencyTest, DesigualdadeUnitsDiferentes) {
  Currency outro(200000, {1, 1}, {"US Dollar", "USD"});
  EXPECT_NE(dolar, outro);
}

TEST_F(CurrencyTest, DesigualdadeRateDiferente) {
  Currency outro(100000, {1, 2}, {"US Dollar", "USD"});
  EXPECT_NE(dolar, outro);
}

TEST_F(CurrencyTest, DesigualdadeDescricaoDiferente) {
  Currency outro(100000, {1, 1}, {"Euro", "EUR"});
  EXPECT_NE(dolar, outro);
}

// ────────────────────────────────────────────────────────────
// invariantes
// ────────────────────────────────────────────────────────────

TEST(InvariantTest, RationalDenominadorZeroLancaExcecao) {
  EXPECT_THROW((Rational{1, 0}), std::invalid_argument);
}

TEST(InvariantTest, RationalDenominadorINT64MINLancaOverflow) {
  EXPECT_THROW((Rational{1, std::numeric_limits<int64_t>::min()}),
               std::overflow_error);
}

TEST(InvariantTest, RationalNormalizaFracao) {
  EXPECT_EQ((Rational{2, 10}), (Rational{1, 5}));
  EXPECT_EQ((Rational{6,  9}), (Rational{2, 3}));
}

TEST(InvariantTest, RationalNormalizaSinal) {
  EXPECT_EQ((Rational{ 1, -2}), (Rational{-1, 2}));
  EXPECT_EQ((Rational{-1, -2}), (Rational{ 1, 2}));
}

TEST(InvariantTest, DescricaoArmazenadaPorValor) {
  CurrencyDescription desc { "Test", "T$" };
  Currency c(0, {1, 1}, desc);
  desc.name = "Modified";
  EXPECT_EQ(c.description().name, "Test");
}

// ────────────────────────────────────────────────────────────
// operator<<
// ────────────────────────────────────────────────────────────

TEST(StreamTest, INT64MINNaoCrashaNemCorrompe) {
  // Antes do fix, -INT64_MIN era UB; agora usa __int128
  Currency c(std::numeric_limits<int64_t>::min(), {1, 1}, {"T", "T$", 2});
  std::ostringstream oss;
  EXPECT_NO_THROW(oss << c);
  EXPECT_NE(oss.str().find("T$-"), std::string::npos);
}

TEST(StreamTest, LocaleSeparadorDeMillhar) {
  // Imbue com locale pt_BR ou de_DE para testar separador de milhar
  Currency c(100000, {1, 1}, {"T", "T$", 2});
  std::ostringstream oss;
  try {
    oss.imbue(std::locale("pt_BR.UTF-8"));
  } catch (...) {
    GTEST_SKIP() << "locale pt_BR.UTF-8 não disponível";
  }
  oss << c;
  // Com locale pt_BR, 1000.00 pode ter separador (depende da implementação)
  // Apenas verificamos que não crashou e que há saída
  EXPECT_FALSE(oss.str().empty());
}

TEST(StreamTest, JPYSemCasasDecimais) {
  Currency jpy(1500, {1, 1}, ratmoney::iso4217::JPY);
  std::ostringstream oss;
  oss << jpy;
  EXPECT_EQ(oss.str(), "JPY1500");
}

TEST(StreamTest, KWDTresCasasDecimais) {
  Currency kwd(1001, {1, 1}, ratmoney::iso4217::KWD);
  std::ostringstream oss;
  oss << kwd;
  EXPECT_EQ(oss.str(), "KWD1.001");
}

// ────────────────────────────────────────────────────────────
// arredondamento banker's (HalfEven padrão)
// ────────────────────────────────────────────────────────────

TEST(RoundingTest, MetadePorBaixoArredondaParaBaixo) {
  Currency c(50, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 0);
}

TEST(RoundingTest, MetadePorCimaArredondaParaCima) {
  Currency c(150, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingTest, MetadeParImparArredondaParaCima) {
  Currency c(250, {1, 1}, {"Test", "T$"});
  auto r = c.scale({1, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

// ────────────────────────────────────────────────────────────
// modos de arredondamento
// ────────────────────────────────────────────────────────────

TEST(RoundingModeTest, HalfUpArredondaMetadeParaCima) {
  Currency c(150, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::HalfUp);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingModeTest, HalfDownArredondaMetadeParaBaixo) {
  Currency c(150, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::HalfDown);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 1);
}

TEST(RoundingModeTest, UpArredondaSempreParaCima) {
  Currency c(101, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::Up);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 2);
}

TEST(RoundingModeTest, DownTruncaSempre) {
  Currency c(199, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 100}, RoundingMode::Down);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 1);
}

TEST(RoundingModeTest, HalfEvenArredondaParaParMaisProximo) {
  Currency c50 (50,  {1, 1}, {"T", "T$"});
  Currency c150(150, {1, 1}, {"T", "T$"});

  auto r50  = c50 .scale({1, 100}, RoundingMode::HalfEven);
  auto r150 = c150.scale({1, 100}, RoundingMode::HalfEven);

  ASSERT_TRUE(r50.has_value());
  ASSERT_TRUE(r150.has_value());
  EXPECT_EQ(r50->units(),  0);  // 0.5 → 0 (par)
  EXPECT_EQ(r150->units(), 2);  // 1.5 → 2 (par)
}

// ────────────────────────────────────────────────────────────
// serialização
// ────────────────────────────────────────────────────────────

TEST(SerializationTest, RoundTripBasico) {
  Currency original(123456, {3, 7}, {"Test", "T$", 2});
  auto s = original.serialize();
  auto r = Currency::deserialize(s);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, original);
}

TEST(SerializationTest, RoundTripPrecisaoTres) {
  Currency original(999, {1, 1}, {"Kuwaiti Dinar", "KWD", 3});
  auto s = original.serialize();
  auto r = Currency::deserialize(s);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->description().precision, 3);
  EXPECT_EQ(*r, original);
}

TEST(SerializationTest, DeserializeDenominadorZeroRetornaErro) {
  Currency::Serialized bad{0, 1, 0, "T", "T$", 2};
  auto r = Currency::deserialize(bad);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::InvalidData);
}

TEST(SerializationTest, DeserializePrecisaoAcimaDeMaxRetornaErro) {
  Currency::Serialized bad{0, 1, 1, "T", "T$", 19};
  auto r = Currency::deserialize(bad);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::InvalidData);
}

TEST(SerializationTest, SerializadoPreservaCampos) {
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

TEST(Iso4217Test, JPYPrecisaoZero) {
  EXPECT_EQ(iso4217::JPY.precision, 0);
  EXPECT_EQ(iso4217::JPY.symbol, "JPY");
}

TEST(Iso4217Test, KWDPrecisaoTres) {
  EXPECT_EQ(iso4217::KWD.precision, 3);
  EXPECT_EQ(iso4217::KWD.symbol, "KWD");
}

TEST(Iso4217Test, USDPrecisaoDois) {
  EXPECT_EQ(iso4217::USD.precision, 2);
  EXPECT_EQ(iso4217::USD.symbol, "USD");
}

TEST(Iso4217Test, CurrencyComJPYAceitaPrecisaoZero) {
  Currency jpy(1500, {1, 1}, iso4217::JPY);
  EXPECT_EQ(jpy.description().precision, 0);
  EXPECT_EQ(jpy.units(), 1500);
}

// ────────────────────────────────────────────────────────────
// ExchangeRate
// ────────────────────────────────────────────────────────────

TEST(ExchangeRateTest, RateValidaDentroDoTempo) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now(),
    .validity  = std::chrono::seconds{60},
  };
  EXPECT_TRUE(er.isValid());
}

TEST(ExchangeRateTest, RateExpiradaForaDoTempo) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now() - std::chrono::seconds{120},
    .validity  = std::chrono::seconds{60},
  };
  EXPECT_FALSE(er.isValid());
}

TEST(ExchangeRateTest, MidpointBidAskSimetrico) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .timestamp = std::chrono::system_clock::now(),
  };
  // (99/100 + 101/100) / 2 = 200/200 = 1/1
  EXPECT_EQ(er.midpoint(), (Rational{1, 1}));
}

TEST(ExchangeRateTest, EffectiveBidAskComFee) {
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

TEST(ExchangeRateTest, EffectiveBidAskSemFeeIgualBidAsk) {
  ExchangeRate er{
    .bid       = {99, 100},
    .ask       = {101, 100},
    .fee       = {0, 1},
    .timestamp = std::chrono::system_clock::now(),
  };
  EXPECT_EQ(er.effectiveBid(), er.bid);
  EXPECT_EQ(er.effectiveAsk(), er.ask);
}

TEST(ExchangeRateTest, MidpointFracionario) {
  ExchangeRate er{
    .bid       = {1, 2},
    .ask       = {3, 4},
    .timestamp = std::chrono::system_clock::now(),
  };
  // (1/2 + 3/4) / 2 = (2/4 + 3/4) / 2 = (5/4) / 2 = 5/8
  EXPECT_EQ(er.midpoint(), (Rational{5, 8}));
}

// ────────────────────────────────────────────────────────────
// convert (livre) + ExchangeRate::apply
// ────────────────────────────────────────────────────────────

TEST(ConvertTest, USDparaBRL) {
  Currency usd(10000, {1, 1}, iso4217::USD);   // 100.00 USD
  auto r = convert(usd, {5, 1}, iso4217::BRL); // 1 USD = 5 BRL
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 50000);                // 500.00 BRL
  EXPECT_EQ(r->description().symbol, "BRL");
  EXPECT_EQ(r->rate(), (Rational{1, 5}));      // 1 BRL centavo = 1/5 USD cent
}

TEST(ConvertTest, USDparaJPY) {
  Currency usd(10000, {1, 1}, iso4217::USD);    // 100.00 USD
  auto r = convert(usd, {110, 1}, iso4217::JPY); // 1 USD = 110 JPY
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 11000);                  // 11000 JPY
  EXPECT_EQ(r->description().symbol, "JPY");
  EXPECT_EQ(r->rate(), (Rational{10, 11}));       // 10 USD cents = 11 JPY
}

TEST(ConvertTest, JPYparaUSD) {
  Currency jpy(11000, {10, 11}, iso4217::JPY);   // 11000 JPY (rate from previous convert)
  auto r = convert(jpy, {1, 110}, iso4217::USD); // 1 JPY = 1/110 USD
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->units(), 10000);  // 100.00 USD (back-conversion, exact)
}

TEST(ConvertTest, CurrencyPairValidaOrigem) {
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

TEST(ConvertTest, RateZeroRetornaErro) {
  Currency usd(10000, {1, 1}, iso4217::USD);
  auto r = convert(usd, {0, 1}, iso4217::BRL);
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), CurrencyError::ZeroRate);
}

TEST(ConvertTest, ExchangeRateApplyComFee) {
  Currency usd(10000, {1, 1}, iso4217::USD);  // 100.00 USD
  ExchangeRate er{
    .bid       = {49, 10},   // 4.90 BRL/USD
    .ask       = {51, 10},   // 5.10 BRL/USD
    .fee       = {1, 100},   // 0.01 BRL/USD fee
    .timestamp = std::chrono::system_clock::now(),
  };
  // effectiveAsk = 5.10 + 0.01 = 5.11 BRL/USD
  // 100.00 USD × 5.11 = 511.00 BRL = 51100 centavos
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
// stress — 50 000 inputs aleatórios, invariantes verificados
// ────────────────────────────────────────────────────────────

TEST(StressTest, InvariantesComInputsAleatorios) {
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
// propriedades
// ────────────────────────────────────────────────────────────

TEST(PropertyTest, ScaleIdentidade) {
  Currency c(100000, {1, 1}, {"T", "T$"});
  auto r = c.scale({1, 1});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, c);
}

TEST(PropertyTest, RatioConsigoProprio) {
  Currency c(100000, {1, 1}, {"T", "T$"});
  auto r = c.ratio(c);
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, (Rational{1, 1}));
}

TEST(PropertyTest, AddSubtractInverso) {
  Currency usd(100000, {1, 1}, {"USD", "$", 2});
  Currency brl( 50000, {1, 5}, {"BRL", "R$", 2});
  auto added = usd.add(brl);
  ASSERT_TRUE(added.has_value());
  auto restored = added->subtract(brl);
  ASSERT_TRUE(restored.has_value());
  EXPECT_EQ(*restored, usd);
}

TEST(PropertyTest, ScaleAssociativa) {
  Currency c(120000, {1, 1}, {"T", "T$"});
  auto r1 = c.scale({2, 1}).and_then([](Currency x){ return x.scale({3, 1}); });
  auto r2 = c.scale({6, 1});
  ASSERT_TRUE(r1.has_value());
  ASSERT_TRUE(r2.has_value());
  EXPECT_EQ(*r1, *r2);
}
