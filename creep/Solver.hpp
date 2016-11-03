#ifndef CREEP_SOLVER_HPP
#define CREEP_SOLVER_HPP

#include "Command.hpp"
#include "Game.hpp"
#include "Node.hpp"

struct Heuristics {
    float timeMultiplier;
    float distanceSquareMultiplier;
    float spreadRadiusMultiplier;
};

struct Solution {
    std::shared_ptr<Node> node;
    Heuristics heuristics;
    int floorsRemaining;
    int time;
};

Solution findSolution(Game game, const Heuristics& heuristics,
        std::shared_ptr<Node> startingNode = nullptr);

#endif // CREEP_SOLVER_HPP
