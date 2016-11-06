#include "Game.hpp"
#include "Options.hpp"
#include "Random.hpp"
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

void printCommands(std::ostream& stream, const Solver::Solution& solution) {
    const auto& commands = solution.commands;
    stream << commands.size() << "\n";
    for (const Command& command : commands) {
        stream << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
}

void solve(Game& game, const Options& options) {
    util::ThreadPool threadPool{options.numThreads};
    RandomGenerator rng{options.seed == 0 ? std::random_device{}() :
            options.seed};
    Solver solver{rng, game, options.solverParameters,
            options.solverHeuristics};
    threadPool.start();
    for (std::size_t iteration = 0;
                options.maxIterations == 0 || iteration < options.maxIterations;
                ++iteration) {
        auto solution = solver.iterate(threadPool.getIoService());
        std::ofstream fs{options.outputFileName};
        printCommands(fs, solution);
        if (iteration % 1 == 0) {
            std::cerr << "Iteration #" << iteration <<
                    " floors=" << solution.floorsRemaining <<
                    " time=" << solution.time << "\n";
        }
    }
    threadPool.wait();
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
