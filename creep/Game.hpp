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
            history(other.history), commands(other.commands) {
        calculateNextCommand();
    }

    Game& operator=(const Game& other) {
        timeLimit  = other.timeLimit;
        history = other.history;
        commands = other.commands;
        calculateNextCommand();
        return *this;
    }

    Game(Game&& other) : timeLimit(other.timeLimit),
            history(std::move(other.history)),
            commands(std::move(other.commands)) {
        calculateNextCommand();
    }

    Game& operator=(Game&& other) {
        timeLimit  = other.timeLimit;
        history = std::move(other.history);
        commands = std::move(other.commands);
        calculateNextCommand();
        return *this;
    }

    void addCommand(const Command& command) {
        commands.emplace(command.time, command);
        calculateNextCommand();
    }

    void removeCommand(const Command& command);

    void rewind(int to) {
        history.resize(to + 1);
        assert(getStatus().getTime() == to);
        calculateNextCommand();
    }

    void tick();
    void print(std::ostream& stream);

    const Status& getStatus() const { return history.back(); }
    const std::vector<Status>& getHistory() const { return history; }
    const Commands& getCommands() const { return commands; }

    bool hasTime() const { return getStatus().getTime() < timeLimit; }
    bool canContinue() const;

private:
    void calculateNextCommand() {
        nextCommand = commands.lower_bound(getStatus().getTime());
    }

    int timeLimit = 0;
    std::vector<Status> history;
    Commands commands;
    Commands::iterator nextCommand = commands.end();
};

#endif // CREEP_GAME_HPP
