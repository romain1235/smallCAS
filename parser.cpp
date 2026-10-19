#pragma once
#include "parser.hpp"
#include <cstdint>

namespace scas {

namespace{

class Parser{
public:
    const char * p;
    TreeStack * stack;
    Tree * pushAdd(){
        uint32_t mark = stack->size;
        Tree * add = stack->pushAdd(0);
        if (add == nullptr){return nullptr;}
        Tree * first = pushUnary(true);
        if (first == nullptr){return nullptr;}
        uint8_t nChildren = 1;
        while (true){
            if (eat('+')){
                if (pushUnary(true)==nullptr){return nullptr;}
                nChildren++;
            }else if(eat('-')){
                if (nChildren>1){
                    add->asNAry()->nChildren=nChildren;
                    if (stack->keepOnly(mark+3, add)==nullptr){return nullptr;};
                }else{
                    if (stack->keepOnly(mark+3, first)==nullptr){return nullptr;}
                }
                stack->buffer[mark+2]=static_cast<uint8_t>(Type::Sub);
                if (pushUnary(true)==nullptr){return nullptr;}
                nChildren=1;
            }else{
                if (nChildren==1){
                    return stack->keepOnly(mark, first);
                }else{
                    add->asNAry()->nChildren = nChildren;
                    return add;
                }
            }
        }
    }
    bool canJuxtapose(){
        switch (*p) {
            case '/':
            case '+':
            case '-':
            case '\0':
            case ')':
                return false;
            default:
                return true;
        }
    }
    Tree * pushUnary(bool mul){
        bool neg = false;
        while (true){
            if (eat('+')){
                continue;
            }else if(eat('-')){
                neg=!neg;
            }else{
                if (neg){
                    Tree * opposite = stack->pushNode(Type::Opposite);
                    if (opposite==nullptr){return nullptr;}
                    if ((mul?pushMulAndDiv():pushPow())==nullptr){return nullptr;}
                    return opposite;
                }
                return mul?pushMulAndDiv():pushPow();
            }
        }
    }
    Tree * pushMulAndDiv(){
        uint32_t mark = stack->size;
        Tree * mul = stack->pushMul(0);
        if (mul == nullptr){return nullptr;}
        Tree * first = pushPow();
        if (first == nullptr){return nullptr;}
        uint8_t nChildren = 1;
        while (true){
            if (eat('*') || canJuxtapose()){
                if (pushUnary(false)==nullptr){return nullptr;}
                nChildren++;
            }else if(eat('/')){
                if (nChildren>1){
                    mul->asNAry()->nChildren=nChildren;
                    if (stack->keepOnly(mark+3, mul)==nullptr){return nullptr;};
                }else{
                    if (stack->keepOnly(mark+3, first)==nullptr){return nullptr;}
                }
                stack->buffer[mark+2]=static_cast<uint8_t>(Type::Div);
                if (pushUnary(false)==nullptr){return nullptr;}
                nChildren=1;
            }else{
                if (nChildren==1){
                    return stack->keepOnly(mark, first);
                }else{
                    mul->asNAry()->nChildren = nChildren;
                    return mul;
                }
            }
        }
    }
    Tree * pushPow(){
        uint32_t mark = stack->size;
        Tree * pow = stack->pushNode(Type::Pow);
        if (pow==nullptr){return nullptr;}
        Tree * base = pushPrimary();
        if (base==nullptr){return nullptr;}
        if (eat('^')){
            if (pushUnary(false)==nullptr){return nullptr;}
            return pow;
        }else{
            return stack->keepOnly(mark, base);
        }
    }
    Tree * pushPrimary(){
        if (eat('(')){
            Tree * parentheses = stack->pushNode(Type::Parentheses);
            if (pushAdd()==nullptr){return nullptr;}
            if (eat(')')){
                return parentheses;
            }
            return nullptr;
        }
        if (*p>='0'&&*p<='9'){
            return pushNumber();
        }
        if ((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')){
            return pushSymbol();
        }
        return nullptr;
    }
    Tree * pushNumber(){
        uint32_t mark = stack->size;
        Tree * number = stack->pushBigInt(nullptr, 0, false);
        uint8_t nDigit = 0;
        while (*p>='0'&&*p<='9'){
            uint16_t carry = *(p++)-'0';
            for (uint8_t i = 0; i<nDigit; i++){
                uint16_t nextValue = number->asBigInt()->digits[i]*10+carry;
                number->asBigInt()->digits[i]=static_cast<uint8_t>(nextValue);
                carry=nextValue>>8;
            }
            while (carry!=0){
                if(stack->pushBlock(carry)==nullptr){return nullptr;}
                nDigit++;
                carry>>=8;
            }
        }
        if (nDigit<=1){
            uint8_t value;
            if (nDigit==1){
                value = number->asBigInt()->digits[0];
            }else{
                value = 0;
            }
            stack->trimTo(mark);
            return stack->pushSmallInt(value, false);
        }else{
            number->asBigInt()->nDigit = nDigit;
            return number;
        }
    }
    Tree * pushSymbol(){
        Tree * symbol = stack->pushSymbol(nullptr, 0);
        if (symbol==nullptr){return nullptr;}
        uint8_t nChar = 0;
        while ((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')){
            if (stack->pushBlock(*(p++))==nullptr){return nullptr;}
            nChar++;
        }
        symbol->asSymbol()->nChar = nChar;
        return symbol;
    }
    void skip(){
        while (*p==' ' || *p=='\t'){p++;}
    }
    bool eat(char c){
        skip();
        if (*p==c){
            p++;
            return true;
        }
        return false;
    }

};

}

Tree * parse(TreeStack * stack, const char * text){
    uint32_t mark = stack->size;
    Parser parser;
    parser.stack = stack;
    parser.p = text;
    Tree * result = parser.pushAdd();
    parser.skip();
    if (result == nullptr || *parser.p!='\0'){
        stack->trimTo(mark);
        return nullptr;
    }
    return result;
}

}