# DESIGN — ratmoney

Relatório técnico de design. Cada decisão é rastreável a uma fonte;
cada invariante é enunciado formalmente e verificável no código.

---

## 1. Problema

### 1.1 Por que `double` está errado para dinheiro

O tipo `double` (IEEE 754-2019, binary64) representa números reais
como `(-1)^s × 2^e × m`, onde `m` é uma mantissa de 53 bits.
Isso significa que apenas frações cujo denominador é potência de 2
têm representação exata. A fração `1/10` — um centavo de real —
não é representável:

```
0.1 (decimal) = 0.0001100110011... (binário, infinito)
```

Na prática:

```cpp
double x = 0.1 + 0.2;
// x == 0.30000000000000004, não 0.3
```

A IEEE 754-2019 garante que o resultado de cada operação é o valor
real arredondado para o representável mais próximo. Para cálculos
acumulados isso introduz erro sistemático — e em sistemas financeiros,
erro sistemático é fraude.

**Referências**: Goldberg (1991) prova formalmente que nenhum
sistema de ponto flutuante binário pode representar `0.1` exatamente.
Bloch (2018), Item 60, demonstra o problema com dinheiro em Java.

### 1.2 Requisitos formais de uma representação monetária correta

Uma representação monetária é **correta** se satisfaz:

1. **Exatidão**: para qualquer valor `v` dentro do intervalo suportado,
   `represent(v)` = `v` sem erro de arredondamento.
2. **Fechamento**: operações sobre representações produzem representações.
3. **Arredondamento determinístico**: dado o mesmo modo de arredondamento
   e os mesmos operandos, o resultado é sempre o mesmo.
4. **Detecção explícita de overflow**: valores fora do intervalo não
   produzem resultados silenciosamente errados — produzem erros.

`double` viola (1) e (4). Uma solução baseada em inteiros + aritmética
racional satisfaz todos os quatro.

---

## 2. Modelo de dados

### 2.1 `Rational` como elemento de ℚ

Seja ℤ o conjunto dos inteiros e ℚ o conjunto dos racionais.
Um `Rational` representa um elemento de ℚ como um par (num, den) ∈ ℤ²
na **forma canônica**:

```
Rational{num, den}  representa  num / den
```

**Invariantes de representação:**

```
I1. den > 0
I2. gcd(|num|, den) = 1  (fração irredutível)
I3. num ∈ [INT64_MIN, INT64_MAX]
I4. den ∈ [1, INT64_MAX]
```

A combinação (I1, I2) garante unicidade: cada racional tem exatamente
uma representação canônica. Sem isso, `{1,2}` e `{2,4}` seriam iguais
semanticamente mas diferentes em memória, violando a igualdade estrutural.

**Construção**: o construtor normaliza qualquer (n, d) para a forma canônica:

```
g  = gcd(|n|, |d|)           via algoritmo de Euclides em __int128
rn = n / g
rd = d / g
se rd < 0: rn = -rn, rd = -rd   (garante I1)
```

O uso de `__int128` é necessário porque `gcd(INT64_MIN, INT64_MIN)` = 2^63,
que não cabe em `int64_t`. A normalização de sinal também pode
produzir `-INT64_MIN` = 2^63, que estoura `int64_t`. Ambos os casos
são detectados e lançam `std::overflow_error`.

**Referências**: Knuth (1997), §4.5, define a forma canônica e prova
unicidade. O algoritmo de Euclides está em Knuth (1997), §4.5.2.

### 2.2 `Currency` como trípla (units, rate, description)

```
Currency = (units : ℤ,  rate : ℚ⁺,  description : CurrencyDescription)
```

- **units**: contagem de unidades na menor denominação da moeda
  (centavos para BRL, cents para USD, unidades para JPY).
- **rate**: valor de 1 unidade desta moeda em unidades de uma moeda-base
  implícita. Permite converter entre moedas.
- **description**: metadados (nome, símbolo, `precision`).

**Semântica de `rate`**: se USD tem `rate = {1,1}` e BRL tem `rate = {1,5}`,
então 1 centavo de BRL = 1/5 cent de USD. A escolha da moeda-base é
implícita e consistente dentro de um conjunto de operações.

**Invariante de `precision`**: `precision` ∈ [0, 18].
O valor 18 é o limite em que `10^precision` ainda cabe em `uint64_t`
(10^19 > UINT64_MAX ≈ 1.8 × 10^19). Validado em `deserialize`.

**Referências**: Fowler (2002), pp. 488–495, formaliza o Money Pattern
como `{amount, currency}`. Evans (2003), cap. 5, fundamenta a
semântica de Value Object imutável.

