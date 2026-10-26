// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkpoint.hpp"
#include <cstdint>

namespace scas {

Checkpoint * Checkpoint::active = nullptr;
Error Checkpoint::error;
uint8_t Checkpoint::data;

Checkpoint::Checkpoint(TreeStack * stack)
    : stack(stack), stackSize(stack->size), parent(active){active=this;}
Checkpoint::~Checkpoint(){
    active = parent;
}
void Checkpoint::raise(Error error, uint8_t data){
    Checkpoint * checkpoint = active;
    active = checkpoint->parent;
    Checkpoint::error = error;
    Checkpoint::data = data;
    checkpoint->stack->size = checkpoint->stackSize;
    longjmp(checkpoint->env, 1);
}
}