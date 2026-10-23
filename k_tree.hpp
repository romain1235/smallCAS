#pragma once
#include <cstdint>
#include "tree.hpp"

namespace scas {

template<uint8_t... bytes>
struct KTree {
    static constexpr uint16_t size = sizeof...(bytes);
    static constexpr uint8_t block[] = {bytes...};
    operator Tree*() const {return Tree::fromBlock(const_cast<uint8_t*>(block));}
};

namespace kTree {

template<uint8_t... acc>
constexpr KTree<acc...> concat() {
    return {};
}

template<uint8_t... acc, uint8_t... head, typename... tail>
constexpr auto concat(KTree<head...>, tail... rest) {
    return concat<acc..., head...>(rest...);
}

}

template<Type tag>
struct KUnary {
    template<uint8_t... child>
    constexpr auto operator()(KTree<child...>) const {
        return KTree<static_cast<uint8_t>(tag), child...>{};
    }
};

template<Type tag>
struct KBinary {
    template<uint8_t... left, uint8_t... right>
    constexpr auto operator()(KTree<left...>, KTree<right...>) const {
        return KTree<static_cast<uint8_t>(tag), left..., right...>{};
    }
};

template<Type tag>
struct KNAry {
    template<typename... children>
    constexpr auto operator()(children... child) const {
        static_assert(sizeof...(children) <= 255);
        return kTree::concat<static_cast<uint8_t>(tag), static_cast<uint8_t>(sizeof...(children))>(child...);
    }
};

template<uint8_t value>
constexpr KTree<static_cast<uint8_t>(Type::PosSmallInt), value> KSmallInt{};

template<uint8_t value>
constexpr KTree<static_cast<uint8_t>(Type::NegSmallInt), value> KNegSmallInt{};

template<uint8_t... digits>
constexpr KTree<static_cast<uint8_t>(Type::PosBigInt), static_cast<uint8_t>(sizeof...(digits)), digits...> KBigInt{};

template<uint8_t... digits>
constexpr KTree<static_cast<uint8_t>(Type::NegBigInt), static_cast<uint8_t>(sizeof...(digits)), digits...> KNegBigInt{};

template<char... chars>
constexpr KTree<static_cast<uint8_t>(Type::Symbol), static_cast<uint8_t>(sizeof...(chars)), static_cast<uint8_t>(chars)...> KSymbol{};

constexpr KTree<static_cast<uint8_t>(Type::Undef)> KUndef{};
constexpr KUnary<Type::Opposite> KOpposite{};
constexpr KUnary<Type::Parentheses> KParentheses{};
constexpr KBinary<Type::Sub> KSub{};
constexpr KBinary<Type::Pow> KPow{};
constexpr KBinary<Type::Div> KDiv{};
constexpr KNAry<Type::Mul> KMul{};
constexpr KNAry<Type::Add> KAdd{};

}
