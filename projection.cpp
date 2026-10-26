// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tree.hpp"
#include "tree_stack.hpp"
#include <cstdint>

namespace scas {

Tree * Tree::project(TreeStack * stack){
    switch (type()) {
        case Type::Add:
        case Type::Mul:{
            Tree * nAry = stack->pushNAry(type(), asNAry()->nChildren);
            Tree * child = nextNode();
            for (uint8_t i = 0; i<asNAry()->nChildren; i++){
                child->project(stack);
                child = child->nextTree();
            }
            return nAry;
        }
        case Type::Pow:{
            Tree * pow = stack->pushNode(Type::Pow);
            Tree * base = nextNode();
            base->project(stack);
            base->nextTree()->project(stack);
            return pow;
        }
        case Type::Opposite:{
            Tree * mul = stack->pushNAry(Type::Mul, 2);
            stack->pushSmallInt(1, true);
            nextNode()->project(stack);
            return mul;
        }
        case Type::Sub:{
            Tree * add = stack->pushNAry(Type::Add, 2);
            Tree * child = nextNode();
            child->project(stack);
            stack->pushNAry(Type::Mul, 2);
            stack->pushSmallInt(1, true);
            child->nextTree()->project(stack);
            return add;
        }
        case Type::Div:{
            Tree * mul = stack->pushNAry(Type::Mul, 2);
            Tree * child = nextNode();
            child->project(stack);
            stack->pushNode(Type::Pow);
            child->nextTree()->project(stack);
            stack->pushSmallInt(1, true);
            return mul;
        }
        case Type::Parentheses:
            return nextNode()->project(stack);
        default:
            return stack->pushTree(this);
    }
}

}