#pragma once

#include "currency.h"

namespace ratmoney::iso4217 {

// ── Americas ──────────────────────────────────────────────────
inline const CurrencyDescription USD{"US Dollar",          "USD", 2};
inline const CurrencyDescription CAD{"Canadian Dollar",    "CAD", 2};
inline const CurrencyDescription MXN{"Mexican Peso",       "MXN", 2};
inline const CurrencyDescription BRL{"Brazilian Real",     "BRL", 2};
inline const CurrencyDescription ARS{"Argentine Peso",     "ARS", 2};
inline const CurrencyDescription CLP{"Chilean Peso",       "CLP", 0};
inline const CurrencyDescription UYU{"Uruguayan Peso",     "UYU", 2};
inline const CurrencyDescription COP{"Colombian Peso",     "COP", 2};

// ── Europe ────────────────────────────────────────────────────
inline const CurrencyDescription EUR{"Euro",               "EUR", 2};
inline const CurrencyDescription GBP{"British Pound",      "GBP", 2};
inline const CurrencyDescription CHF{"Swiss Franc",        "CHF", 2};
inline const CurrencyDescription SEK{"Swedish Krona",      "SEK", 2};
inline const CurrencyDescription NOK{"Norwegian Krone",    "NOK", 2};
inline const CurrencyDescription DKK{"Danish Krone",       "DKK", 2};
inline const CurrencyDescription PLN{"Polish Zloty",       "PLN", 2};
inline const CurrencyDescription CZK{"Czech Koruna",       "CZK", 2};
inline const CurrencyDescription HUF{"Hungarian Forint",   "HUF", 2};

// ── Asia / Pacific ────────────────────────────────────────────
inline const CurrencyDescription JPY{"Japanese Yen",       "JPY", 0};
inline const CurrencyDescription CNY{"Chinese Yuan",       "CNY", 2};
inline const CurrencyDescription HKD{"Hong Kong Dollar",   "HKD", 2};
inline const CurrencyDescription SGD{"Singapore Dollar",   "SGD", 2};
inline const CurrencyDescription KRW{"South Korean Won",   "KRW", 0};
inline const CurrencyDescription INR{"Indian Rupee",       "INR", 2};
inline const CurrencyDescription AUD{"Australian Dollar",  "AUD", 2};
inline const CurrencyDescription NZD{"New Zealand Dollar", "NZD", 2};
inline const CurrencyDescription TWD{"Taiwan Dollar",      "TWD", 2};
inline const CurrencyDescription THB{"Thai Baht",          "THB", 2};

// ── Middle East / Africa ──────────────────────────────────────
inline const CurrencyDescription KWD{"Kuwaiti Dinar",      "KWD", 3};
inline const CurrencyDescription BHD{"Bahraini Dinar",     "BHD", 3};
inline const CurrencyDescription OMR{"Omani Rial",         "OMR", 3};
inline const CurrencyDescription JOD{"Jordanian Dinar",    "JOD", 3};
inline const CurrencyDescription SAR{"Saudi Riyal",        "SAR", 2};
inline const CurrencyDescription AED{"UAE Dirham",         "AED", 2};
inline const CurrencyDescription ILS{"Israeli Shekel",     "ILS", 2};
inline const CurrencyDescription ZAR{"South African Rand", "ZAR", 2};

// ── Crypto (informal, not ISO) ────────────────────────────────
inline const CurrencyDescription BTC{"Bitcoin",            "BTC", 8};
inline const CurrencyDescription ETH{"Ethereum",           "ETH", 8};  // practical min 8

} // namespace ratmoney::iso4217
