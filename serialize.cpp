// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tree.hpp"
#include "serialize.hpp"
#include <cstdint>

namespace scas {

char op(Type t){
    switch (t) {
        case Type::Mul:
            return '*';
        case Type::Div:
            return '/';
        case Type::Sub:
            return '-';
        case Type::Add:
            return '+';
        case Type::Pow:
            return '^';
        default:
            return '\0';
    }
}

void pushChar(char * out, uint16_t size, uint16_t & p, char c){
    if (p+1<size){
        out[p++] = c;
    }
}

LayoutShape Tree::layoutShape(bool left, bool layout){
    if (hasParentheses(2)){
        return LayoutShape::Layout;
    }
    switch (type()) {
        case Type::Parentheses:
            return LayoutShape::Layout;
        case Type::Symbol:
        case Type::Undef:
            return LayoutShape::Letters;
        case Type::NegBigInt:
        case Type::PosBigInt:
        case Type::NegSmallInt:
        case Type::PosSmallInt:
            return LayoutShape::Number;
        case Type::Div:
            return layout?LayoutShape::Layout:LayoutShape::Operation;
        case Type::Mul:{
            return LayoutShape::Operation;
        }
        case Type::Pow:
            return left?LayoutShape::Operation:nextNode()->layoutShape(false, layout);
    }
}

bool canJuxtapose(Tree * left, Tree * right, bool layout){
    LayoutShape leftShape = left->layoutShape(true, layout);
    LayoutShape rightShape = right->layoutShape(false, layout);
    if (leftShape==LayoutShape::Layout){
        return true;
    }
    if (rightShape==LayoutShape::Layout){
        return true;
    }
    switch (leftShape) {
        case LayoutShape::Number:
            switch (rightShape) {
                case LayoutShape::Letters:
                    return true;
                case LayoutShape::Number:
                case LayoutShape::Operation:
                    return false;
            }
        case LayoutShape::Letters:
        case LayoutShape::Operation:
            return false;
    }
}

bool Tree::hasParentheses(uint8_t parentPriority){
    uint8_t childPriority;
    switch (type()) {
        case Type::Sub:
        case Type::Opposite:
            childPriority = 0;
            break;
        case Type::Add:
            childPriority = 1;
            break;
        case Type::Div:
            childPriority = 2;
            break;
        case Type::Mul:
            childPriority = 3;
            break;
        case Type::Pow:
            childPriority = 4;
            break;
        default:
            childPriority = 5;
            break;
    }
    return parentPriority > childPriority;
}

void writeInt(uint8_t * digits, uint8_t nDigit, char * out, uint16_t size, uint16_t & p){
    char * start = out+p;
    uint16_t nOutputDigit = 0;
    for (uint8_t i = nDigit; i>0; i--){
        uint16_t carry = digits[i-1];
        for (uint16_t j = 0; j<nOutputDigit; j++){
            uint16_t nextValue = ((start[j]-'0')<<8)+carry;
            start[j] = (nextValue%10)+'0';
            carry = nextValue/10;
        }
        while (carry!=0) {
            pushChar(out, size, p, '0'+carry%10);
            if (p+1>=size){return;}
            carry/=10;
            nOutputDigit++;
        }
    }
    if (nOutputDigit == 0){
        pushChar(out, size, p, '0');
        return;
    }
    for (uint16_t i = 0; i<nOutputDigit/2; i++){
        char temp = start[i];
        start[i] = start[nOutputDigit-i-1];
        start[nOutputDigit-i-1] = temp;
    }
}

void Tree::serialize(char * out, uint16_t size, uint16_t & p){
    switch (type()) {
        case Type::Symbol:
            for (uint8_t i = 0; i<asSymbol()->nChar; i++){
                pushChar(out, size, p, asSymbol()->name[i]);
            }
            break;
        case Type::Undef:{
            const char text[] = "undef";
            for (uint8_t i = 0; i<sizeof(text)-1; i++){
                pushChar(out, size, p, text[i]);
            }
            break;
        }
        case Type::NegBigInt:
        case Type::PosBigInt:
            if (type()==Type::NegBigInt){pushChar(out, size, p, '-');}
            writeInt(asBigInt()->digits, asBigInt()->nDigit, out, size, p);
            break;
        case Type::NegSmallInt:
        case Type::PosSmallInt:
            if (type()==Type::NegSmallInt){pushChar(out, size, p, '-');}
            writeInt(&asSmallInt()->value, 1, out, size, p);
            break;
        case Type::Opposite:{
            pushChar(out, size, p, '-');
        }
        default:
            Tree * child = nextNode();
            uint8_t n = numberOfChildren();
            for (uint8_t i = 0 ; i<n; i++){
                uint8_t parentPriority;
                switch (type()) {
                    case Type::Add:
                        parentPriority = i==0 ? 0 : 1;
                        break;
                    case Type::Sub:
                        parentPriority = i==0 ? 0 : 2;
                        break;
                    case Type::Div:
                        parentPriority = i==0 ? 2 : 4;
                        break;
                    case Type::Mul:
                    case Type::Opposite:
                        parentPriority = 2;
                        break;
                    case Type::Pow:
                        parentPriority = 5;
                        break;
                    default:
                        parentPriority = 6;
                        break;
                }
                bool parentheses = child->hasParentheses(parentPriority);
                if (parentheses){pushChar(out, size, p, '(');}
                child->serialize(out, size, p);
                if (parentheses){pushChar(out, size, p, ')');}
                if (i!=n-1){
                    if (!(type()==Type::Mul && canJuxtapose(child, child->nextTree(), false))){
                        pushChar(out, size, p, op(type()));
                    }
                }
                child = child->nextTree();
            }
    }
    if (p>=size){
        out[p-1]='\0';
    }else{
        out[p]='\0';
    }
}

void Tree::serialize(char *out, uint16_t size){
    uint16_t p = 0;
    serialize(out, size, p);
}

}