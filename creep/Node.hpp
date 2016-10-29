#ifndef CREEP_NODE_HPP
#define CREEP_NODE_HPP

#include <Point.hpp>

#include <istream>
#include <ostream>

enum class NodeType { PlaceTumorFromQueen = 1, PlaceTumorFromTumor = 2 };

inline
std::ostream& operator<<(std::ostream& os, NodeType nodeType) {
    return os << static_cast<int>(nodeType);
}

inline
std::istream& operator>>(std::istream& is, NodeType& nodeType) {
    int value;
    is >> value;
    nodeType = static_cast<NodeType>(value);
    return is;
}

struct Node {
    int time;
    NodeType type;
    int id;
    Point position;
};

#endif // CREEP_NODE_HPP
