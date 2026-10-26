// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tree_stack.hpp"
#include "checkpoint.hpp"
#include <cstdint>
#include <cstring>

namespace scas {

TreeStack::TreeStack(uint8_t * buffer, uint32_t bufferSize)
    : buffer(buffer), bufferSize(bufferSize), size(0) {}
Tree * TreeStack::pushBlock(uint8_t block){
    if (size>=bufferSize){
        Checkpoint::raise(Error::StackOverflow);
    }
    buffer[size++]=block;
    return Tree::fromBlock(buffer+size-1);
}
Tree * TreeStack::pushSmallInt(uint8_t value, bool neg){
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegSmallInt:Type::PosSmallInt));
    pushBlock(value);
    return tree;
}
Tree * TreeStack::pushBigInt(const uint8_t * digits, uint8_t nDigit, bool neg){
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegBigInt:Type::PosBigInt));
    pushBlock(nDigit);
    for (uint8_t i = 0; i<nDigit; i++){
        pushBlock(digits[i]);
    }
    return tree;
}
Tree * TreeStack::pushSmallRational(uint8_t numerator, uint8_t denominator, bool neg){
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegSmallRational:Type::PosSmallRational));
    pushBlock(numerator);
    pushBlock(denominator);
    return tree;
}
Tree * TreeStack::pushBigRational(const uint8_t * digits, uint8_t nNumeratorDigit, uint8_t nDenominatorDigit, bool neg){
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegBigRational:Type::PosBigRational));
    pushBlock(nNumeratorDigit);
    pushBlock(nDenominatorDigit);
    for (uint16_t i = 0; i<nNumeratorDigit+nDenominatorDigit; i++){
        pushBlock(digits[i]);
    }
    return tree;
}
Tree * TreeStack::pushSymbol(const char * name, uint8_t nChar){
    Tree * tree = pushBlock(static_cast<uint8_t>(Type::Symbol));
    pushBlock(nChar);
    for (uint8_t i = 0; i<nChar; i++){
        pushBlock(name[i]);
    }
    return tree;
}
Tree * TreeStack::pushNode(Type type){
    return pushBlock(static_cast<uint8_t>(type));
}
Tree * TreeStack::pushNAry(Type type, uint8_t nChildren){
    Tree * tree = pushBlock(static_cast<uint8_t>(type));
    pushBlock(nChildren);
    return tree;
}
Tree * TreeStack::pushTree(Tree * tree){
    uint32_t treeSize = tree->treeSize();
    if (size+treeSize>bufferSize){Checkpoint::raise(Error::StackOverflow);}
    Tree * newTree = Tree::fromBlock(buffer+size);
    memcpy(buffer+size, tree, treeSize);
    size+=treeSize;
    return newTree;
}
Tree * TreeStack::keepOnly(uint32_t mark, Tree * result){
    uint32_t n = result->treeSize();
    if (mark+n>bufferSize){Checkpoint::raise(Error::StackOverflow);}
    uint8_t * dest = buffer+mark;
    uint8_t * src = result->block();
    if (src!=dest){
        memmove(dest, src, n);
    }
    size = mark+n;
    return Tree::fromBlock(dest);
}
void TreeStack::trimTo(uint32_t mark){
    if (mark>size){
        mark = size;
    }
    size = mark;
}
}