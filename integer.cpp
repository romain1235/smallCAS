#include "integer.hpp"
#include "checkpoint.hpp"
#include <cassert>
#include <csetjmp>
#include <cstdint>
#include <cstring>

namespace scas {

namespace {
//to use this, the integer should be a the end of the stack
Tree * simplifyInteger(Tree * integer, uint32_t mark, TreeStack * stack){
    switch (integer->type()) {
        case Type::PosBigInt:
        case Type::NegBigInt:{
            int16_t i = integer->asBigInt()->nDigit-1;
            while(i>=0 && integer->asBigInt()->digits[i]==0){
                integer->asBigInt()->nDigit--;
                stack->size--;
                i--;
            }
            if (integer->asBigInt()->nDigit>127){
                Checkpoint::raise(Error::IntOverflow);
            }
            if (integer->asBigInt()->nDigit==1){
                uint8_t value = integer->asBigInt()->digits[0];
                bool neg = integer->type()==Type::NegBigInt;
                stack->trimTo(mark);
                return stack->pushSmallInt(value, neg);
            }else if(integer->asBigInt()->nDigit==0){
                stack->trimTo(mark);
                return stack->pushSmallInt(0, false);
            }else{
                return integer;
            }
        }
        default:
            return integer;
    }
}

Tree * pushShifted(Integer x, uint8_t shift, TreeStack * stack){
    Tree * result = stack->pushBigInt(nullptr, 0, false);
    uint16_t carry = 0;
    for (uint8_t i = 0; i<x.nDigit; i++){
        carry |= static_cast<uint16_t>(x.digits[i])<<shift;
        stack->pushBlock(static_cast<uint8_t>(carry));
        carry >>= 8;
    }
    uint8_t n = x.nDigit;
    if (carry!=0){
        stack->pushBlock(static_cast<uint8_t>(carry));
        n++;
    }
    result->asBigInt()->nDigit = n;
    return result;
}

Tree * pushHalved(Integer x, TreeStack * stack){
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, false);
    for (uint8_t i = 0; i<x.nDigit; i++){
        stack->pushBlock(0);
    }
    result->asBigInt()->nDigit = x.nDigit;
    uint8_t * digits = result->asBigInt()->digits;
    uint8_t carry = 0;
    for (int16_t i = x.nDigit-1; i>=0; i--){
        uint8_t cur = x.digits[i];
        digits[i] = (cur>>1) | carry;
        carry = (cur & 1)<<7;
    }
    return simplifyInteger(result, mark, stack);
}

}

Integer::Integer(const uint8_t * digits, uint8_t nDigit, bool neg)
    : digits(digits), nDigit(nDigit), neg(neg){}

Integer::Integer(Tree * tree){
    switch (tree->type()) {
        case Type::PosSmallInt:
        case Type::PosBigInt:
            neg = false;   
            break;
        case Type::NegSmallInt:
        case Type::NegBigInt:
            neg = true;
            break;
    }
    switch (tree->type()) {
        case Type::PosSmallInt:
        case Type::NegSmallInt:
            nDigit = 1;
            digits = &tree->asSmallInt()->value;
            break;
        case Type::PosBigInt:
        case Type::NegBigInt:
            nDigit = tree->asBigInt()->nDigit;
            digits = tree->asBigInt()->digits;
            break;
    }
}

Tree * Integer::fromUint32(uint32_t n, TreeStack * stack, bool neg){
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, neg);
    while (n!=0) {
        stack->pushBlock(static_cast<uint8_t>(n));
        result->asBigInt()->nDigit++;
        n>>=8;        
    }
    return simplifyInteger(result, mark, stack);
}

Tree * Integer::fromUint64(uint64_t n, TreeStack * stack, bool neg){
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, neg);
    while (n!=0) {
        stack->pushBlock(static_cast<uint8_t>(n));
        result->asBigInt()->nDigit++;
        n>>=8;        
    }
    return simplifyInteger(result, mark, stack);
}

uint32_t Integer::toUint32(){
    uint32_t result = 0;
    for (uint8_t i = 0; i<nDigit&&i<4; i++){
        result|=static_cast<uint32_t>(digits[i])<<(i*8);
    }
    return result;
}

