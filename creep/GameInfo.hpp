#ifndef CREEP_GAMEINFO_HPP
#define CREEP_GAMEINFO_HPP

#include "Point.hpp"
#include "Table.hpp"

#include <istream>

struct GameInfo {
    Table table;
    Point hatcheryPosition;
    int timeLimit;
};

GameInfo loadGameInfo(std::istream& stream);

#endif // CREEP_GAMEINFO_HPP
