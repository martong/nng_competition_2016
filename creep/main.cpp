#include "Game.hpp"
#include "Options.hpp"
#include "Solver.hpp"

#include <util/PrefixMap.hpp>

#include <fstream>
#include <iostream>

void simulate(Game& game, const Options&) {
    int numberOfCommands = 0;
    std::cin >> numberOfCommands;
    for (int i = 0; i < numberOfCommands; ++i) {
        Command command;
        std::cin >> command.time >> command.type >> command.id >>
                command.position.x >> command.position.y;
        game.addCommand(command);
    }
    game.print(std::cout);
    while (game.canContinue()) {
        game.tick();
        game.print(std::cout);
    }
}

void solve(Game& game, const Options& options) {
    std::vector<Solution> solutions;
    iterateFinder(options.timeMultiplierFinder,
            [&](float timeMultiplier) {
                iterateFinder(options.distanceSquareMultiplierFinder,
                        [&](float distanceSquareMultiplier) {
                            iterateFinder(options.spreadRadiusMultiplierFinder,
                                    [&](float spreadRadiusMultiplier) {
                                        solutions.push_back(findSolution(game, {
                                                timeMultiplier,
                                                distanceSquareMultiplier,
                                                spreadRadiusMultiplier}));
                                    });
                        });
            });
    const auto& commands = std::min_element(solutions.begin(), solutions.end(),
            [](const Solution& lhs, const Solution& rhs) {
                return lhs.floorsRemaining < rhs.floorsRemaining ||
                        (lhs.floorsRemaining == rhs.floorsRemaining && 
                         lhs.time < rhs.time);
            })->commands;
    if (solutions.empty()) {
        std::cerr << "There was no simulations.\n";
        return;
    }
    std::cout << commands.size() << "\n";
    for (const Command& command : commands) {
        std::cout << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
}

int main(int argc, const char* argv[]) {
    Options options = parseOptions(argc, argv);
    util::PrefixMap<void(*)(Game&, const Options&)> actions{
            {"simulate", simulate},
            {"solve", solve}};
    std::ifstream inputFile{options.inputFileName};
    Game game{inputFile};
    inputFile.close();
    actions.at(options.type)(game, options);
}
