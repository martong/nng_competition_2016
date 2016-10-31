#include "Game.hpp"
#include "Options.hpp"
#include "Solver.hpp"

#include <util/PrefixMap.hpp>
#include <util/ThreadPool.hpp>

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

template<typename OnFinished>
void doSolve(const Game& game, const Heuristics& heuristics,
        const OnFinished& onFinished) {
    auto solution = findSolution(game, heuristics);
    onFinished(solution);
}

void solve(Game& game, const Options& options) {
    std::vector<Solution> solutions;
    util::ThreadPool threadPool{options.numThreads};
    boost::asio::io_service& ioService = threadPool.getIoService();
    boost::asio::strand strand{ioService};
    auto onFinished =
            [&solutions, &ioService, &strand](const Solution& solution) {
                ioService.post(strand.wrap(
                        [&solutions, solution]() {
                            solutions.push_back(solution);
                        }));
            };
    threadPool.start();
    iterateFinder(options.timeMultiplierFinder,
            [&](float timeMultiplier) {
                iterateFinder(options.distanceSquareMultiplierFinder,
                        [&](float distanceSquareMultiplier) {
                            iterateFinder(options.spreadRadiusMultiplierFinder,
                                    [&](float spreadRadiusMultiplier) {
                                        ioService.post(
                                                [=, &game, &onFinished]() {
                                                    doSolve(game, {
                                                    timeMultiplier,
                                                    distanceSquareMultiplier,
                                                    spreadRadiusMultiplier},
                                                    onFinished);
                                                });
                                    });
                        });
            });
    threadPool.wait();
    if (solutions.empty()) {
        std::cerr << "There was no simulations.\n";
        return;
    }
    const auto& solution = *std::min_element(solutions.begin(), solutions.end(),
            [](const Solution& lhs, const Solution& rhs) {
                return lhs.floorsRemaining < rhs.floorsRemaining ||
                        (lhs.floorsRemaining == rhs.floorsRemaining &&
                         lhs.time < rhs.time);
            });
    std::cerr << "Best solution: tm=" << solution.heuristics.timeMultiplier <<
            " dsm=" << solution.heuristics.distanceSquareMultiplier <<
            " srm=" << solution.heuristics.spreadRadiusMultiplier <<
            " floors=" << solution.floorsRemaining <<
            " time=" << solution.time << "\n";
    const auto& commands = solution.commands;
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
