#pragma once
#include <cstdint>
#include "tree.hpp"

namespace scas {

class TreeRef;

class TreeStack {

public:
    uint8_t * buffer;
    uint32_t bufferSize;
    uint32_t size;
    TreeStack(uint8_t * buffer, uint32_t bufferSize);
    Tree * treeAtId(uint16_t id);
    TreeRef newTreeRef(Tree * tree);
    void deleteTreeRef(uint16_t id);
    uint16_t duplicateRef(uint16_t id);
    void deleteTree(Tree * tree);
private:
    static constexpr uint16_t kMaxRefs = 256;
    uint32_t refs[kMaxRefs];// -1=not used
};

}