---

## 3. Algoritmos

### 3.1 GCD via algoritmo de Euclides

```
gcd(a, 0) = a
gcd(a, b) = gcd(b, a mod b),  b ≠ 0
```

**Terminação**: a sequência |b_0|, |b_1|, ... é estritamente decrescente
de inteiros não-negativos, portanto atinge 0 em tempo finito.

**Complexidade**: O(log min(|a|, |b|)) iterações. Para valores int64_t,
no máximo ~93 iterações (Teorema de Lamé: o pior caso é pares
de Fibonacci consecutivos).

**Por que `__int128`**: o GCD precisa computar `a mod b` internamente.
Para `a = INT64_MIN = -2^63`, `|a| = 2^63` que não cabe em `int64_t`.
Com `__int128`, o intervalo é ±2^127, suficiente para todos os produtos
intermediários da normalização.

```cpp
static __int128 gcd128(__int128 a, __int128 b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) { __int128 t = b; b = a % b; a = t; }
    return a == 0 ? 1 : a;
}
```

O caso `a == 0` retorna 1 (não 0) para evitar divisão por zero na
normalização: `Rational{0, d}` → `g = gcd(0, d) = d` → `{0/d, d/d}` = `{0, 1}`.

### 3.2 `roundedDiv` — arredondamento inteiro com 5 modos

**Entrada**: n, d ∈ ℤ, d ≠ 0, modo ∈ {HalfEven, HalfUp, HalfDown, Up, Down}

**Definições auxiliares**:

```
q    = trunc(n / d)          divisão inteira em C++ (trunca em direção a zero)
r    = n - q × d             resto: |r| < |d|, sinal igual a n
pos  = (n ≥ 0) ↔ (d ≥ 0)    verdadeiro se n/d ≥ 0
|r|' = |r| × 2               usado para comparar com |d| sem divisão
```

**Se r = 0**: retorna q imediatamente (resultado exato).

**Caso r ≠ 0**:

| Modo | Condição de incremento | Semantica |
|---|---|---|
| `HalfEven` | `|r|' > |d|`  ou  (`|r|' = |d|` e q ímpar) | arredonda para o par mais próximo |
| `HalfUp`   | `|r|' ≥ |d|` | arredonda metade para longe de zero |
| `HalfDown` | `|r|' > |d|` | arredonda metade em direção a zero |
| `Up`       | sempre        | sempre para longe de zero |
| `Down`     | nunca         | trunca (já é q) |

Quando a condição é verdadeira, o incremento é `+1` se `pos`, `-1` caso contrário.

