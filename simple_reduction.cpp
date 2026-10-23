#include "tree.hpp"
#include "tree_stack.hpp"
#include "k_tree.hpp"
#include "rational.hpp"
#include "checkpoint.hpp"
#include <csetjmp>
#include <cstdint>
#include <cstring>

namespace scas {

void reverse(uint8_t * start, uint8_t * end){
    while (start<end){
        end--;
        uint8_t temp = *start;
        *start = *end;
        *end = temp;
        start++;
    }
}

void moveTree(Tree * tree, Tree * newPos){
    uint8_t * start = newPos->block();
    uint8_t * mid = tree->block();
    uint8_t * end = tree->nextTree()->block();
    reverse(start, mid);
    reverse(mid, end);
    reverse(start, end);
}

uint8_t rank(Type t){
    switch (t) {
        case Type::PosSmallInt:
        case Type::NegSmallInt:
        case Type::PosBigInt:
        case Type::NegBigInt:
        case Type::PosSmallRational:
        case Type::NegSmallRational:
        case Type::PosBigRational:
        case Type::NegBigRational:
            return 0;
        case Type::Mul:
            return 1;
        case Type::Pow:
            return 2;
        case Type::Add:
            return 3;
        case Type::Symbol:
            return 4;
        default:
            return 5;
    }
}

int8_t compare(Tree * a, Tree * b, TreeStack * stack){
    uint8_t rankA = rank(a->type());
    uint8_t rankB = rank(b->type());
    if (rankA > rankB){
        return -compare(b, a, stack);
    }
    if (rankA<rankB){
        if(a->type()==Type::Pow){
            int8_t c = compare(a->nextNode(), b, stack);
            if (c!=0){return c;}
            return compare(a->nextNode()->nextTree(), KSmallInt<1>, stack);
        }
        if (a->type()==Type::Add || a->type()==Type::Mul){
            int8_t c = compare(a->child(a->numberOfChildren()-1), b, stack);
            return c<0 ? -1 : 1;
        }
        return -1;
    }
    if (rankA==0){
        return Rational::compare(a, b, stack);
    }
    if (a->type()==Type::Symbol){
        int8_t i = 0;
        while (true){
            bool endA = i >= a->asSymbol()->nChar;
            bool endB = i >= b->asSymbol()->nChar;
            if (endA && endB){return 0;}
            if (endA){return -1;}
            if (endB){return 1;}
            if (a->asSymbol()->name[i] < b->asSymbol()->name[i]){return -1;}
            if (a->asSymbol()->name[i] > b->asSymbol()->name[i]){return 1;}
            i++;
        }
    }
    if (a->type()==Type::Pow){
        int8_t c = compare(a->nextNode(), b->nextNode(), stack);
        if (c!=0){return c;}
        return compare(a->nextNode()->nextTree(), b->nextNode()->nextTree(), stack);
    }
    if (a->type()==Type::Add || a->type()==Type::Mul){
        uint8_t nA = a->numberOfChildren();
        uint8_t nB = b->numberOfChildren();
        uint8_t n = nA < nB ? nA : nB;
        for (uint8_t i = 1; i<=n; i++){
            int8_t c = compare(a->child(nA-i), b->child(nB-i), stack);
            if (c!=0){return c;}
        }
        if (nA==nB){return 0;}
        return nA<nB ? -1 : 1;
    }
    return 0;
}

Tree * Tree::simpleReduction(TreeStack *stack){
    switch (type()) {
        case Type::Add:
        case Type::Mul:{
            uint32_t mark = stack->size;
            Tree * nAry = stack->pushNAry(type(), 0);
            uint8_t nChildren = asNAry()->nChildren;
            uint8_t nAddedChildren = 0;
            Tree * child = nextNode();
            while (nChildren>0) {
                uint32_t childMark = stack->size;
                Tree * newChild = child->simpleReduction(stack);
                child = child->nextTree();
                nChildren--;
                if (newChild->type()==type()){
                    nAddedChildren+=newChild->asNAry()->nChildren;
                    memmove(newChild->block(), newChild->block()+sizeof(NAry), newChild->treeSize()-sizeof(NAry));
                    stack->size-=sizeof(NAry);
                }else{
                    nAddedChildren+=1;
                    if(type()==Type::Add){
                        if (newChild->type()==Type::PosSmallInt && newChild->asSmallInt()->value==0){
                            stack->trimTo(childMark);
                            nAddedChildren-=1;
                        }
                    }else{
                        if (newChild->type()==Type::PosSmallInt && newChild->asSmallInt()->value==0){
                            stack->trimTo(mark);
                            return stack->pushSmallInt(0, false);
                        }else if(newChild->type()==Type::PosSmallInt && newChild->asSmallInt()->value==1){
                            stack->trimTo(childMark);
                            nAddedChildren-=1;
                        }
                    }
                }
            }
            if (nAddedChildren == 1){
                return stack->keepOnly(mark, nAry->nextNode());
            }else if (nAddedChildren == 0){
                stack->trimTo(mark);
                if (type()==Type::Add){
                    return stack->pushSmallInt(0, false);
                }else{
                    return stack->pushSmallInt(1, false);
                }
            }
            nAry->asNAry()->nChildren = nAddedChildren;
            uint8_t nSortedChild = 0;
            Tree * firstChild = nAry->nextNode();
            child = firstChild;
            for (uint8_t i = 0; i<nAddedChildren; i++){
                uint8_t newPos = 0;
                Tree * compared = firstChild;
                for (uint8_t j = 0; j<nSortedChild; j++){
                    if (compare(compared, child, stack)<0){
                        compared = compared->nextTree();
                        newPos++;
                    }else{
                        break;;
                    }
                }
                Tree * nextChild = child->nextTree();
                if (newPos!=i){
                    moveTree(child, compared);
                }
                child = nextChild;
                nSortedChild++;
            }
            Tree * newNAry = stack->pushNAry(type(), 0);
            nAddedChildren = 1;
            child = firstChild;
            uint32_t childMark = stack->size;
            Tree * lastAddedChild = stack->pushTree(child);
            child = child->nextTree();
            for (uint8_t i = 1; i<nAry->asNAry()->nChildren; i++){
                if (lastAddedChild==nullptr){
                    childMark = stack->size;
                    lastAddedChild = stack->pushTree(child);
                    child = child->nextTree();
                    nAddedChildren=1;
                    continue;
                }
                if (lastAddedChild->isNumber() && child->isNumber()){
                    if (type()==Type::Mul){
                        lastAddedChild = stack->keepOnly(childMark, Rational::mul(lastAddedChild, child, stack));
                    }else{
                        lastAddedChild = stack->keepOnly(childMark, Rational::add(lastAddedChild, child, stack));
                        if (lastAddedChild->type()==Type::PosSmallInt && lastAddedChild->asSmallInt()->value==0){
                            nAddedChildren-=1;
                            Tree * first = newNAry->nextNode();
                            if (first->block()!=stack->buffer+childMark){
                                Tree * previous = first;
                                while (previous->nextTree()->block()!=stack->buffer+childMark){
                                    previous = previous->nextTree();
                                }
                                stack->trimTo(childMark);
                                lastAddedChild = previous;
                                childMark = previous->block()-stack->buffer;
                            }else{
                                stack->trimTo(childMark);
                                lastAddedChild = nullptr;
                            }
                        }
                    }
                    child = child->nextTree();
                    continue;
                }
                Type aType = lastAddedChild->type();
                Type bType = child->type();
                if (type()==Type::Mul){
                    Tree * aBase = aType==Type::Pow?lastAddedChild->nextNode():lastAddedChild;
                    Tree * aExp = aType==Type::Pow?aBase->nextTree():KSmallInt<1>;
                    Tree * bBase = bType==Type::Pow?child->nextNode():child;
                    Tree * bExp = bType==Type::Pow?bBase->nextTree():KSmallInt<1>;
                    if (aBase->isIdenticalTo(bBase)){
                        Tree * newChild = stack->pushNode(Type::Pow);
                        stack->pushTree(aBase);
                        Tree * exp = stack->pushNAry(Type::Add, 2);
                        stack->pushTree(aExp);
                        stack->pushTree(bExp);
                        lastAddedChild = stack->keepOnly(childMark, newChild->simpleReduction(stack));
                        if (lastAddedChild->type()==Type::PosSmallInt && lastAddedChild->asSmallInt()->value==1){
                            nAddedChildren-=1;
                            Tree * first = newNAry->nextNode();
                            if (first->block()!=stack->buffer+childMark){
                                Tree * previous = first;
                                while (previous->nextTree()->block()!=stack->buffer+childMark){
                                    previous = previous->nextTree();
                                }
                                stack->trimTo(childMark);
                                lastAddedChild = previous;
                                childMark = previous->block()-stack->buffer;
                            }else{
                                stack->trimTo(childMark);
                                lastAddedChild = nullptr;
                            }
                        }
                        child = child->nextTree();
                        continue;
                    }
                }
                if (type()==Type::Add){
                    Tree * aFirstChild;
                    uint8_t aNChildren;
                    Tree * aCoef;
                    if (aType==Type::Mul){
                        aFirstChild = lastAddedChild->nextNode();
                        aNChildren = lastAddedChild->numberOfChildren();
                        if (aFirstChild->isNumber()){
                            aCoef = aFirstChild;
                            aFirstChild = aFirstChild->nextTree();
                            aNChildren-=1;
                        }else{
                            aCoef = KSmallInt<1>;
                        }
                    }else{
                        aFirstChild = lastAddedChild;
                        aNChildren = 1;
                        aCoef = KSmallInt<1>;
                    }
                    Tree * bFirstChild;
                    uint8_t bNChildren;
                    Tree * bCoef;
                    if (bType==Type::Mul){
                        bFirstChild = child->nextNode();
                        bNChildren = child->numberOfChildren();
                        if (bFirstChild->isNumber()){
                            bCoef = bFirstChild;
                            bFirstChild = bFirstChild->nextTree();
                            bNChildren-=1;
                        }else{
                            bCoef = KSmallInt<1>;
                        }
                    }else{
                        bFirstChild = child;
                        bNChildren = 1;
                        bCoef = KSmallInt<1>;
                    }
                    if (aNChildren == bNChildren){
                        bool canAdd = true;
                        Tree * aChild = aFirstChild;
                        Tree * bChild = bFirstChild;
                        for (uint8_t i = 0; i<aNChildren; i++){
                            if (aChild->isIdenticalTo(bChild)){
                                aChild = aChild->nextTree();
                                bChild = bChild->nextTree();
                                continue;
                            }
                            canAdd = false;
                            break;
                        }
                        if (canAdd){
                            Tree * newChild = stack->pushNAry(Type::Mul, aNChildren+1);
                            stack->pushNAry(Type::Add, 2);
                            stack->pushTree(aCoef);
                            stack->pushTree(bCoef);
                            Tree * aChild = aFirstChild;
                            for (uint8_t i = 0; i<aNChildren; i++){
                                stack->pushTree(aChild);
                                aChild = aChild->nextTree();
                            }
                            lastAddedChild = stack->keepOnly(childMark, newChild->simpleReduction(stack));
                            if (lastAddedChild->type()==Type::PosSmallInt && lastAddedChild->asSmallInt()->value==0){
                                nAddedChildren-=1;
                                Tree * first = newNAry->nextNode();
                                if (first->block()!=stack->buffer+childMark){
                                    Tree * previous = first;
                                    while (previous->nextTree()->block()!=stack->buffer+childMark){
                                        previous = previous->nextTree();
                                    }
                                    stack->trimTo(childMark);
                                    lastAddedChild = previous;
                                    childMark = previous->block()-stack->buffer;
                                }else{
                                    stack->trimTo(childMark);
                                    lastAddedChild = nullptr;
                                }
                            }
                            child = child->nextTree();
                            continue;
                        }
                    }
                }
                childMark = stack->size;
                lastAddedChild = stack->pushTree(child);
                child = child->nextTree();
                nAddedChildren+=1;
            }
            if (lastAddedChild==nullptr){
                stack->trimTo(mark);
                if (type()==Type::Add){
                    return stack->pushSmallInt(0, false);
                }else{
                    return stack->pushSmallInt(1, false);
                }
            }
            if (nAddedChildren==1){
                return stack->keepOnly(mark, newNAry->nextNode());
            }
            newNAry->asNAry()->nChildren = nAddedChildren;
            return stack->keepOnly(mark, newNAry);
        }
        case Type::Pow:{
            uint32_t mark = stack->size;
            Tree * base = nextNode();
            Tree * exp = base->nextTree();
            Tree * pow = stack->pushNode(Type::Pow);
            base = base->simpleReduction(stack);
            exp = exp->simpleReduction(stack);
            if (base->type()==Type::PosSmallInt && base->asSmallInt()->value == 1){
                return stack->keepOnly(mark, stack->pushSmallInt(1, false));
            }
            if (exp->isInteger()){
                if (exp->type() == Type::PosSmallInt && exp->asSmallInt()->value == 0){
                    if (base->type()==Type::PosSmallInt && base->asSmallInt()->value == 0){
                        return stack->keepOnly(mark, stack->pushNode(Type::Undef));
                    }
                    return stack->keepOnly(mark, stack->pushSmallInt(1, false));
                }else if (exp->type() == Type::PosSmallInt && exp->asSmallInt()->value == 1){
                    return stack->keepOnly(mark, base);
                }
                if (base->type()==Type::PosSmallInt && base->asSmallInt()->value==0 && Integer(exp).neg){
                    return stack->keepOnly(mark, stack->pushNode(Type::Undef));
                }
                if (base->isNumber()){
                    Checkpoint checkpoint(stack);
                    if (setjmp(checkpoint.env)==0){
                        return stack->keepOnly(mark, Rational::pow(base, exp, stack));
                    }else{
                        if (Checkpoint::error!=Error::IntOverflow){
                            Checkpoint::raise(Checkpoint::error);
                        }
                    }
                }
            }
            return pow;
        }
        default:
            return stack->pushTree(this);
    }
}

}