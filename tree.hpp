#pragma once
#include <cstdint>

namespace scas {

class Tree{
public:
    uint16_t headerSize(){return 1;};//test
    uint8_t numberOfChildren(){return 1;};//test
    Tree * nextNode();
    Tree * nextTree();
    uint32_t treeSize();
    uint8_t * block(){return reinterpret_cast<uint8_t *>(this);}
    static Tree * fromBlock(uint8_t * b) {
        return reinterpret_cast<Tree *>(b);
    }
};
}