**Prova de correção para HalfEven (Banker's Rounding)**:

Seja x = n/d ∈ ℚ. Queremos `round(x)` = inteiro mais próximo de x;
em caso de empate, o par.

- Se `|r| < |d|/2`: `|x - q| < 1/2` → q é o mais próximo. ✓
- Se `|r| > |d|/2`: `|x - (q+sign)| < 1/2` → q+sign é o mais próximo. ✓
- Se `|r| = |d|/2` (empate): q e q+sign são equidistantes.
  - q par → retorna q (já é par). ✓
  - q ímpar → retorna q+sign = q±1 (que é par). ✓

A condição de empate `2|r| = |d|` é verificada em inteiros, sem divisão,
evitando erros de ponto flutuante.

**Por que HalfEven é não-enviesado**: para uma distribuição uniforme de
empates, metade cai em q par (sem ajuste) e metade em q ímpar (ajuste
para cima ou para baixo com igual probabilidade). O valor esperado do
erro de arredondamento converge para zero.

HalfUp, em contraste, sistematicamente arredonda para cima em empates,
introduzindo viés positivo de `+0.5/n` por operação (onde n é a base
de arredondamento). Em sistemas financeiros com bilhões de transações,
esse viés acumula.

**Referências**: IEEE 754-2019, §4.3.3, define `roundTiesToEven`.
Cowlishaw (2003), §2.7, analisa o viés de cada modo.
Hauser (1996) discute implementações corretas de arredondamento.

### 3.3 `convertUnits128` — conversão entre taxas

**Teorema**: dado um valor `A` em unidades da menor denominação de
uma moeda X (com taxa `rₓ = from.num / from.den`), o valor equivalente
em unidades da menor denominação de uma moeda Y (com taxa `r_y = to.num / to.den`)
na mesma moeda-base é:

```
B = round( A × from.num × to.den / (from.den × to.num) )
```

**Prova**:

```
valor_base = A × rₓ = A × (from.num / from.den)

B = valor_base / r_y
  = [A × (from.num / from.den)] / (to.num / to.den)
  = A × from.num × to.den / (from.den × to.num)    □
```

**Análise de overflow**:

O numerador `n = A × from.num × to.den` é um produto de três valores
int64_t. `__int128` suporta ±2^127 ≈ ±1.7 × 10^38.

```
|A|         ≤  9.2 × 10^18  (INT64_MAX)
|from.num|  ≤  9.2 × 10^18
|to.den|    ≤  9.2 × 10^18
produto     ≤  (9.2 × 10^18)^3 ≈ 7.8 × 10^56  >>  2^127
```

**Limitação conhecida**: para valores extremos (|A|, |from.num|,
|to.den| todos próximos de INT64_MAX), o produto pode estourar
`__int128` silenciosamente. Na prática, taxas de câmbio normalizadas
por GCD têm numeradores e denominadores na ordem de 10^3–10^6, e
quantias monetárias são limitadas pelo negócio; o produto permanece
em ≈ 10^30 << 2^127. Esta limitação é documentada mas não verificada
em tempo de execução por razões de desempenho.

---

## 4. Modos de arredondamento

### 4.1 Tabela de comportamento

Todas as colunas assumem `scale({1, 100})` sobre `Currency(n, {1,1}, ...)`.
O valor real é `n/100`; o resultado inteiro é o valor arredondado.

| n   | n/100 real | HalfEven | HalfUp | HalfDown | Up | Down |
|-----|------------|----------|--------|----------|----|------|
| 50  | 0.5        | 0        | 1      | 0        | 1  | 0    |
| 150 | 1.5        | 2        | 2      | 1        | 2  | 1    |
| 250 | 2.5        | 2        | 3      | 2        | 3  | 2    |
| 350 | 3.5        | 4        | 4      | 3        | 4  | 3    |
| 101 | 1.01       | 1        | 1      | 1        | 2  | 1    |
| 199 | 1.99       | 2        | 2      | 2        | 2  | 1    |
| -50 | -0.5       | 0        | -1     | 0        | -1 | 0    |
| -150| -1.5       | -2       | -2     | -1       | -2 | -1   |

### 4.2 Relação com IEEE 754-2019

| Este projeto    | IEEE 754-2019         | Descrição |
|---|---|---|
| `HalfEven`      | `roundTiesToEven`     | Padrão bancário; não-enviesado |
| `HalfUp`        | `roundTiesToAway`     | Arredondamento "escolar" |
| `HalfDown`      | (não nomeado)         | Inverso de HalfUp |
| `Up`            | `roundTowardPositive` | Equivalente a teto (⌈x⌉) para x ≥ 0 |
| `Down`          | `roundTowardZero`     | Truncamento (⌊|x|⌋ × sign) |

Nota: IEEE 754 também define `roundTowardNegative` (chão, ⌊x⌋), que
não está implementado aqui pois não é comum em contextos financeiros.

### 4.3 Normas bancárias

O arredondamento bancário padrão em sistemas reais:

- **Banco Central do Brasil (BCB)**: arredondamento a 2 casas decimais;
  o Cosif especifica HalfUp para apresentação.
- **ISO 4217**: define `precision` (exponent) mas não o modo de arredondamento;
  deixa para o sistema implementador.
- **Decimal Arithmetic (IBM/Python/Java)**: padrão `ROUND_HALF_EVEN` como
  default (Cowlishaw, 2003).

O padrão deste projeto é `HalfEven` por ser matematicamente não-enviesado;
todos os outros modos são explicitamente disponíveis via `RoundingMode`.

---

## 5. Decisões de design

### 5.1 `int64_t` + `Rational` vs. alternativas

| Alternativa | Problema |
|---|---|
| `double` | Representação inexata de decimais (§1.1) |
| `long double` | Mesmo problema, expoente maior |
| Decimal fixo (e.g. centavos em int64_t) | Não suporta taxas de câmbio arbitrárias sem aproximação |
| BigInteger / BigDecimal | Alocações dinâmicas; latência imprevisível |
| `__int128` diretamente | Sem suporte a ponto flutuante decimal; não portável fora de GCC/Clang |

`int64_t` para unidades + `Rational` para taxas oferece:
- Representação exata de qualquer valor inteiro de unidades
- Conversão exata entre moedas desde que o produto intermediário caiba em `__int128`
- Operações sem alocação dinâmica (exceto no construtor via `make_unique<Impl>`)
- Overflow detectável e explícito via `fitsInt64`

**Referência**: Knuth (1997), §4.5, compara aritmética racional com ponto
flutuante para aplicações de precisão exata.

### 5.2 `std::expected<T, CurrencyError>` vs. exceções

Exceções têm dois problemas em código de domínio financeiro:

1. **Invisibilidade**: `add(other)` pode lançar; sem `noexcept`, o chamador
   não sabe quais erros são possíveis olhando apenas a assinatura.
2. **Custo em hot paths**: o mecanismo de stack unwinding tem overhead
   em compiladores sem zero-cost exceptions (comuns em código embarcado
   ou de alta frequência).

`std::expected<T, CurrencyError>` força o chamador a verificar o resultado:

```cpp
auto r = a.add(b);
if (!r) handle(r.error());   // erro explícito no tipo
```

O atributo `[[nodiscard]]` garante erro de compilação se o resultado
for ignorado.

**Referência**: P0323R12 (Douglas & Filion, 2022) documenta o raciocínio
formal para `std::expected` no C++23. Alexandrescu (2012) introduziu
o conceito de `Expected<T,E>` como alternativa type-safe a exceções.

### 5.3 pImpl (Pointer to Implementation)

```cpp
class Currency {
    struct Impl;
    std::unique_ptr<Impl> impl_;   // opaco para o chamador
};
```

**Motivação — estabilidade de ABI**: sem pImpl, adicionar um campo
privado a `Currency` muda o `sizeof(Currency)`, quebrando todo código
compilado contra a versão anterior. Com pImpl, `sizeof(Currency)` = 
`sizeof(std::unique_ptr)` = 8 bytes, independente do tamanho de `Impl`.

**Trade-off**: cada acesso a `impl_->field` é uma indireção de ponteiro.
Em arquiteturas modernas com cache L1 quente, isso é ~4 ns vs ~1 ns
para acesso direto. Aceitável para domínio financeiro (não é HFT puro).

**Regra dos Cinco**: `unique_ptr` com tipo incompleto requer que
destrutor, copy constructor e copy assignment sejam definidos no `.cpp`
(onde `Impl` é completo), não no `.h`. Move constructor e move
assignment podem usar `= default` no `.cpp`.

**Referências**: Sutter (2000), Item 26, define o Fast Pimpl Idiom e
analisa os trade-offs de ABI. Meyers (2014), Item 22, especifica
as regras de `unique_ptr` com tipos incompletos.

### 5.4 `Rational` vs. decimal fixo

Decimal fixo (e.g., `int64_t` representando centavos) é simples mas
incapaz de representar taxas de câmbio arbitrárias. `1 BRL = 0.2 USD`
requer um denominador 5; não existe representação decimal finita.

`Rational{1, 5}` representa 1/5 exatamente. A consequência é que
operações como `add` precisam converter entre taxas, introduzindo
divisão racional — que é exata, mas pode requerer arredondamento
ao projetar de volta para inteiros.

O contrato da biblioteca é: **a aritmética é exata**; **o arredondamento
é explícito e controlado pelo chamador via `RoundingMode`**.

---

## 6. Validação

### 6.1 Propriedades matemáticas verificadas

O `StressTest` executa 50.000 inputs aleatórios via `mt19937_64` e
verifica três propriedades para cada `Currency c(u, rate, desc)` válida:

**P1 — Scale identity**:
```
c.scale({1, 1}) = c
```
Prova: `roundedDiv(u × 1, 1, mode) = roundedDiv(u, 1, mode) = u` (exato). ✓

**P2 — Add-zero identity**:
```
c.add(Currency(0, rate, desc)) = c
```
Prova: `convertUnits128(0, rate, rate, mode) = 0`; `u + 0 = u`. ✓

**P3 — Ratio-self**:
```
u ≠ 0 → c.ratio(c) = Rational{1, 1}
```
Prova: `convertUnits128(u, rate, rate, mode) = u` (mesmo rate, divisão exata);
`Rational(u, u)` normaliza para `{1, 1}` via GCD. ✓

O `StressTest` também verifica implicitamente que nenhuma dessas
operações lança exceção para inputs válidos, cobrindo o comportamento
de `noexcept` informal das operações.

### 6.2 O que o fuzzer cobre

O `fuzz_target.cpp` com libFuzzer + ASan + UBSan detecta:

- Acesso fora de limites em `gcd128`, `roundedDiv`, `convertUnits128`
- Overflow de inteiro com sinal (UBSan: `signed integer overflow`)
- Use-after-free ou double-free em operações de cópia/move do pImpl
- Leitura de memória não-inicializada
- Qualquer combinação de inputs que produza crash ou undefined behavior

**O que o fuzzer não cobre**:

- Correção semântica (e.g., resultado errado sem crash)
- Overflow silencioso de `__int128` em `convertUnits128` para inputs extremos (§3.3)
- Interação com o sistema operacional (locale, `std::chrono`)

### 6.3 Limites conhecidos

| Limitação | Causa | Condição de ocorrência |
|---|---|---|
| Overflow em `convertUnits128` | produto de 3 fatores int64 pode exceder `__int128` | `|units| × |from.num| × |to.den| > 2^127` |
| `precision` > 18 rejeitado | `10^19 > UINT64_MAX` em `operator<<` | qualquer `deserialize` com `precision ≥ 19` |
| `Rational` com denominador `INT64_MIN` rejeitado | `-INT64_MIN = INT64_MAX + 1` estoura | chamada `Rational(n, INT64_MIN)` |
| `operator<<` não suporta `precision` > 18 | mesmo motivo acima | moedas fictícias com precisão extrema |

---

## 7. Conformidade normativa

### 7.1 ISO 4217:2015

O padrão ISO 4217 define para cada moeda:

- **Alphabetic code**: 3 letras (USD, BRL, EUR, ...)
- **Numeric code**: 3 dígitos (840 para USD)
- **Minor unit** (= `precision`): número de casas decimais da menor denominação

Mapeamento das constantes `ratmoney::iso4217`:

| Constante | ISO alphabetic | ISO numeric | Minor unit |
|---|---|---|---|
| `USD` | USD | 840 | 2 |
| `EUR` | EUR | 978 | 2 |
| `BRL` | BRL | 986 | 2 |
| `JPY` | JPY | 392 | 0 |
| `KWD` | KWD | 414 | 3 |
| `BHD` | BHD | 048 | 3 |
| `CLP` | CLP | 152 | 0 |
| `BTC` | (não ISO) | — | 8 (convencional) |

Nota: BTC e ETH não têm código ISO 4217 oficial; o `precision = 8`
(satoshi / gwei) é convencional entre plataformas.

O numeric code não está implementado; o alphabetic code está no campo
`symbol` por convenção desta biblioteca (não é o campo `name`).

### 7.2 Modos de arredondamento em normas bancárias brasileiras

O **COSIF** (Plano Contábil das Instituições do Sistema Financeiro Nacional,
Banco Central do Brasil) especifica arredondamento a 2 casas decimais
para demonstrações contábeis. O modo não é explicitamente nomeado
como HalfEven, mas a prática padrão em sistemas SISBACEN é HalfUp.

A escolha de `HalfEven` como padrão desta biblioteca segue a norma
internacional (IEEE 754, Cowlishaw/IBM Decimal Arithmetic) e é
matematicamente não-enviesada. Para uso com sistemas que requerem
HalfUp, o parâmetro `RoundingMode` deve ser passado explicitamente.

---

## Referências

- **Alexandrescu, A. (2012).** *Systematic Error Handling in C++.* NDC 2012.
- **Bloch, J. (2018).** *Effective Java*, 3ª ed., Item 60. Addison-Wesley.
- **Cowlishaw, M.F. (2003).** *General Decimal Arithmetic Specification*, v1.12. IBM.
- **Douglas, V. & Filion, J. (2022).** *P0323R12: std::expected.* ISO/IEC JTC1/SC22/WG21.
- **Evans, E. (2003).** *Domain-Driven Design*, cap. 5. Addison-Wesley.
- **Fowler, M. (2002).** *Patterns of Enterprise Application Architecture*, pp. 488–495. Addison-Wesley.
- **Goldberg, D. (1991).** *What Every Computer Scientist Should Know About Floating-Point Arithmetic.* ACM Computing Surveys, 23(1), 5–48.
- **Hauser, J.R. (1996).** *Handling Floating-Point Exceptions in Numeric Programs.* ACM TOMS, 22(2).
- **IEEE Std 754-2019.** *IEEE Standard for Floating-Point Arithmetic.* IEEE.
- **ISO 4217:2015.** *Codes for the representation of currencies and funds.* ISO.
- **Knuth, D.E. (1997).** *The Art of Computer Programming, Vol. 2*, §4.5. Addison-Wesley.
- **Martin, R.C. (2008).** *Clean Code*, cap. 6. Prentice Hall.
- **Meyers, S. (2014).** *Effective Modern C++*, Item 22. O'Reilly.
- **Sutter, H. (2000).** *Exceptional C++*, Item 26. Addison-Wesley.
- **Warren, H.S. Jr. (2012).** *Hacker's Delight*, 2ª ed., caps. 2 e 12. Addison-Wesley.
- **CERT C++ Coding Standard — INT32-C.** SEI/Carnegie Mellon University.