Tree * Integer::add(Integer a, Integer b, TreeStack * stack){
    if (a.neg==b.neg){
        return absAdd(a, b, stack, a.neg);
    }else{
        if (absCompare(a, b)==-1){
            return absSub(b, a, stack, b.neg);
        }else{
            return absSub(a, b, stack, a.neg);
        }
    }
}
Tree * Integer::absAdd(Integer a, Integer b, TreeStack * stack, bool neg){
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, neg);
    uint16_t carry = a.digits[0]+b.digits[0];
    uint8_t i = 0;
    uint8_t n = a.nDigit > b.nDigit ? a.nDigit : b.nDigit;
    while (carry!=0 || i<n) {
        stack->pushBlock(static_cast<uint8_t>(carry));
        carry>>=8;
        i++;
        if (i<a.nDigit){
            carry+=a.digits[i];
        }
        if (i<b.nDigit){
            carry+=b.digits[i];
        }
    }
    result->asBigInt()->nDigit = i;
    return simplifyInteger(result, mark, stack);
}
Tree * Integer::absSub(Integer a, Integer b, TreeStack * stack, bool neg){
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, neg);
    int16_t carry = a.digits[0]-b.digits[0];
    uint8_t i = 0;
    while (i<a.nDigit) {
        stack->pushBlock(static_cast<uint8_t>(carry));
        carry = carry<0 ? -1 : 0;
        i++;
        if (i<a.nDigit){
            carry+=a.digits[i];
        }
        if (i<b.nDigit){
            carry-=b.digits[i];
        }
    }
    result->asBigInt()->nDigit = i;
    return simplifyInteger(result, mark, stack);
}
Tree * Integer::mul(Integer a, Integer b, TreeStack *stack){
    if (a.nDigit<=4 && b.nDigit<=4){
        uint32_t intA = a.toUint32();
        uint32_t intB = b.toUint32();
        uint64_t result = static_cast<uint64_t>(intA)*intB;
        return fromUint64(result, stack, a.neg != b.neg);
    }
    uint32_t mark = stack->size;
    Tree * result = stack->pushBigInt(nullptr, 0, a.neg != b.neg);
    uint8_t n = a.nDigit + b.nDigit;
    for (uint8_t i = 0; i < n; i++) {
        stack->pushBlock(0);
    }
    result->asBigInt()->nDigit = n;
    for (uint8_t i = 0; i<a.nDigit; i++){
        uint16_t carry = 0;
        for (uint8_t j = 0; j<b.nDigit; j++){
            carry += result->asBigInt()->digits[i + j] + a.digits[i] * b.digits[j];
            result->asBigInt()->digits[i + j] = static_cast<uint8_t>(carry);
            carry >>= 8;
        }
        result->asBigInt()->digits[i + b.nDigit] = static_cast<uint8_t>(carry);
    }
    return simplifyInteger(result, mark, stack);
}
DivResult Integer::absDiv(Integer a, Integer b, TreeStack *stack){
    assert(!(b.nDigit==1 && b.digits[0]==0));
    uint32_t mark = stack->size;
    if (absCompare(a, b)<0){
        Tree * remainder = a.nDigit==1
            ? stack->pushSmallInt(a.digits[0], false)
            : stack->pushBigInt(a.digits, a.nDigit, false);
        return {stack->pushSmallInt(0, false), remainder};
    }
    if (a.nDigit<=4 && b.nDigit<=4){
        uint32_t intA = a.toUint32();
        uint32_t intB = b.toUint32();
        Tree * quotient = fromUint32(intA/intB, stack);
        Tree * remainder = fromUint32(intA%intB, stack);
        return {quotient, remainder};
    }
    uint8_t shift = 0;
    uint8_t top = b.digits[b.nDigit-1];
    while ((top & 0x80)==0){
        top <<= 1;
        shift++;
    }
    Tree * bTree = pushShifted(b, shift, stack);
    Tree * aTree = pushShifted(a, shift, stack);
    stack->pushBlock(0);
    aTree->asBigInt()->nDigit++;
    uint8_t bN = bTree->asBigInt()->nDigit;
    uint8_t aN = aTree->asBigInt()->nDigit-1;
    uint8_t * bDigits = bTree->asBigInt()->digits;
    uint8_t * aDigits = aTree->asBigInt()->digits;
    uint8_t qN = aN-bN+1;
    uint32_t qMark = stack->size;
    Tree * qTree = stack->pushBigInt(nullptr, 0, false);
    for (uint8_t i = 0; i<qN; i++){
        stack->pushBlock(0);
    }
    qTree->asBigInt()->nDigit = qN;
    uint8_t * qDigits = qTree->asBigInt()->digits;
    for (int16_t j = qN-1; j>=0; j--){
        uint16_t qhat = ((uint16_t)aDigits[j+bN]<<8 | aDigits[j+bN-1]) / bDigits[bN-1];
        if (qhat>255){
            qhat = 255;
        }
        int32_t borrow = 0;
        for (uint8_t i = 0; i<bN; i++){
            int32_t diff = (int32_t)aDigits[j+i] - (int32_t)qhat*bDigits[i] + borrow;
            aDigits[j+i] = static_cast<uint8_t>(diff);
            borrow = diff<0 ? -((-diff+255)>>8) : diff>>8;
        }
        int32_t high = (int32_t)aDigits[j+bN] + borrow;
        while (high<0){
            qhat--;
            uint16_t carry = 0;
            for (uint8_t i = 0; i<bN; i++){
                carry += aDigits[j+i]+bDigits[i];
                aDigits[j+i] = static_cast<uint8_t>(carry);
                carry >>= 8;
            }
            high += carry;
        }
        aDigits[j+bN] = static_cast<uint8_t>(high);
        qDigits[j] = static_cast<uint8_t>(qhat);
    }
    qTree = simplifyInteger(qTree, qMark, stack);
    if (shift!=0){
        uint8_t carry = 0;
        for (int16_t i = bN-1; i>=0; i--){
            uint8_t cur = aDigits[i];
            aDigits[i] = (cur>>shift) | carry;
            carry = static_cast<uint8_t>(cur<<(8-shift));
        }
    }
    uint32_t remainderMark = stack->size;
    Tree * remainder = stack->pushTree(aTree);
    remainder = simplifyInteger(remainder, remainderMark, stack);
    uint32_t bytes = stack->size-qMark;
    memmove(stack->buffer+mark, stack->buffer+qMark, bytes);
    stack->size = mark+bytes;
    qTree = Tree::fromBlock(stack->buffer+mark);
    return {qTree, qTree->nextTree()};
}
Tree * Integer::gcd(Integer a, Integer b, TreeStack * stack){
    if (a.nDigit<=4 && b.nDigit<=4){
        uint32_t intA = a.toUint32();
        uint32_t intB = b.toUint32();
        while(intB!=0){
            uint32_t temp = intA%intB;
            intA = intB;
            intB = temp;
        }
        return fromUint32(intA, stack);
    }
    uint32_t mark = stack->size;
    Tree * aTree = stack->pushBigInt(a.digits, a.nDigit, false);
    Tree * bTree = stack->pushBigInt(b.digits, b.nDigit, false);
    a = Integer(aTree);
    b = Integer(bTree);
    while (b.nDigit!=1 || b.digits[0]!=0) {
        uint32_t divMark = stack->size;
        Integer temp = Integer(stack->keepOnly(divMark, absDiv(a, b, stack).remainder));
        a = b;
        b= temp;
        uint32_t aSize = aTree->treeSize();
        memmove(stack->buffer+mark, stack->buffer+mark+aSize, stack->size-mark-aSize);
        stack->size-=aSize;
        a.digits-=aSize;
        b.digits-=aSize;
    }
    Tree * result = stack->keepOnly(mark, aTree);
    return simplifyInteger(result, mark, stack);
}
int8_t Integer::absCompare(Integer a, Integer b){
    if (a.nDigit==b.nDigit){
        for (uint8_t i = a.nDigit; i>0; i--){
            if (a.digits[i-1]>b.digits[i-1]){
                return 1;
            }else if (a.digits[i-1]<b.digits[i-1]){
                return -1;
            }
        }
        return 0;
    }else{
        return a.nDigit<b.nDigit?-1:1;
    }
}
Tree * Integer::pow(Integer base, Integer exp, TreeStack *stack){
    if (exp.nDigit==1 && exp.digits[0]==0){
        assert(!(base.nDigit==1 && base.digits[0]==0));
        return stack->pushSmallInt(1, false);
    }
    uint32_t mark = stack->size;
    Tree * i1 = stack->pushSmallInt(1, false);
    Tree * i2 = base.nDigit==1
        ? stack->pushSmallInt(base.digits[0], base.neg)
        : stack->pushBigInt(base.digits, base.nDigit, base.neg);
    Tree * e = exp.nDigit==1
        ? stack->pushSmallInt(exp.digits[0], false)
        : stack->pushBigInt(exp.digits, exp.nDigit, false);
    while (!(e->type()==Type::PosSmallInt && e->asSmallInt()->value==1)){
        bool odd = Integer(e).digits[0] & 1;
        Tree * next1 = i1;
        if (odd){
            next1 = mul(Integer(i1), Integer(i2), stack);
        }
        Tree * next2 = mul(Integer(i2), Integer(i2), stack);
        Tree * nextE = pushHalved(Integer(e), stack);
        uint32_t n1 = next1->treeSize();
        uint32_t n2 = next2->treeSize();
        uint32_t nE = nextE->treeSize();
        memmove(stack->buffer+mark, next1->block(), n1);
        memmove(stack->buffer+mark+n1, next2->block(), n2+nE);
        stack->size = mark+n1+n2+nE;
        i1 = Tree::fromBlock(stack->buffer+mark);
        i2 = Tree::fromBlock(stack->buffer+mark+n1);
        e = Tree::fromBlock(stack->buffer+mark+n1+n2);
    }
    return stack->keepOnly(mark, mul(Integer(i1), Integer(i2), stack));
}
}