#ifndef CREEP_NODE_HPP
#define CREEP_NODE_HPP

#include <Point.hpp>

#include <istream>
#include <ostream>

enum class CommandType { PlaceTumorFromQueen = 1, PlaceTumorFromTumor = 2 };

inline
std::ostream& operator<<(std::ostream& os, CommandType nodeType) {
    return os << static_cast<int>(nodeType);
}

inline
std::istream& operator>>(std::istream& is, CommandType& nodeType) {
    int value;
    is >> value;
    nodeType = static_cast<CommandType>(value);
    return is;
}

struct Command {
    int time;
    CommandType type;
    int id;
    Point position;
};

#endif // CREEP_NODE_HPP
