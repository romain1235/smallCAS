#include "tree.hpp"
#include "tree_stack.hpp"
#include <cstdint>

namespace scas {

Tree * Tree::project(TreeStack * stack){
    switch (type()) {
        case Type::Add:
        case Type::Mul:{
            Tree * nAry = stack->pushNAry(type(), asNAry()->nChildren);
            if (nAry==nullptr){return nullptr;}
            Tree * child = nextNode();
            for (uint8_t i = 0; i<asNAry()->nChildren; i++){
                if (child->project(stack)==nullptr){return nullptr;}
                child = child->nextTree();
            }
            return nAry;
        }
        case Type::Pow:{
            Tree * pow = stack->pushNode(Type::Pow);
            if (pow == nullptr){return nullptr;}
            Tree * base = nextNode();
            if (base->project(stack)==nullptr){return nullptr;}
            if (base->nextTree()->project(stack)==nullptr){return nullptr;}
            return pow;
        }
        case Type::Opposite:{
            Tree * mul = stack->pushNAry(Type::Mul, 2);
            if (mul == nullptr){return nullptr;}
            if (stack->pushSmallInt(1, true)==nullptr){return nullptr;}
            if (nextNode()->project(stack)==nullptr){return nullptr;}
            return mul;
        }
        case Type::Sub:{
            Tree * add = stack->pushNAry(Type::Add, 2);
            if (add==nullptr){return nullptr;}
            Tree * child = nextNode();
            if (child->project(stack)==nullptr){return nullptr;}
            if (stack->pushNAry(Type::Mul, 2)==nullptr){return nullptr;}
            if (stack->pushSmallInt(1, true)==nullptr){return nullptr;}
            if (child->nextTree()->project(stack)==nullptr){return nullptr;}
            return add;
        }
        case Type::Div:{
            Tree * mul = stack->pushNAry(Type::Mul, 2);
            if (mul == nullptr){return nullptr;}
            Tree * child = nextNode();
            if (child->project(stack)==nullptr){return nullptr;}
            if (stack->pushNode(Type::Pow)==nullptr){return nullptr;}
            if (child->nextTree()->project(stack)==nullptr){return nullptr;}
            if (stack->pushSmallInt(1, true)==nullptr){return nullptr;}
            return mul;
        }
        case Type::Parentheses:
            return nextNode()->project(stack);
        default:
            return stack->pushTree(this);
    }
}

}