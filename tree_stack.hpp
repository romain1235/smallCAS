#pragma once
#include <cstdint>
#include "tree.hpp"

namespace scas {

class TreeStack {

public:
    uint8_t * buffer;
    uint32_t bufferSize;
    uint32_t size;
    TreeStack(uint8_t * buffer, uint32_t bufferSize);
    Tree * pushBlock(uint8_t block);
    Tree * pushSmallInt(uint8_t value, bool neg);
    Tree * pushBigInt(const uint8_t * digits, uint8_t nDigit, bool neg);
    Tree * pushSmallRational(uint8_t numerator, uint8_t denominator, bool neg);
    Tree * pushBigRational(const uint8_t * digits, uint8_t nNumeratorDigit, uint8_t nDenominatorDigit, bool neg);
    Tree * pushSymbol(const char * name, uint8_t nChar);
    Tree * pushNode(Type type);
    Tree * pushNAry(Type type, uint8_t nChildren);
    Tree * pushTree(Tree * tree);
    Tree * keepOnly(uint32_t mark, Tree * result);
    void trimTo(uint32_t mark);
};

}