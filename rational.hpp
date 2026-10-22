#pragma once
#include <cstdint>
#include "tree.hpp"
#include "integer.hpp"

namespace scas {

struct RationalInts{
    Integer numerator;
    Integer denominator;
};

class Rational{
public:
    const uint8_t * numerator;
    const uint8_t * denominator;
    uint8_t nNumeratorDigit;
    uint8_t nDenominatorDigit;
    bool neg;

    Rational(uint8_t * numerator, uint8_t nNumeratorDigit, uint8_t * denominator, uint8_t nDenominatorDigit, bool neg);
    Rational(Tree * tree);

    RationalInts toInts();
    static Tree * fromInts(Integer numerator, Integer denominator, TreeStack * stack);
    static int8_t absCompare(Rational a, Rational b, TreeStack * stack);
    static int8_t compare(Rational a, Rational b, TreeStack * stack);
    static Tree * add(Rational a, Rational b, TreeStack * stack);
    static Tree * mul(Rational a, Rational b, TreeStack * stack);
    static Tree * div(Rational a, Rational b, TreeStack * stack);
};

}