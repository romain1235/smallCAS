// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iostream"
#include "tree_stack.hpp"
#include "parser.hpp"
#include <cstdint>

using namespace scas;

uint8_t buffer[1024*1024];

int main() {
    TreeStack stack(buffer, sizeof(buffer));
    char out[1024];
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        Tree * inputTree = parse(&stack, input.c_str()).result;
        if (inputTree == nullptr){
            std::cout << "error" << std::endl;       
        }else{
            inputTree->serialize(out, sizeof(out));
            std::cout << out << " = ";
            Tree * projected = inputTree->project(&stack);
            Tree * reduced = projected->simpleReduction(&stack);
            Tree * beautified = reduced->beautify(&stack);
            beautified->serialize(out, sizeof(out));
            std::cout << out << std::endl;
        }
    }
}