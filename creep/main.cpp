#include "Game.hpp"
#include "Solver.hpp"

#include <util/PrefixMap.hpp>

#include <fstream>
#include <iostream>

void simulate(Game& game) {
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

void solve(Game& game) {
    auto solution = findSolution(game);
    std::cout << solution.size() << "\n";
    for (const Command& command : solution) {
        std::cout << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
}

int main(int argc, const char* argv[]) {
    assert(argc == 3);
    util::PrefixMap<void(*)(Game&)> actions{
            {"simulate", simulate},
            {"solve", solve}};
    std::ifstream inputFile{argv[2]};
    Game game{inputFile};
    inputFile.close();
    actions.at(argv[1])(game);
}
