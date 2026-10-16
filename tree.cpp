#include "tree.hpp"
#include <cstdint>

namespace scas {

uint16_t Tree::headerSize(){
    switch (type()) {
        case Type::NegSmallInt:
        case Type::PosSmallInt:
            return sizeof(SmallInt);
        case Type::NegSmallRational:
        case Type::PosSmallRational:
            return sizeof(SmallRational);
        case Type::NegBigInt:
        case Type::PosBigInt:
            return sizeof(BigInt)+asBigInt()->nDigit;
        case Type::NegBigRational:
        case Type::PosBigRational:
            return sizeof(BigRational)+asBigRational()->nDenominatorDigit+asBigRational()->nNumeratorDigit;
        case Type::Symbol:
            return sizeof(Symbol)+asSymbol()->nChar;
        case Type::Add:
        case Type::Mul:
            return sizeof(NAry);
        default:
            return 1;
    };    
}

uint8_t Tree::numberOfChildren(){
    switch (type()) {
        case Type::Div:
        case Type::Pow:
            return 2;
        case Type::Mul:
        case Type::Add:
            return asNAry()->nChildren;
        default:
            return 0;
    }
}

Tree * Tree::nextNode(){
    return fromBlock(block()+headerSize());
}
Tree * Tree::nextTree(){
    Tree * t = nextNode();
    uint16_t remaining = numberOfChildren();
    while (remaining > 0) {
      remaining += t->numberOfChildren();
      t = t->nextNode();
      remaining--;
    }
    return t;
}
uint32_t Tree::treeSize(){
    return nextTree()->block()-block();
}

}