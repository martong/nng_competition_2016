#ifndef CREEP_GAME_HPP
#define CREEP_GAME_HPP

#include "Command.hpp"
#include "Status.hpp"

#include <istream>
#include <ostream>
#include <map>

class Game {
public:
    Game(std::istream& stream);

    void addCommand(const Command& command) {
        commands.push_back(command);
    }

    void tick();
    void print(std::ostream& stream);

    const Status& getStatus() const { return status; }
    const std::vector<Command> getCommands() const { return commands; }

    bool hasTime() const { return status.getTime() < timeLimit; }
    bool canContinue() const;

private:
    int timeLimit = 0;
    Status status;
    std::vector<Command> commands;
    std::size_t nextCommand = 0;
};

#endif // CREEP_GAME_HPP
