#include "tree.hpp"
#include "tree_stack.hpp"
#include "rational.hpp"
#include <cstdint>
#include <cstring>

namespace scas {

Tree * Tree::beautify(TreeStack * stack){
    switch (type()) {
        case Type::Mul:
        case Type::Add:{
            uint32_t mark = stack->size;
            uint8_t n = 0;
            Tree * nAry = stack->pushNAry(type(), 0);
            Tree * child = nextNode();
            bool opposite = false;
            for (uint8_t i = 0; i<numberOfChildren(); i++){
                uint32_t childMark = stack->size;
                Tree * newChild = child->beautify(stack);
                n+=1;
                bool insert = false;
                if (type()==Type::Mul && n==1 && newChild->type()==Type::Opposite){
                    Tree * inner = newChild->nextNode();
                    if (inner->type()==Type::PosSmallInt && inner->asSmallInt()->value==1){
                        stack->trimTo(childMark);
                        n = 0;
                        opposite = true;
                    }
                }
                if (type()==Type::Mul && n>1){
                    if (newChild->type()==Type::Div){
                        Tree * numerator = newChild->nextNode();
                        if (numerator->type()==Type::PosSmallInt && numerator->asSmallInt()->value == 1){
                            stack->keepOnly(childMark, numerator->nextTree());
                            insert = true;
                        }
                    }
                }
                if (type() == Type::Add && n>1){
                    if (newChild->type() == Type::Opposite){
                        stack->keepOnly(childMark, newChild->nextNode());
                        insert = true;
                    }
                }
                if (insert){
                    nAry->asNAry()->nChildren = n;
                    if (n==2){
                        uint32_t start = mark + 2;
                        memmove(stack->buffer + mark + 3, stack->buffer + start, stack->size - start);
                        stack->size += 1;
                    }else{
                        stack->keepOnly(mark+3, nAry)->asNAry()->nChildren-=1;
                    }
                    stack->buffer[mark+2] = static_cast<uint8_t>(type()==Type::Mul?Type::Div:Type::Sub);
                    n=1;
                }
                child = child->nextTree();
            }
            if (n==1){
                if (opposite){
                    stack->keepOnly(mark+1, nAry->nextNode());
                    stack->buffer[mark]=static_cast<uint8_t>(Type::Opposite);
                    return fromBlock(stack->buffer+mark);
                }
                return stack->keepOnly(mark, nAry->nextNode());
            }else{
                nAry->asNAry()->nChildren = n;
                if (opposite){
                    stack->keepOnly(mark+1, nAry);
                    stack->buffer[mark]=static_cast<uint8_t>(Type::Opposite);
                    return fromBlock(stack->buffer+mark);
                }
                return nAry;
            }
        }
        case Type::PosSmallInt:
        case Type::NegSmallInt:
        case Type::PosBigInt:
        case Type::NegBigInt:
        case Type::PosSmallRational:
        case Type::NegSmallRational:
        case Type::PosBigRational:
        case Type::NegBigRational:{
            Tree * result = fromBlock(stack->buffer+stack->size);
            if (isNeg()){
                stack->pushNode(Type::Opposite);
            }
            if (isInteger()){
                Tree * integer = stack->pushTree(this);
                integer->block()[0] &= 0b11111110;
            }else{
                stack->pushNode(Type::Div);
                RationalInts parts = Rational(this).toInts();
                if (parts.numerator.nDigit==1){
                    stack->pushSmallInt(parts.numerator.digits[0], false);
                }else{
                    stack->pushBigInt(parts.numerator.digits, parts.numerator.nDigit, false);
                }
                if (parts.denominator.nDigit==1){
                    stack->pushSmallInt(parts.denominator.digits[0], false);
                }else{
                    stack->pushBigInt(parts.denominator.digits, parts.denominator.nDigit, false);
                }
            }
            return result;
        }
        case Type::Pow:{
            Tree * base = nextNode();
            Tree * exp = base->nextTree();
            Tree * result;
            if (exp->isNeg()){
                result = stack->pushNode(Type::Div);
                stack->pushSmallInt(1, false);
                if (exp->type()==Type::NegSmallInt && exp->asSmallInt()->value == 1){
                    base->beautify(stack);
                }else{
                    stack->pushNode(Type::Pow);
                    base->beautify(stack);
                    stack->pushTree(exp)->block()[0] &= 0b11111110;
                }
            }else{
                result = stack->pushNode(Type::Pow);
                base->beautify(stack);
                exp->beautify(stack);
            }
            return result;
        }
        default:
            return stack->pushTree(this);
    }
}

}