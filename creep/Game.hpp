#ifndef CREEP_GAME_HPP
#define CREEP_GAME_HPP

#include "Command.hpp"
#include "Status.hpp"

#include <istream>
#include <ostream>
#include <map>

class Game {
public:
    using Commands = std::map<int, Command>;

    Game(std::istream& stream);

    void addCommand(const Command& command) {
        commands.emplace(command.time, command);
        nextCommand = commands.lower_bound(getStatus().getTime());
    }

    void tick();
    void print(std::ostream& stream);

    const Status& getStatus() const { return history.back(); }
    const Commands getCommands() const { return commands; }

    bool hasTime() const { return getStatus().getTime() < timeLimit; }
    bool canContinue() const;

private:
    int timeLimit = 0;
    std::vector<Status> history;
    Commands commands;
    Commands::iterator nextCommand = commands.end();
};

#endif // CREEP_GAME_HPP
