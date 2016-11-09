#include "GameInfo.hpp"

#include <MatrixIO.hpp>

GameInfo loadGameInfo(std::istream& stream) {
    GameInfo gameInfo;
    std::size_t width = 0;
    std::size_t height = 0;
    stream >> gameInfo.timeLimit >> width >> height;
    while (stream.get() != '\n') {}
    auto matrix = loadMatrix(stream, '#', width, height, true);
    gameInfo.table = Table{width, height};
    for (Point p : matrixRange(matrix)) {
        gameInfo.table[p] = matrix[p] == '.' ? MapElement::Floor : MapElement::Wall;
    }
    stream >> gameInfo.hatcheryPosition.x >> gameInfo.hatcheryPosition.y;
    return gameInfo;
}

