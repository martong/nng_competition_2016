#include "Game.hpp"

#include <fstream>
#include <iostream>

int main(int argc, const char* argv[]) {
    assert(argc == 2);
    std::ifstream inputFile{argv[1]};
    Game game{inputFile};
    inputFile.close();
    int numberOfNodes = 0;
    std::cin >> numberOfNodes;
    for (int i = 0; i < numberOfNodes; ++i) {
        Node node;
        std::cin >> node.time >> node.type >> node.id >>
                node.position.x >> node.position.y;
        game.addNode(node);
    }
    game.print(std::cout);
    while (game.canContinue()) {
        game.tick();
        game.print(std::cout);
    }
}
