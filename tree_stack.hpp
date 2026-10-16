#pragma once
#include <cstdint>
#include "tree.hpp"

namespace scas {

class TreeRef;

class TreeStack {

public:
    uint8_t * buffer;
    uint32_t bufferSize;
    uint32_t size;
    TreeStack(uint8_t * buffer, uint32_t bufferSize);
    Tree * treeAtId(uint16_t id);
    TreeRef newTreeRef(Tree * tree);
    void deleteTreeRef(uint16_t id);
    uint16_t duplicateRef(uint16_t id);
    void deleteTree(Tree * tree);
    Tree * pushBlock(uint8_t block);
    Tree * pushSmallInt(uint8_t value, bool neg);
    Tree * pushBigInt(uint8_t * digits, uint8_t nDigit, bool neg);
    Tree * pushSmallRational(uint8_t numerator, uint8_t denominator, bool neg);
    Tree * pushBigRational(uint8_t * digits, uint8_t nNumeratorDigit, uint8_t nDenominatorDigit, bool neg);
    Tree * pushSymbol(const char * name, uint8_t nChar);
    Tree * pushDiv();
    Tree * pushPow();
    Tree * pushMul(uint8_t nChildren);
    Tree * pushAdd(uint8_t nChildren);
    Tree * pushTree(Tree * tree);
private:
    static constexpr uint16_t kMaxRefs = 256;
    uint32_t refs[kMaxRefs];// -1=not used
};

}