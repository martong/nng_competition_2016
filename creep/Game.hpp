#ifndef CREEP_GAME_HPP
#define CREEP_GAME_HPP

#include "Command.hpp"
#include "Status.hpp"

#include <istream>
#include <ostream>
#include <map>

class Game {
public:
    using Commands = std::multimap<int, Command>;

    Game(std::istream& stream);

    Game(const Game& other) : timeLimit(other.timeLimit),
            status(other.status), commands(other.commands) {
        calculateNextCommand();
    }

    Game& operator=(const Game& other) {
        timeLimit  = other.timeLimit;
        status = other.status;
        commands = other.commands;
        calculateNextCommand();
        return *this;
    }

    Game(Game&& other) : timeLimit(other.timeLimit),
            status(std::move(other.status)),
            commands(std::move(other.commands)) {
        calculateNextCommand();
    }

    Game& operator=(Game&& other) {
        timeLimit  = other.timeLimit;
        status = std::move(other.status);
        commands = std::move(other.commands);
        calculateNextCommand();
        return *this;
    }

    void addCommand(const Command& command) {
        commands.emplace(command.time, command);
        calculateNextCommand();
    }

    void setStatus(Status status) {
        this->status = std::move(status);
    }

    void removeCommand(const Command& command);
    void removeCommands(Commands::const_iterator from,
            Commands::const_iterator to);

    void tick();
    void print(std::ostream& stream);

    const Status& getStatus() const { return status; }
    const Commands& getCommands() const { return commands; }

    bool hasTime() const { return status.getTime() < timeLimit; }
    bool canContinue() const;

private:
    void calculateNextCommand() {
        nextCommand = commands.lower_bound(getStatus().getTime());
    }

    int timeLimit = 0;
    Status status;
    Commands commands;
    Commands::iterator nextCommand = commands.end();
};

#endif // CREEP_GAME_HPP
