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

inline bool isInsideCircle(Point p, int radius) {
    // magic
    int dx_q1=2*p.x+(0<p.x?1:-1);
    int dy_q1=2*p.y+(0<p.y?1:-1);
    int d2_q2=dx_q1*dx_q1+dy_q1*dy_q1;
    return d2_q2<=radius*radius*4;
}

inline bool hasCreep(int element) {
    return element != MapElement::Wall && element != MapElement::Floor;
}

#endif // CREEP_TABLE_HPP
