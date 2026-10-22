#include "rational.hpp"
#include <cstdint>

namespace scas {

namespace {
constexpr uint8_t one = 1;

Tree * simplifyRational(Tree * rational, uint32_t mark, TreeStack * stack){
    switch (rational->type()) {
        case Type::PosBigRational:
        case Type::NegBigRational:{
            uint8_t nNum = rational->asBigRational()->nNumeratorDigit;
            uint8_t nDen = rational->asBigRational()->nDenominatorDigit;
            uint8_t * digits = rational->asBigRational()->digits;
            bool neg = rational->type()==Type::NegBigRational;
            if (nNum==1 && digits[0]==0){
                stack->trimTo(mark);
                return stack->pushSmallInt(0, false);
            }
            if (nDen==1 && digits[nNum]==1){
                if (nNum==1){
                    uint8_t value = digits[0];
                    stack->trimTo(mark);
                    return stack->pushSmallInt(value, neg);
                }
                return stack->keepOnly(mark,stack->pushBigInt(digits, nNum, neg));
            }
            if (nNum==1 && nDen==1){
                uint8_t numerator = digits[0];
                uint8_t denominator = digits[1];
                stack->trimTo(mark);
                return stack->pushSmallRational(numerator, denominator, neg);
            }
            return rational;
        }
        default:
            return rational;
    }
}
}

Rational::Rational(uint8_t * numerator, uint8_t nNumeratorDigit, uint8_t * denominator, uint8_t nDenominatorDigit, bool neg)
    : numerator(numerator), denominator(denominator), nNumeratorDigit(nNumeratorDigit), nDenominatorDigit(nDenominatorDigit), neg(neg){}

Rational::Rational(Tree * tree){
    switch (tree->type()) {
        case Type::PosSmallRational:
        case Type::PosBigRational:
        case Type::PosSmallInt:
        case Type::PosBigInt:
            neg = false;
            break;
        case Type::NegSmallRational:
        case Type::NegBigRational:
        case Type::NegSmallInt:
        case Type::NegBigInt:
            neg = true;
            break;
    }
    switch (tree->type()) {
        case Type::PosSmallRational:
        case Type::NegSmallRational:
            nNumeratorDigit = 1;
            nDenominatorDigit = 1;
            numerator = &tree->asSmallRational()->numerator;
            denominator = &tree->asSmallRational()->denominator;
            break;
        case Type::PosBigRational:
        case Type::NegBigRational:
            nNumeratorDigit = tree->asBigRational()->nNumeratorDigit;
            nDenominatorDigit = tree->asBigRational()->nDenominatorDigit;
            numerator = tree->asBigRational()->digits;
            denominator = numerator + nNumeratorDigit;
            break;
        case Type::PosSmallInt:
        case Type::NegSmallInt:
            nNumeratorDigit = 1;
            nDenominatorDigit = 1;
            numerator = &tree->asSmallInt()->value;
            denominator = &one;
            break;
        case Type::PosBigInt:
        case Type::NegBigInt:
            nNumeratorDigit = tree->asBigInt()->nDigit;
            nDenominatorDigit = 1;
            numerator = tree->asBigInt()->digits;
            denominator = &one;
            break;
    }
}

RationalInts Rational::toInts(){
    return {
        Integer(numerator, nNumeratorDigit, neg),
        Integer(denominator, nDenominatorDigit, false)
    };
}

Tree * Rational::fromInts(Integer numerator, Integer denominator, TreeStack * stack){
    uint32_t mark = stack->size;
    Tree * gcd = Integer::gcd(numerator, denominator, stack);
    uint32_t divMark = stack->size;
    Integer newNumerator = stack->keepOnly(divMark, Integer::absDiv(numerator, gcd, stack).qotient);
    divMark = stack->size;
    Integer newDenominator = stack->keepOnly(divMark, Integer::absDiv(denominator, gcd, stack).qotient);
    Tree * rational = stack->pushBigRational(nullptr, 0, 0, numerator.neg);
    for (uint8_t i = 0; i<newNumerator.nDigit; i++){
        stack->pushBlock(newNumerator.digits[i]);
    }
    for (uint8_t i = 0; i<newDenominator.nDigit; i++){
        stack->pushBlock(newDenominator.digits[i]);
    }
    rational->asBigRational()->nNumeratorDigit = newNumerator.nDigit;
    rational->asBigRational()->nDenominatorDigit = newDenominator.nDigit;
    rational = stack->keepOnly(mark, rational);
    return simplifyRational(rational, mark, stack);
}

int8_t Rational::absCompare(Rational a, Rational b, TreeStack * stack){
    uint32_t mark = stack->size;
    RationalInts aInts = a.toInts();
    RationalInts bInts = b.toInts();
    int8_t c = Integer::absCompare(
        Integer::mul(aInts.numerator, bInts.denominator, stack), 
        Integer::mul(bInts.numerator, aInts.denominator, stack)
    );
    stack->trimTo(mark);
    return c;
}

int8_t Rational::compare(Rational a, Rational b, TreeStack *stack){
    if (a.neg==b.neg){
        if (a.neg){
            return -absCompare(a, b, stack);
        }else{
            return absCompare(a, b, stack);
        }
    }else{
        return a.neg?-1:1;
    }
}

Tree * Rational::add(Rational a, Rational b, TreeStack *stack){
    RationalInts aInts = a.toInts();
    RationalInts bInts = b.toInts();
    uint32_t mark = stack->size;
    Integer numerator = stack->keepOnly(mark, Integer::add(
        Integer::mul(aInts.numerator, bInts.denominator, stack), 
        Integer::mul(aInts.denominator, bInts.numerator, stack), 
        stack));
    Integer denominator = Integer::mul(aInts.denominator, bInts.denominator, stack);
    return stack->keepOnly(mark, fromInts(numerator, denominator, stack));
}

Tree * Rational::mul(Rational a, Rational b, TreeStack *stack){
    RationalInts aInts = a.toInts();
    RationalInts bInts = b.toInts();
    uint32_t mark = stack->size;
    Integer numerator = Integer::mul(aInts.numerator, bInts.numerator, stack);
    Integer denominator = Integer::mul(aInts.denominator, bInts.denominator, stack);
    return stack->keepOnly(mark, fromInts(numerator, denominator, stack));
}

Tree * Rational::div(Rational a, Rational b, TreeStack *stack){
    RationalInts aInts = a.toInts();
    RationalInts bInts = b.toInts();
    bInts.denominator.neg = bInts.numerator.neg;
    uint32_t mark = stack->size;
    Integer numerator = Integer::mul(aInts.numerator, bInts.denominator, stack);
    Integer denominator = Integer::mul(aInts.denominator, bInts.numerator, stack);
    return stack->keepOnly(mark, fromInts(numerator, denominator, stack));
}


}