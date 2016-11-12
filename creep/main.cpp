#include "AIGameManager.hpp"
#include "Game.hpp"
#include "Options.hpp"
#include "Solver.hpp"
#include "LearningController.hpp"

#include <util/PrefixMap.hpp>
#include <util/ThreadPool.hpp>

#include <boost/archive/text_iarchive.hpp>

#include <fstream>
#include <iostream>

void simulate(const GameInfo& gameInfo, const Options&) {
    Game game{gameInfo};
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
void doSolve(const GameInfo& gameInfo, const Heuristics& heuristics,
        const OnFinished& onFinished) {
    std::cerr << "Solving: tm=" << heuristics.timeMultiplier <<
            " dsm=" << heuristics.distanceSquareMultiplier <<
            " srm=" << heuristics.spreadRadiusMultiplier << "\n";
    auto solution = findSolution(gameInfo, heuristics);
    std::cerr << "Solved: tm=" << solution.heuristics.timeMultiplier <<
            " dsm=" << solution.heuristics.distanceSquareMultiplier <<
            " srm=" << solution.heuristics.spreadRadiusMultiplier <<
            " floors=" << solution.floorsRemaining <<
            " time=" << solution.time << "\n";
    onFinished(solution);
}

void solve(const GameInfo& gameInfo, const Options& options) {
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
                                                [=, &gameInfo, &onFinished]() {
                                                    doSolve(gameInfo, {
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
    auto commands = getCommands(solution.node);
    std::cout << commands.size() << "\n";
    for (const Command& command : commands) {
        std::cout << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
}

void neural(const GameInfo& gameInfo, const Options& options) {
    util::ThreadPool threadPool{options.numThreads};
    threadPool.start();
    boost::asio::io_service& ioService = threadPool.getIoService();
    LearningController learningController{options.learningParameters,
        {gameInfo}, ioService};
    learningController.run();
    threadPool.wait();
}

void generate(const GameInfo& gameInfo, const Options& options) {
    NeuralNetwork neuralNetwork;
    std::ifstream fs{options.aiFileName};
    boost::archive::text_iarchive ar(fs);
    ar >> neuralNetwork;
    AIGameManager gameManager{options.learningParameters.commonParameters,
            gameInfo};
    gameManager.setNeuralNetwork(neuralNetwork);
    gameManager.init();
    gameManager.run();
    std::cout << gameManager.getCommands().size() << "\n";
    for (const Command& command : gameManager.getCommands()) {
        std::cout << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
}

int main(int argc, const char* argv[]) {
    Options options = parseOptions(argc, argv);
    util::PrefixMap<void(*)(const GameInfo&, const Options&)> actions{
            {"simulate", simulate},
            {"neural", neural},
            {"generate", generate},
            {"solve", solve}};
    std::ifstream inputFile{options.inputFileName};
    GameInfo gameInfo{loadGameInfo(inputFile)};
    inputFile.close();
    actions.at(options.type)(gameInfo, options);
}
