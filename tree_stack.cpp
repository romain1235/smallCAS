#include "tree_stack.hpp"
#include "tree_ref.hpp"
#include <cstdint>
#include <cstring>

namespace scas {

TreeStack::TreeStack(uint8_t * buffer, uint32_t bufferSize)
    : buffer(buffer), bufferSize(bufferSize), size(0) {
        for (uint16_t i = 0; i < kMaxRefs; i++){
            refs[i]=-1;
        }
    }

Tree * TreeStack::treeAtId(uint16_t id){
    if (refs[id]==-1){return nullptr;}
    if (refs[id]==-2){return nullptr;}
    return Tree::fromBlock(buffer+refs[id]);
}
TreeRef TreeStack::newTreeRef(Tree * tree){
    for (uint16_t i = 0; i < kMaxRefs; i++){
        if (refs[i]==-1){
            refs[i] = tree->block()-buffer;
            return TreeRef(i,this);
        }
    }
    return TreeRef(static_cast<uint16_t>(-1), nullptr);
}
void TreeStack::deleteTreeRef(uint16_t id){
    refs[id] = -1;
}
uint16_t TreeStack::duplicateRef(uint16_t id){
    if (id == static_cast<uint16_t>(-1)) {
        return id;
    }
    uint32_t offset = refs[id];
    for (uint16_t i = 0; i < kMaxRefs; i++) {
        if (refs[i] == static_cast<uint32_t>(-1)) {
            refs[i] = offset;
            return i;
        }
    }
    return static_cast<uint16_t>(-1);
}
void TreeStack::deleteTree(Tree *tree){
    uint8_t * start = tree->block();
    uint32_t n = tree->treeSize();
    uint32_t offset = start - buffer;
    memmove(start, start+n, size-offset-n);
    size-=n;
    for(uint16_t i = 0; i<kMaxRefs; i++){
        if (refs[i]==-1){continue;}
        if (refs[i]==-2){continue;}
        if(refs[i]>=offset+n){
            refs[i]-=n;
        }else if (refs[i]>=offset){
            refs[i]=-2;
        }
    }
}
}