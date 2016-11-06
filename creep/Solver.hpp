#ifndef CREEP_SOLVER_HPP
#define CREEP_SOLVER_HPP

#include "Command.hpp"
#include "Random.hpp"
#include "Game.hpp"

#include <boost/asio/io_service.hpp>

class Solver {
public:
    struct Parameters {
        std::size_t runsPerIteration = 20;
        float timeScore = 1.0f;
        float floorPenalty = 100.0f;
        float heatFlowStrength = 0.3f;
        int heatFlowMaxDistance = 10;
        float cooldownFactor = 0.75f;
        float maximumScore = 1000.0;
    };

    struct Heuristics {
        float spreadRadiusMultiplier = 100.0f;
        float timeMultiplier = 100.0f;
        float minimumDistanceMultiplier = 2.0f;
    };

    struct Solution {
        std::vector<Command> commands;
        int floorsRemaining;
        int time;
        float score;
    };

    Solver(RandomGenerator& rng, Game game, const Parameters& parameters,
            const Heuristics& heuristics) :
            rng(rng), initialGame(std::move(game)),
            heatMap(game.getStatus().width(), game.getStatus().height(), 0.0f),
            parameters(parameters), heuristics(heuristics) {
    }

    Solution iterate(boost::asio::io_service& ioService);

private:
    void normalizeSolutions(std::vector<Solution>& solutions);
    void updateHeatMap(const Solution& solution);
    void coolDownHeatMap();

    RandomGenerator& rng;
    Game initialGame;
    Matrix<float> heatMap;
    Parameters parameters;
    Heuristics heuristics;
};

#endif // CREEP_SOLVER_HPP
