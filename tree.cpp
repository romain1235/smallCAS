#include "tree.hpp"
#include <cstdint>

namespace scas {

Tree * Tree::nextNode(){
    return fromBlock(block()+headerSize());
}
Tree * Tree::nextTree(){
    Tree * t = nextNode();
    uint32_t remaining = numberOfChildren();
    while (remaining > 0) {
      remaining += t->numberOfChildren();
      t = t->nextNode();
      remaining--;
    }
    return t;
}
uint32_t Tree::treeSize(){
    return nextTree()->block()-block();
}

}