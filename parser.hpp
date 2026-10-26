// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "tree_stack.hpp"
#include <cstdint>

namespace scas {

enum class SyntaxError : uint8_t {
    None,
    IncompleteExpression,
    MissingParenthesis,
};

struct ParseReturn{
    Tree * result;
    SyntaxError error;
    const char * errorPos;
};

ParseReturn parse(TreeStack * stack, const char * text);

}