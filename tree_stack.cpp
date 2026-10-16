#include "tree_stack.hpp"
#include "tree_ref.hpp"
#include <cstdint>
#include <cstring>

namespace scas {

TreeStack::TreeStack(uint8_t * buffer, uint32_t bufferSize)
    : buffer(buffer), bufferSize(bufferSize), size(0) {
        for (uint16_t i = 0; i < kMaxRefs; i++){
            refs[i]=-1;
        }
    }

Tree * TreeStack::treeAtId(uint16_t id){
    if (refs[id]==-1){return nullptr;}
    if (refs[id]==-2){return nullptr;}
    return Tree::fromBlock(buffer+refs[id]);
}
TreeRef TreeStack::newTreeRef(Tree * tree){
    for (uint16_t i = 0; i < kMaxRefs; i++){
        if (refs[i]==-1){
            refs[i] = tree->block()-buffer;
            return TreeRef(i,this);
        }
    }
    return TreeRef(static_cast<uint16_t>(-1), nullptr);
}
void TreeStack::deleteTreeRef(uint16_t id){
    refs[id] = -1;
}
uint16_t TreeStack::duplicateRef(uint16_t id){
    if (id == static_cast<uint16_t>(-1)) {
        return id;
    }
    uint32_t offset = refs[id];
    for (uint16_t i = 0; i < kMaxRefs; i++) {
        if (refs[i] == static_cast<uint32_t>(-1)) {
            refs[i] = offset;
            return i;
        }
    }
    return static_cast<uint16_t>(-1);
}
void TreeStack::deleteTree(Tree *tree){
    uint8_t * start = tree->block();
    uint32_t n = tree->treeSize();
    uint32_t offset = start - buffer;
    memmove(start, start+n, size-offset-n);
    size-=n;
    for(uint16_t i = 0; i<kMaxRefs; i++){
        if (refs[i]==-1){continue;}
        if (refs[i]==-2){continue;}
        if(refs[i]>=offset+n){
            refs[i]-=n;
        }else if (refs[i]>=offset){
            refs[i]=-2;
        }
    }
}
Tree * TreeStack::pushBlock(uint8_t block){
    if (size>=bufferSize){return nullptr;}
    buffer[size++]=block;
    return Tree::fromBlock(buffer+size-1);
}
Tree * TreeStack::pushSmallInt(uint8_t value, bool neg){
    if (size+2>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegSmallInt:Type::PosSmallInt));
    pushBlock(value);
    return tree;
}
Tree * TreeStack::pushBigInt(uint8_t * digits, uint8_t nDigit, bool neg){
    if (size+2+nDigit>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegBigInt:Type::PosBigInt));
    pushBlock(nDigit);
    for (uint8_t i = 0; i<nDigit; i++){
        pushBlock(digits[i]);
    }
    return tree;
}
Tree * TreeStack::pushSmallRational(uint8_t numerator, uint8_t denominator, bool neg){
    if (size+3>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegSmallRational:Type::PosSmallRational));
    pushBlock(numerator);
    pushBlock(denominator);
    return tree;
}
Tree * TreeStack::pushBigRational(uint8_t * digits, uint8_t nNumeratorDigit, uint8_t nDenominatorDigit, bool neg){
    if (size+3+nNumeratorDigit+nDenominatorDigit>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(neg?Type::NegBigRational:Type::PosBigRational));
    pushBlock(nNumeratorDigit);
    pushBlock(nDenominatorDigit);
    for (uint16_t i = 0; i<nNumeratorDigit+nDenominatorDigit; i++){
        pushBlock(digits[i]);
    }
    return tree;
}
Tree * TreeStack::pushSymbol(const char * name, uint8_t nChar){
    if (size+2+nChar>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(Type::Symbol));
    pushBlock(nChar);
    for (uint8_t i = 0; i<nChar; i++){
        pushBlock(name[i]);
    }
    return tree;
}
Tree * TreeStack::pushDiv(){
    return pushBlock(static_cast<uint8_t>(Type::Div));
}
Tree * TreeStack::pushPow(){
    return pushBlock(static_cast<uint8_t>(Type::Pow));    
}
Tree * TreeStack::pushMul(uint8_t nChildren){
    if (size+2>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(Type::Mul));
    pushBlock(nChildren);
    return tree;
}
Tree * TreeStack::pushAdd(uint8_t nChildren){
    if (size+2>bufferSize){return nullptr;}
    Tree * tree = pushBlock(static_cast<uint8_t>(Type::Add));
    pushBlock(nChildren);
    return tree;
}
Tree * TreeStack::pushTree(Tree * tree){
    uint32_t treeSize = tree->treeSize();
    if (size+treeSize>bufferSize){return nullptr;}
    Tree * newTree = Tree::fromBlock(buffer+size);
    memcpy(buffer+size, tree, treeSize);
    size+=treeSize;
    return newTree;
}
}