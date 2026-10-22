#pragma once
#include <cstdint>
#include "tree.hpp"
#include "tree_stack.hpp"

namespace scas {

struct DivResult{
    Tree * qotient;
    Tree * remainder;
};

class Integer{
public:
    const uint8_t * digits;
    uint8_t nDigit;
    bool neg;

    Integer(const uint8_t * digits, uint8_t nDigit, bool neg);
    Integer(Tree * tree);

    uint32_t toUint32();
    
    static Tree * fromUint32(uint32_t n, TreeStack  * stack, bool neg = false);
    static Tree * fromUint64(uint64_t n, TreeStack  * stack, bool neg = false);
    static Tree * add(Integer a, Integer b, TreeStack * stack);
    static Tree * absAdd(Integer a, Integer b, TreeStack * stack, bool neg);
    static Tree * absSub(Integer a, Integer b, TreeStack * stack, bool neg);
    static Tree * mul(Integer a, Integer b, TreeStack * stack);
    static DivResult absDiv(Integer a, Integer b, TreeStack * stack);
    static Tree * gcd(Integer a, Integer b, TreeStack * stack);
    static int8_t absCompare(Integer a, Integer b);
};

}