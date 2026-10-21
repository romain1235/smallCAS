#pragma once
#include <csetjmp>
#include <cstdint>
#include "tree_stack.hpp"

namespace scas {

enum class Error: uint8_t{
    StackOverflow,
    IntOverflow,
    SyntaxError,
};

class Checkpoint{
public:
    static Checkpoint * active;
    static Error error;
    static uint8_t data;
    Checkpoint * parent;
    TreeStack * stack;
    uint32_t stackSize;
    jmp_buf env;
    Checkpoint(TreeStack * stack);
    ~Checkpoint();
    [[noreturn]] static void raise(Error error, uint8_t data = 0);
};

}