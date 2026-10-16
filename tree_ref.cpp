#include "tree_ref.hpp"
#include "tree_stack.hpp"
#include <cstdint>

namespace scas {
TreeRef::TreeRef(uint16_t id, TreeStack * stack)
    : id(id), stack(stack){}
TreeRef::~TreeRef(){
    if (id==static_cast<uint16_t>(-1) || stack==nullptr){
        return;
    }
    stack->deleteTreeRef(id);
}
TreeRef::TreeRef(const TreeRef & other){
    stack = other.stack;
    if (stack!=nullptr){
        id = other.stack->duplicateRef(other.id);
    }else {
        id = -1;
    }
}
TreeRef & TreeRef::operator=(const TreeRef & other) {
    if (this == &other) {
        return *this;
    }
    if (id != static_cast<uint16_t>(-1) && stack != nullptr) {
        stack->deleteTreeRef(id);
    }
    stack = other.stack;
    if (stack != nullptr) {
        id = stack->duplicateRef(other.id);
    } else {
        id = static_cast<uint16_t>(-1);
    }
    return *this;
}
Tree * TreeRef::tree(){
    if (id==static_cast<uint16_t>(-1) || stack==nullptr){
        return nullptr;
    }
    return stack->treeAtId(id);
}
}