#pragma once

#include <cstdint>
namespace scas {

enum class Type: uint8_t{
    PosSmallInt,
    NegSmallInt,
    PosBigInt,
    NegBigInt,
    PosSmallRational,
    NegSmallRational,
    PosBigRational,
    NegBigRational,
    Mul,
    Add,
    Sub,
    Pow,
    Div,
    Symbol,
    Opposite,
    Parentheses,
};

struct SmallInt{
    Type type;
    uint8_t value;
};

struct BigInt{
    Type type;
    uint8_t nDigit;
    uint8_t digits[];
};

struct SmallRational{
    Type type;
    uint8_t numerator;
    uint8_t denominator;
};

struct BigRational{
    Type type;
    uint8_t nNumeratorDigit;
    uint8_t nDenominatorDigit;
    uint8_t digits[];
};

struct NAry{
    Type type;
    uint8_t nChildren;
};

struct Symbol{
    Type type;
    uint8_t nChar;
    char name[];//without \0 at the end
};

}