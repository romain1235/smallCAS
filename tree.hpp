#pragma once
#include <cstdint>
#include "type.hpp"
#include "serialize.hpp"

namespace scas {

class TreeStack;

class Tree{
public:
    uint16_t headerSize();
    uint8_t numberOfChildren();
    Tree * nextNode();
    Tree * nextTree();
    Tree * child(uint8_t index);
    uint32_t treeSize();
    uint8_t * block(){return reinterpret_cast<uint8_t *>(this);}
    static Tree * fromBlock(uint8_t * b) {
        return reinterpret_cast<Tree *>(b);
    }

    SmallInt * asSmallInt(){return reinterpret_cast<SmallInt *>(this);}
    BigInt * asBigInt(){return reinterpret_cast<BigInt *>(this);}
    SmallRational * asSmallRational(){return reinterpret_cast<SmallRational *>(this);}
    BigRational * asBigRational(){return reinterpret_cast<BigRational *>(this);}
    Symbol * asSymbol(){return reinterpret_cast<Symbol *>(this);}
    NAry * asNAry(){return reinterpret_cast<NAry *>(this);}

    Type type() {return static_cast<Type>(block()[0]);}

    void serialize(char * out, uint16_t size);//in serialize.cpp
    LayoutShape layoutShape(bool left, bool layout/*if we are serializing in a layout*/);//in serialize.cpp
    bool hasParentheses(uint8_t parentPriority);//in serialize.cpp

    Tree * project(TreeStack * stack);//in projection.cpp
    Tree * simpleReduction(TreeStack * stack);//in simple_reduction.cpp
private:
    void serialize(char * out, uint16_t size, uint16_t & p);//in serialize.cpp
};
}