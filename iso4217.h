#pragma once

#include "currency.h"

namespace ratmoney::iso4217 {

// ── North America ──────────────────────────────────────────────
inline const CurrencyDescription BMD{"Bermudian Dollar",              "BMD", 2};
inline const CurrencyDescription CAD{"Canadian Dollar",               "CAD", 2};
inline const CurrencyDescription MXN{"Mexican Peso",                  "MXN", 2};
inline const CurrencyDescription USD{"US Dollar",                     "USD", 2};

// ── Central America & Caribbean ───────────────────────────────
inline const CurrencyDescription ANG{"Netherlands Antillean Guilder", "ANG", 2};
inline const CurrencyDescription AWG{"Aruban Florin",                 "AWG", 2};
inline const CurrencyDescription BBD{"Barbadian Dollar",              "BBD", 2};
inline const CurrencyDescription BSD{"Bahamian Dollar",               "BSD", 2};
inline const CurrencyDescription BZD{"Belize Dollar",                 "BZD", 2};
inline const CurrencyDescription CRC{"Costa Rican Colón",             "CRC", 2};
inline const CurrencyDescription CUP{"Cuban Peso",                    "CUP", 2};
inline const CurrencyDescription DOP{"Dominican Peso",                "DOP", 2};
inline const CurrencyDescription GTQ{"Guatemalan Quetzal",            "GTQ", 2};
inline const CurrencyDescription HNL{"Honduran Lempira",              "HNL", 2};
inline const CurrencyDescription HTG{"Haitian Gourde",                "HTG", 2};
inline const CurrencyDescription JMD{"Jamaican Dollar",               "JMD", 2};
inline const CurrencyDescription KYD{"Cayman Islands Dollar",         "KYD", 2};
inline const CurrencyDescription NIO{"Nicaraguan Córdoba",            "NIO", 2};
inline const CurrencyDescription PAB{"Panamanian Balboa",             "PAB", 2};
inline const CurrencyDescription SVC{"Salvadoran Colón",              "SVC", 2};
inline const CurrencyDescription TTD{"Trinidad and Tobago Dollar",    "TTD", 2};
inline const CurrencyDescription XCD{"East Caribbean Dollar",         "XCD", 2};

// ── South America ─────────────────────────────────────────────
inline const CurrencyDescription ARS{"Argentine Peso",                "ARS", 2};
inline const CurrencyDescription BOB{"Bolivian Boliviano",            "BOB", 2};
inline const CurrencyDescription BRL{"Brazilian Real",                "BRL", 2};
inline const CurrencyDescription CLP{"Chilean Peso",                  "CLP", 0};
inline const CurrencyDescription COP{"Colombian Peso",                "COP", 2};
inline const CurrencyDescription GYD{"Guyanese Dollar",               "GYD", 2};
inline const CurrencyDescription PEN{"Peruvian Sol",                  "PEN", 2};
inline const CurrencyDescription PYG{"Paraguayan Guaraní",            "PYG", 0};
inline const CurrencyDescription SRD{"Surinamese Dollar",             "SRD", 2};
inline const CurrencyDescription UYU{"Uruguayan Peso",                "UYU", 2};
inline const CurrencyDescription VES{"Venezuelan Bolívar Soberano",   "VES", 2};

// ── Europe ────────────────────────────────────────────────────
inline const CurrencyDescription ALL{"Albanian Lek",                  "ALL", 2};
inline const CurrencyDescription AMD{"Armenian Dram",                 "AMD", 2};
inline const CurrencyDescription AZN{"Azerbaijani Manat",             "AZN", 2};
inline const CurrencyDescription BAM{"Convertible Mark",              "BAM", 2};
inline const CurrencyDescription BGN{"Bulgarian Lev",                 "BGN", 2};
inline const CurrencyDescription BYN{"Belarusian Ruble",              "BYN", 2};
inline const CurrencyDescription CHF{"Swiss Franc",                   "CHF", 2};
inline const CurrencyDescription CZK{"Czech Koruna",                  "CZK", 2};
inline const CurrencyDescription DKK{"Danish Krone",                  "DKK", 2};
inline const CurrencyDescription EUR{"Euro",                          "EUR", 2};
inline const CurrencyDescription FKP{"Falkland Islands Pound",        "FKP", 2};
inline const CurrencyDescription GBP{"British Pound",                 "GBP", 2};
inline const CurrencyDescription GEL{"Georgian Lari",                 "GEL", 2};
inline const CurrencyDescription GIP{"Gibraltar Pound",               "GIP", 2};
inline const CurrencyDescription HUF{"Hungarian Forint",              "HUF", 2};
inline const CurrencyDescription ISK{"Icelandic Króna",               "ISK", 0};
inline const CurrencyDescription MDL{"Moldovan Leu",                  "MDL", 2};
inline const CurrencyDescription MKD{"Macedonian Denar",              "MKD", 2};
inline const CurrencyDescription NOK{"Norwegian Krone",               "NOK", 2};
inline const CurrencyDescription PLN{"Polish Zloty",                  "PLN", 2};
inline const CurrencyDescription RON{"Romanian Leu",                  "RON", 2};
inline const CurrencyDescription RSD{"Serbian Dinar",                 "RSD", 2};
inline const CurrencyDescription RUB{"Russian Ruble",                 "RUB", 2};
inline const CurrencyDescription SEK{"Swedish Krona",                 "SEK", 2};
inline const CurrencyDescription SHP{"Saint Helena Pound",            "SHP", 2};
inline const CurrencyDescription TRY{"Turkish Lira",                  "TRY", 2};
inline const CurrencyDescription UAH{"Ukrainian Hryvnia",             "UAH", 2};

// ── Middle East ───────────────────────────────────────────────
inline const CurrencyDescription AED{"UAE Dirham",                    "AED", 2};
inline const CurrencyDescription BHD{"Bahraini Dinar",                "BHD", 3};
inline const CurrencyDescription ILS{"Israeli New Shekel",            "ILS", 2};
inline const CurrencyDescription IQD{"Iraqi Dinar",                   "IQD", 3};
inline const CurrencyDescription IRR{"Iranian Rial",                  "IRR", 2};
inline const CurrencyDescription JOD{"Jordanian Dinar",               "JOD", 3};
inline const CurrencyDescription KWD{"Kuwaiti Dinar",                 "KWD", 3};
inline const CurrencyDescription LBP{"Lebanese Pound",                "LBP", 2};
inline const CurrencyDescription OMR{"Omani Rial",                    "OMR", 3};
inline const CurrencyDescription QAR{"Qatari Riyal",                  "QAR", 2};
inline const CurrencyDescription SAR{"Saudi Riyal",                   "SAR", 2};
inline const CurrencyDescription SYP{"Syrian Pound",                  "SYP", 2};
inline const CurrencyDescription YER{"Yemeni Rial",                   "YER", 2};

// ── Africa ────────────────────────────────────────────────────
inline const CurrencyDescription AOA{"Angolan Kwanza",                "AOA", 2};
inline const CurrencyDescription BIF{"Burundian Franc",               "BIF", 0};
inline const CurrencyDescription BWP{"Botswana Pula",                 "BWP", 2};
inline const CurrencyDescription CDF{"Congolese Franc",               "CDF", 2};
inline const CurrencyDescription CVE{"Cape Verdean Escudo",           "CVE", 2};
inline const CurrencyDescription DJF{"Djiboutian Franc",              "DJF", 0};
inline const CurrencyDescription DZD{"Algerian Dinar",                "DZD", 2};
inline const CurrencyDescription EGP{"Egyptian Pound",                "EGP", 2};
inline const CurrencyDescription ERN{"Eritrean Nakfa",                "ERN", 2};
inline const CurrencyDescription ETB{"Ethiopian Birr",                "ETB", 2};
inline const CurrencyDescription GHS{"Ghanaian Cedi",                 "GHS", 2};
inline const CurrencyDescription GMD{"Gambian Dalasi",                "GMD", 2};
inline const CurrencyDescription GNF{"Guinean Franc",                 "GNF", 0};
inline const CurrencyDescription KES{"Kenyan Shilling",               "KES", 2};
inline const CurrencyDescription KMF{"Comorian Franc",                "KMF", 0};
inline const CurrencyDescription LRD{"Liberian Dollar",               "LRD", 2};
inline const CurrencyDescription LSL{"Lesotho Loti",                  "LSL", 2};
inline const CurrencyDescription LYD{"Libyan Dinar",                  "LYD", 3};
inline const CurrencyDescription MAD{"Moroccan Dirham",               "MAD", 2};
inline const CurrencyDescription MGA{"Malagasy Ariary",               "MGA", 2};
inline const CurrencyDescription MRU{"Mauritanian Ouguiya",           "MRU", 2};
inline const CurrencyDescription MUR{"Mauritian Rupee",               "MUR", 2};
inline const CurrencyDescription MWK{"Malawian Kwacha",               "MWK", 2};
inline const CurrencyDescription MZN{"Mozambican Metical",            "MZN", 2};
inline const CurrencyDescription NAD{"Namibian Dollar",               "NAD", 2};
inline const CurrencyDescription NGN{"Nigerian Naira",                "NGN", 2};
inline const CurrencyDescription RWF{"Rwandan Franc",                 "RWF", 0};
inline const CurrencyDescription SCR{"Seychellois Rupee",             "SCR", 2};
inline const CurrencyDescription SDG{"Sudanese Pound",                "SDG", 2};
inline const CurrencyDescription SLE{"Sierra Leonean Leone",          "SLE", 2};
inline const CurrencyDescription SOS{"Somali Shilling",               "SOS", 2};
inline const CurrencyDescription SSP{"South Sudanese Pound",          "SSP", 2};
inline const CurrencyDescription STN{"São Tomé and Príncipe Dobra",   "STN", 2};
inline const CurrencyDescription SZL{"Swazi Lilangeni",               "SZL", 2};
inline const CurrencyDescription TND{"Tunisian Dinar",                "TND", 3};
inline const CurrencyDescription TZS{"Tanzanian Shilling",            "TZS", 2};
inline const CurrencyDescription UGX{"Ugandan Shilling",              "UGX", 0};
inline const CurrencyDescription XAF{"CFA Franc BEAC",                "XAF", 0};
inline const CurrencyDescription XOF{"CFA Franc BCEAO",               "XOF", 0};
inline const CurrencyDescription ZAR{"South African Rand",            "ZAR", 2};
inline const CurrencyDescription ZMW{"Zambian Kwacha",                "ZMW", 2};
inline const CurrencyDescription ZWG{"Zimbabwe Gold",                 "ZWG", 2};

// ── Asia ──────────────────────────────────────────────────────
inline const CurrencyDescription AFN{"Afghan Afghani",                "AFN", 2};
inline const CurrencyDescription BDT{"Bangladeshi Taka",              "BDT", 2};
inline const CurrencyDescription BND{"Brunei Dollar",                 "BND", 2};
inline const CurrencyDescription BTN{"Bhutanese Ngultrum",            "BTN", 2};
inline const CurrencyDescription CNY{"Chinese Yuan",                  "CNY", 2};
inline const CurrencyDescription HKD{"Hong Kong Dollar",              "HKD", 2};
inline const CurrencyDescription IDR{"Indonesian Rupiah",             "IDR", 2};
inline const CurrencyDescription INR{"Indian Rupee",                  "INR", 2};
inline const CurrencyDescription JPY{"Japanese Yen",                  "JPY", 0};
inline const CurrencyDescription KGS{"Kyrgyzstani Som",               "KGS", 2};
inline const CurrencyDescription KHR{"Cambodian Riel",                "KHR", 2};
inline const CurrencyDescription KPW{"North Korean Won",              "KPW", 2};
inline const CurrencyDescription KRW{"South Korean Won",              "KRW", 0};
inline const CurrencyDescription KZT{"Kazakhstani Tenge",             "KZT", 2};
inline const CurrencyDescription LAK{"Lao Kip",                       "LAK", 2};
inline const CurrencyDescription LKR{"Sri Lankan Rupee",              "LKR", 2};
inline const CurrencyDescription MMK{"Myanmar Kyat",                  "MMK", 2};
inline const CurrencyDescription MNT{"Mongolian Tögrög",              "MNT", 2};
inline const CurrencyDescription MOP{"Macanese Pataca",               "MOP", 2};
inline const CurrencyDescription MVR{"Maldivian Rufiyaa",             "MVR", 2};
inline const CurrencyDescription MYR{"Malaysian Ringgit",             "MYR", 2};
inline const CurrencyDescription NPR{"Nepalese Rupee",                "NPR", 2};
inline const CurrencyDescription PHP{"Philippine Peso",               "PHP", 2};
inline const CurrencyDescription PKR{"Pakistani Rupee",               "PKR", 2};
inline const CurrencyDescription SGD{"Singapore Dollar",              "SGD", 2};
inline const CurrencyDescription THB{"Thai Baht",                     "THB", 2};
inline const CurrencyDescription TJS{"Tajikistani Somoni",            "TJS", 2};
inline const CurrencyDescription TMT{"Turkmenistani Manat",           "TMT", 2};
inline const CurrencyDescription TWD{"New Taiwan Dollar",             "TWD", 2};
inline const CurrencyDescription UZS{"Uzbekistani Som",               "UZS", 2};
inline const CurrencyDescription VND{"Vietnamese Đồng",               "VND", 0};

// ── Pacific / Oceania ─────────────────────────────────────────
inline const CurrencyDescription AUD{"Australian Dollar",             "AUD", 2};
inline const CurrencyDescription FJD{"Fijian Dollar",                 "FJD", 2};
inline const CurrencyDescription NZD{"New Zealand Dollar",            "NZD", 2};
inline const CurrencyDescription PGK{"Papua New Guinean Kina",        "PGK", 2};
inline const CurrencyDescription SBD{"Solomon Islands Dollar",        "SBD", 2};
inline const CurrencyDescription TOP{"Tongan Pa'anga",                "TOP", 2};
inline const CurrencyDescription VUV{"Vanuatu Vatu",                  "VUV", 0};
inline const CurrencyDescription WST{"Samoan Tala",                   "WST", 2};
inline const CurrencyDescription XPF{"CFP Franc",                     "XPF", 0};

// ── Precious metals (ISO 4217 X-codes) ───────────────────────
inline const CurrencyDescription XAG{"Silver",                        "XAG", 0};
inline const CurrencyDescription XAU{"Gold",                          "XAU", 0};
inline const CurrencyDescription XPD{"Palladium",                     "XPD", 0};
inline const CurrencyDescription XPT{"Platinum",                      "XPT", 0};

// ── Fund codes (ISO 4217) ─────────────────────────────────────
inline const CurrencyDescription BOV{"Bolivian Mvdol",                "BOV", 2};
inline const CurrencyDescription CHE{"WIR Euro",                      "CHE", 2};
inline const CurrencyDescription CHW{"WIR Franc",                     "CHW", 2};
inline const CurrencyDescription CLF{"Chilean UF",                    "CLF", 4};
inline const CurrencyDescription COU{"Colombian Real Value Unit",     "COU", 2};
inline const CurrencyDescription MXV{"Mexican Unidad de Inversión",   "MXV", 2};
inline const CurrencyDescription USN{"US Dollar (Next Day)",          "USN", 2};
inline const CurrencyDescription UYI{"Uruguayan Peso (Indexed)",      "UYI", 0};
inline const CurrencyDescription UYW{"Unidad Previsional",            "UYW", 4};
inline const CurrencyDescription XDR{"Special Drawing Rights",        "XDR", 0};
inline const CurrencyDescription XSU{"Sucre",                         "XSU", 0};
inline const CurrencyDescription XUA{"ADB Unit of Account",           "XUA", 0};

// ── Non-currency codes ────────────────────────────────────────
inline const CurrencyDescription XTS{"Test Currency",                 "XTS", 2};
inline const CurrencyDescription XXX{"No Currency",                   "XXX", 0};

// ── Crypto (informal, not ISO) ────────────────────────────────
inline const CurrencyDescription BTC{"Bitcoin",                       "BTC", 8};
inline const CurrencyDescription ETH{"Ethereum",                      "ETH", 8};

} // namespace ratmoney::iso4217
