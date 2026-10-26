#include "tree.hpp"
#include "k_tree.hpp"
#include "parser.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

uint8_t buffer[1024*1024];

using namespace scas;


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
        case scas::Type::Undef:
            output+="Undef(";
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

struct serializeTest{
    Tree * tree;
    const char * expectedOutput;
};

serializeTest kSerializeTest[] = {
    {KSymbol<'x'>, "x"},
    {KSymbol<'x','2'>, "x2"},
    {KSmallInt<245>, "245"},
    {KSmallInt<0>, "0"},
    {KNegSmallInt<245>, "-245"},
    {KBigInt<255,255>, "65535"},
    {KBigInt<254,255>, "65534"},
    {KNegBigInt<255,255>, "-65535"},
    {KMul(KSmallInt<245>,KSymbol<'x'>), "245x"},
    {KMul(KSymbol<'x'>,KSmallInt<245>), "x*245"},
    {KMul(KAdd(KSymbol<'x'>,KSymbol<'x'>),KAdd(KSymbol<'x'>,KSymbol<'x'>)),"(x+x)(x+x)"},
    {KAdd(KOpposite(KSmallInt<5>),KSmallInt<5>),"-5+5"},
    {KMul(KOpposite(KSmallInt<5>),KSmallInt<5>),"(-5)5"},
    {KOpposite(KMul(KSmallInt<5>,KSmallInt<5>)),"-5*5"},
    {KAdd(KSmallInt<5>,KOpposite(KSmallInt<5>)),"5+(-5)"},
    {KAdd(KSmallInt<5>,KMul(KOpposite(KSmallInt<5>),KSmallInt<5>)),"5+(-5)5"},
};

constexpr int KNumberOfSerializeTest = sizeof(kSerializeTest)/sizeof(serializeTest);

struct parseTest{
    const char * input;
    Tree * tree;
};

parseTest kParseTest[] = {
    {"-5*-5", KOpposite(KMul(KSmallInt<5>,KOpposite(KSmallInt<5>)))},
};

constexpr int KNumberOfParseTest = sizeof(kParseTest)/sizeof(parseTest);

bool sameTree(Tree * a, Tree * b){
    uint32_t n = a->treeSize();
    return n==b->treeSize() && std::memcmp(a->block(), b->block(), n)==0;
}

int main() {
    for (int i = 0; i<KNumberOfSerializeTest; i++){
        char out[1024];
        kSerializeTest[i].tree->serialize(out, 1024);
        std::string output;
        serializeTree(kSerializeTest[i].tree, output);
        if (std::strcmp(out, kSerializeTest[i].expectedOutput)==0){
            std::cout << output << " -> " << out << std::endl; 
        }else{
            std::cout << "\033[31m" << output << " made " << out << " expected " << kSerializeTest[i].expectedOutput << "\033[0m" << std::endl;
        }
    }
    TreeStack stack(buffer, sizeof(buffer));
    for (int i = 0; i<KNumberOfParseTest; i++){
        stack.trimTo(0);
        Tree * got = parse(&stack, kParseTest[i].input).result;
        std::string output;
        serializeTree(got, output);
        if (got!=nullptr && sameTree(got, kParseTest[i].tree)){
            std::cout << kParseTest[i].input << " -> " << output << std::endl;
        }else{
            std::string expected;
            serializeTree(kParseTest[i].tree, expected);
            std::cout << "\033[31m" << kParseTest[i].input << " made " << output << " expected " << expected << "\033[0m" << std::endl;
        }
    }
    return 0;
}
