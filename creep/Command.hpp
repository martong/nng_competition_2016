#ifndef CREEP_COMMAND_HPP
#define CREEP_COMMAND_HPP

#include <Point.hpp>

#include <istream>
#include <ostream>

enum class CommandType { PlaceTumorFromQueen = 1, PlaceTumorFromTumor = 2 };

inline
std::ostream& operator<<(std::ostream& os, CommandType commandType) {
    return os << static_cast<int>(commandType);
}

inline
std::istream& operator>>(std::istream& is, CommandType& commandType) {
    int value;
    is >> value;
    commandType = static_cast<CommandType>(value);
    return is;
}

struct Command {
    int time;
    CommandType type;
    int id;
    Point position;
};

inline
bool operator==(const Command& lhs, const Command& rhs) {
    return lhs.time == rhs.time && lhs.type == rhs.type &&
            lhs.id == rhs.id && lhs.position == rhs.position;
}

#endif // CREEP_COMMAND_HPP
