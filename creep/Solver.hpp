#ifndef CREEP_SOLVER_HPP
#define CREEP_SOLVER_HPP

#include "Game.hpp"
#include "Command.hpp"

struct Heuristics {
    float timeMultiplier;
    float distanceSquareMultiplier;
    float spreadRadiusMultiplier;
};

struct Solution {
    std::vector<Command> commands;
    int floorsRemaining;
    int time;
};

Solution findSolution(Game game, const Heuristics& heuristics);

#endif // CREEP_SOLVER_HPP
