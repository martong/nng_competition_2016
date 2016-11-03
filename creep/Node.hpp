#ifndef CREEP_NODE_HPP
#define CREEP_NODE_HPP

#include "Command.hpp"
#include "Matrix.hpp"
#include "Status.hpp"

#include <algorithm>
#include <memory>
#include <vector>

struct Node {
    Node(Command command, Status status, std::shared_ptr<Node> ancestor) :
            command(std::move(command)), status(std::move(status)),
            ancestor(std::move(ancestor)) {
    }

    Command command;
    Status status; // before executing the command
    std::shared_ptr<Node> ancestor;
};

inline
std::vector<Command> getCommands(std::shared_ptr<Node> node) {
    std::vector<Command> result;
    while (node) {
        result.push_back(node->command);
        node = node->ancestor;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

#endif // CREEP_NODE_HPP
