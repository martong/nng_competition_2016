#ifndef CREEP_TABLE_HPP
#define CREEP_TABLE_HPP

#include <Matrix.hpp>

namespace MapElement {
    constexpr int Floor = -1;
    constexpr int Wall = -2;
    constexpr int Building = -3;
}

// >= 0: creep at time T
//  < 0: value of MapElement
using Table = Matrix<int>;

inline bool hasCreep(int element) {
    return element != MapElement::Wall && element != MapElement::Floor;
}

#endif // CREEP_TABLE_HPP
