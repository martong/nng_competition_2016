#ifndef CREEP_MANUALSOLVER_HPP
#define CREEP_MANUALSOLVER_HPP

#include "GameInfo.hpp"
#include "Command.hpp"

#include <vector>

void solveManually(const GameInfo& gameInfo,
        const std::vector<Command>& commands);

#endif // CREEP_MANUALSOLVER_HPP
