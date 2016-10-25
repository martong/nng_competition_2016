#ifndef UTIL_DIRECTION_HPP
#define UTIL_DIRECTION_HPP

#include <iosfwd>

enum class Direction {
    left,
    right,
    up,
    down
};

std::ostream& operator<<(std::ostream& out, const Direction& direction);


#endif // UTIL_DIRECTION_HPP
