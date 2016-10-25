#include "Point.hpp"

#include <cassert>

std::ostream& operator<<(std::ostream& os, Point p) {
    os << '(' << p.x << ", " << p.y << ')';
    return os;
}

Direction toDirection(const Point& source, const Point& destination) {
    Point diff = destination - source;
    if (diff == p10) {
        return Direction::right;
    } else if (diff == p01) {
        return Direction::down;
    } else if (diff == p11) {
        assert(false);
    } else if (diff*-1 == p10) {
        return Direction::left;
    } else if (diff*-1 == p01) {
        return Direction::up;
    } else if (diff*-1 == p11) {
        assert(false);
    } else {
        assert(false);
    }
    assert(false);
}
