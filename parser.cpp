#pragma once
#include "parser.hpp"
#include "checkpoint.hpp"
#include <csetjmp>
#include <cstdint>

namespace scas {

namespace{

class Parser{
public:
    const char * volatile p;
    TreeStack * stack;
    Tree * pushAdd(){
        uint32_t mark = stack->size;
        Tree * add = stack->pushNAry(Type::Add, 0);
        Tree * first = pushUnary(true);
        uint8_t nChildren = 1;
        while (true){
            if (eat('+')){
                pushUnary(true);
                nChildren++;
            }else if(eat('-')){
                if (nChildren>1){
                    add->asNAry()->nChildren=nChildren;
                    stack->keepOnly(mark+3, add);
                }else{
                    stack->keepOnly(mark+3, first);
                }
                stack->buffer[mark+2]=static_cast<uint8_t>(Type::Sub);
                pushUnary(true);
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
                    mul?pushMulAndDiv():pushPow();
                    return opposite;
                }
                return mul?pushMulAndDiv():pushPow();
            }
        }
    }
    Tree * pushMulAndDiv(){
        uint32_t mark = stack->size;
        Tree * mul = stack->pushNAry(Type::Mul, 0);
        Tree * first = pushPow();
        uint8_t nChildren = 1;
        while (true){
            if (eat('*') || canJuxtapose()){
                pushUnary(false);
                nChildren++;
            }else if(eat('/')){
                if (nChildren>1){
                    mul->asNAry()->nChildren=nChildren;
                    stack->keepOnly(mark+3, mul);
                }else{
                    stack->keepOnly(mark+3, first);
                }
                stack->buffer[mark+2]=static_cast<uint8_t>(Type::Div);
                pushUnary(false);
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
        Tree * base = pushPrimary();
        if (eat('^')){
            pushUnary(false);
            return pow;
        }else{
            return stack->keepOnly(mark, base);
        }
    }
    Tree * pushPrimary(){
        if (eat('(')){
            Tree * parentheses = stack->pushNode(Type::Parentheses);
            pushAdd();
            if (eat(')')){
                return parentheses;
            }
            Checkpoint::raise(Error::SyntaxError, static_cast<uint8_t>(SyntaxError::MissingParenthesis));
        }
        if (*p>='0'&&*p<='9'){
            return pushNumber();
        }
        if ((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')){
            return pushSymbol();
        }
        Checkpoint::raise(Error::SyntaxError, static_cast<uint8_t>(SyntaxError::IncompleteExpression));
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
                stack->pushBlock(carry);
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
        uint8_t nChar = 0;
        while ((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')){
            stack->pushBlock(*(p++));
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

ParseReturn parse(TreeStack * stack, const char * text){
    uint32_t mark = stack->size;
    Parser parser;
    parser.stack = stack;
    parser.p = text;
    Checkpoint checkpoint(stack);
    if (setjmp(checkpoint.env)==0){
        Tree * result = parser.pushAdd();
        parser.skip();
        if (*parser.p!='\0'){
            Checkpoint::raise(Error::SyntaxError, static_cast<uint8_t>(SyntaxError::IncompleteExpression));
        }
        return {result, SyntaxError::None, nullptr};
    }else{
        if (Checkpoint::error == Error::SyntaxError){
            return {nullptr, static_cast<SyntaxError>(Checkpoint::data), parser.p};
        }else{
            Checkpoint::raise(Checkpoint::error);
        }
    }
}

}