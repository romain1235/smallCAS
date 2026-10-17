#include "parser.hpp"
#include <cstdint>
#include <iostream>
#include <string>

uint8_t buffer[1024*1024];

void serializeTree(scas::Tree * tree, std::string & output){
    if (tree == nullptr){
        output = "invalid tree";
        return;
    }
    switch (tree->type()) {
        case scas::Type::Add:
            output+="Add(";
            break;
        case scas::Type::Sub:
            output+="Sub(";
            break;
        case scas::Type::Mul:
            output+="Mul(";
            break;
        case scas::Type::Div:
            output+="Div(";
            break;
        case scas::Type::Pow:
            output+="Pow(";
            break;
        case scas::Type::Opposite:
            output+="Opposite(";
            break;
        case scas::Type::NegSmallInt:
        case scas::Type::PosSmallInt:
            output += "Number(";
            if (tree->type() == scas::Type::NegSmallInt) {
                output += "-";
            }
            output += std::to_string(tree->asSmallInt()->value);
            break;
        case scas::Type::NegBigInt:
        case scas::Type::PosBigInt: {
            output += "Number(";
            if (tree->type() == scas::Type::NegBigInt) {
                output += "-";
            }
            scas::BigInt* number = tree->asBigInt();
            uint8_t digits[256];
            uint8_t nDigit = number->nDigit;
            for (uint8_t i = 0; i < nDigit; i++) {
                digits[i] = number->digits[i];
            }
            std::string decimal;
            while (nDigit > 0) {
                uint16_t rest = 0;
                for (int i = nDigit - 1; i >= 0; i--) {
                    uint16_t current = (rest << 8) | digits[i];
                    digits[i] = current / 10;
                    rest = current % 10;
                }
                decimal.push_back('0' + rest);
                while (nDigit > 0 && digits[nDigit - 1] == 0) {
                    nDigit--;
                }
            }
            for (int i = decimal.size() - 1; i >= 0; i--) {
                output.push_back(decimal[i]);
            }
            break;
        }
        case scas::Type::Parentheses:
            output+="Parentheses(";
            break;
        case scas::Type::Symbol:
            output+="Symbol(";
            output.append(tree->asSymbol()->name, tree->asSymbol()->nChar);
            break;
    }
    scas::Tree * child = tree->nextNode();
    for (uint8_t i = 0;i<tree->numberOfChildren(); i++){
        serializeTree(child, output);
        if (i!=tree->numberOfChildren()-1){output+=",";}
        child = child->nextTree();
    }
    output+=")";
}

int main() {
    std::string text;
    std::getline(std::cin, text);
    scas::TreeStack stack(buffer,1024*1024);
    scas::Tree * outputTree = scas::parse(&stack, text.c_str());
    std::string output;
    serializeTree(outputTree, output);
    std::cout << output << std::endl;
    return 0;
}
