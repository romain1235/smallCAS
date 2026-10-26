// Copyright (C) 2026 Romain Nardi
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cstdint>
#include "type.hpp"

namespace scas {

enum class LayoutShape: uint8_t{
    Letters,
    Number,
    Layout,//fraction, parentheses, root, ...
    Operation,
};

char op(Type t);

}