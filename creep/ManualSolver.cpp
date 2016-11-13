#include "ManualSolver.hpp"

#include "Game.hpp"
#include "MatrixIO.hpp"

#include <util/PrefixMap.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/lexical_cast.hpp>

#include <readline/history.h>
#include <readline/readline.h>

#include <algorithm>
#include <assert.h>
#include <functional>
#include <fstream>
#include <memory>
#include <string>
#include <iostream>
#include <vector>

namespace {

class ManualSolver {
public:
    ManualSolver(const GameInfo& gameInfo,
            const std::vector<Command>& initialCommands) :
            history{Game{gameInfo}} {
        for (const Command& command : initialCommands) {
            commands.push_back(CommandDescriptor{++commandId, command});
        }
        sortCommands();
        history.back().print(std::cout);
    }

    void execute(const std::vector<std::string>& args) {
        userCommands.at(args[0])(args);
    }

private:
    void tick() {
        Game newGame{history.back()};
        for (const CommandDescriptor& command : commands) {
            if (command.command.time == newGame.getStatus().getTime()) {
                newGame.addCommand(command.command);
            }
        }
        newGame.tick();
        assert(newGame.getStatus().getTime() ==
                static_cast<int>(history.size()));
        history.push_back(newGame);
    }

    void sortCommands() {
        std::sort(commands.begin(), commands.end(),
                [](const CommandDescriptor& lhs, const CommandDescriptor& rhs) {
                    return lhs.command.time < rhs.command.time;
                });
    }

    void gotoTime(const std::vector<std::string> args) {
        const std::string& arg = args.at(1);
        int value = boost::lexical_cast<int>(arg);
        int currentTime = history.back().getStatus().getTime();
        std::cerr << "Current time: " << currentTime << "\n";
        if (arg[0] == '+' || arg[0] == '-') {
            value += currentTime;
        }

        if (currentTime < 0) {
            throw std::logic_error{"Cannot jump to negative time."};
        }

        if (value > currentTime) {
            while (history.back().getStatus().getTime() != value) {
                tick();
            }
        } else {
            // We will never grow, but C++ requires something just in case
            history.resize(value + 1, history.back());
        }
        history.back().print(std::cout);
    }

    void showCreep(const std::vector<std::string> args) {
        Point from;
        from.x = boost::lexical_cast<int>(args.at(1));
        from.y = boost::lexical_cast<int>(args.at(2));
        Point to;
        to.x = boost::lexical_cast<int>(args.at(3));
        to.y = boost::lexical_cast<int>(args.at(4));
        const Status& status = history.back().getStatus();
        if (from.x < 0 || to.x > static_cast<int>(status.width()) ||
                from.x >= to.x ||
                from.y < 0 || to.y > static_cast<int>(status.height()) ||
                from.y >= to.y) {
            throw std::logic_error{"Invalid point."};
        }
        Matrix<int> matrix{static_cast<std::size_t>(to.x - from.x),
                static_cast<std::size_t>(to.y - from.y)};
        for (Point p : matrixRange(matrix)) {
            matrix[p] = status.creepTime(p + from);
        }
        std::cout << "Current time: " << status.getTime() << "\n" <<
                matrix;
    }

    void clearCommands(const std::vector<std::string> /*args*/) {
        boost::remove_erase_if(commands,
                [this](const CommandDescriptor& command) {
                    return command.command.time >=
                            history.back().getStatus().getTime();
                });
    }

    void deleteCommand(const std::vector<std::string>& args) {
        for (std::size_t i = 1; i < args.size(); ++i) {
            int id = boost::lexical_cast<int>(args[i]);
            auto it = std::find_if(commands.begin(), commands.end(),
                    [id](const CommandDescriptor& command) {
                        return command.id == id;
                    });
            if (it == commands.end()) {
                // Only basic exception safety here!
                throw std::logic_error{"No such command."};
            }
            if (it->command.time < history.back().getStatus().getTime()) {
                throw std::logic_error{
                        "Cannot delete already executed command"};
            }
            commands.erase(it);
        }
    }

    void showCommands(const std::vector<std::string>& /*args**/) {
        for (const CommandDescriptor& command : commands) {
            std::cout << "#" << command.id <<
                    " time=" << command.command.time << " type=" <<
                    (command.command.type == CommandType::PlaceTumorFromQueen ?
                            "queen" : "tumor") <<
                    " position=" << command.command.position << "\n";
        }
    }

    void nextTime(const std::vector<std::string>& /*args**/) {
        tick();
        history.back().print(std::cout);
    }

    void addCommand(CommandType type, const std::vector<std::string>& args) {
        int id = boost::lexical_cast<int>(args.at(1));
        Point p;
        p.x = boost::lexical_cast<int>(args.at(2));
        p.y = boost::lexical_cast<int>(args.at(3));
        Game& game = history.back();
        commands.push_back(CommandDescriptor{++commandId,
                Command{game.getStatus().getTime(), type, id, p}});
        sortCommands();
    }

    void dumpCommands(const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            std::ofstream fs{args[1]};
            dumpCommandsTo(fs);
        } else {
            dumpCommandsTo(std::cout);
        }
    }

    void help(const std::vector<std::string>& /*args*/) {
        for (const auto& element : userCommands) {
            std::cout << element.first << "\n";
        }
    }

    void dumpCommandsTo(std::ostream& stream) {
    for (const CommandDescriptor& command : commands) {
        stream << command.command.time << " " << command.command.type << " " <<
                command.command.id << " " << command.command.position.x << " " <<
                command.command.position.y << "\n";
    }
    }

    util::PrefixMap<std::function<
            void(const std::vector<std::string>&)>> userCommands{
            {"clear", std::bind(&ManualSolver::clearCommands,
                    this, std::placeholders::_1)},
            {"commands", std::bind(&ManualSolver::showCommands,
                    this, std::placeholders::_1)},
            {"delete", std::bind(&ManualSolver::deleteCommand,
                    this, std::placeholders::_1)},
            {"dump", std::bind(&ManualSolver::dumpCommands,
                    this, std::placeholders::_1)},
            {"goto", std::bind(&ManualSolver::gotoTime,
                    this, std::placeholders::_1)},
            {"help", std::bind(&ManualSolver::help,
                    this, std::placeholders::_1)},
            {"tumor", std::bind(&ManualSolver::addCommand,
                    this, CommandType::PlaceTumorFromTumor,
                    std::placeholders::_1)},
            {"queen", std::bind(&ManualSolver::addCommand,
                    this, CommandType::PlaceTumorFromQueen,
                    std::placeholders::_1)},
            {"show", std::bind(&ManualSolver::showCreep,
                    this, std::placeholders::_1)},
            {"next", std::bind(&ManualSolver::nextTime,
                    this, std::placeholders::_1)},
    };

    struct CommandDescriptor {
        int id;
        Command command;
    };

    int commandId = 0;
    std::vector<Game> history;
    std::vector<CommandDescriptor> commands;
};

} // unnamed namespace

void solveManually(const GameInfo& gameInfo,
        const std::vector<Command>& commands) {
    ManualSolver manualSolver{gameInfo, commands};
    std::string line;
    while (std::cin.good()) {
        try {
            {
                std::unique_ptr<char[], void(*)(void*)> rawLine{
                        readline("> "), free};
                if (!rawLine) {
                    break;
                }
                if (rawLine[0] != 0) {
                    add_history(rawLine.get());
                }
                line = rawLine.get();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> tokens;
            boost::algorithm::split(tokens, line, boost::algorithm::is_space());
            manualSolver.execute(tokens);
        } catch (std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    }
}
