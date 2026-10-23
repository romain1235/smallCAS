#include "tree.hpp"
#include <cstdint>
#include <cstring>

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

bool Tree::isIdenticalTo(Tree * other){
    uint32_t n = treeSize();
    return n==other->treeSize() && memcmp(block(), other->block(), n)==0;
}

bool Tree::isNumber(){
    switch (type()) {
        case Type::PosSmallInt:
        case Type::NegSmallInt:
        case Type::PosBigInt:
        case Type::NegBigInt:
        case Type::PosSmallRational:
        case Type::NegSmallRational:
        case Type::PosBigRational:
        case Type::NegBigRational:
            return true;
        default:
            return false;
    }
}

bool Tree::isInteger(){
    switch (type()) {
        case Type::PosSmallInt:
        case Type::NegSmallInt:
        case Type::PosBigInt:
        case Type::NegBigInt:
            return true;
        default:
            return false;
    }
}

uint8_t Tree::numberOfChildren(){
    switch (type()) {
        case Type::Div:
        case Type::Pow:
        case Type::Sub:
            return 2;
        case Type::Mul:
        case Type::Add:
            return asNAry()->nChildren;
        case Type::Opposite:
        case Type::Parentheses:
            return 1;
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
Tree * Tree::child(uint8_t index){
    Tree * c = nextNode();
    for (uint8_t i = 0; i < index; i++) {
        c = c->nextTree();
    }
    return c;
}

}