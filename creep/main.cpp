#include "AIGameManager.hpp"
#include "Game.hpp"
#include "Options.hpp"
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
    std::cerr << "Fitness=" << gameManager.getFitness() << "\n";
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
        };
    std::ifstream inputFile{options.inputFileName};
    GameInfo gameInfo{loadGameInfo(inputFile)};
    inputFile.close();
    actions.at(options.type)(gameInfo, options);
}
