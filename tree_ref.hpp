#pragma once
#include <cstdint>
#include "tree.hpp"

namespace scas {

class TreeStack;

class TreeRef{
public:
    TreeRef(uint16_t id, TreeStack * stack);
    ~TreeRef();
    TreeRef(const TreeRef & other);
    TreeRef & operator=(const TreeRef & other);
    Tree * tree();
private:
    uint16_t id;
    TreeStack * stack;
};
}