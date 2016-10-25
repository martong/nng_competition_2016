#include "Direction.hpp"

#include <ostream>
#include <vector>

std::ostream& operator<<(std::ostream& out, const Direction& direction) {
    static std::vector<std::string> directionStrings{
            "left", "right", "up", "down"};
    return out << directionStrings[static_cast<int>(direction)];
}

