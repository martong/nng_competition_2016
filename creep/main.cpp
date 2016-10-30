#include "Game.hpp"

#include <fstream>
#include <iostream>

int main(int argc, const char* argv[]) {
    assert(argc == 2);
    std::ifstream inputFile{argv[1]};
    Game game{inputFile};
    inputFile.close();
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
