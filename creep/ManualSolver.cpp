#include "ManualSolver.hpp"

#include "Game.hpp"
#include "MatrixIO.hpp"

#include <util/PrefixMap.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <assert.h>
#include <functional>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

namespace {

class ManualSolver {
public:
    ManualSolver(const GameInfo& gameInfo) :
            history{Game{gameInfo}} {
        history.back().print(std::cout);
    }

    void execute(const std::vector<std::string>& args) {
        userCommands.at(args[0])(args);
    }

    const std::vector<Command>& getCommands() const {
        return commands;
    }
private:
    void tick() {
        Game newGame{history.back()};
        for (const Command& command : commands) {
            if (command.time == newGame.getStatus().getTime()) {
                newGame.addCommand(command);
            }
        }
        newGame.tick();
        assert(newGame.getStatus().getTime() ==
                static_cast<int>(history.size()));
        history.push_back(newGame);
    }

    void gotoTime(const std::vector<std::string> args) {
        const std::string& arg = args.at(1);
        int value = boost::lexical_cast<int>(arg);
        int currentTime = history.back().getStatus().getTime();
        if (arg[0] == '+' || arg[0] == '-') {
            value += currentTime;
        }

        if (currentTime < 0) {
            throw std::logic_error{"Cannot jump to negative time."};
        }

        if (value > currentTime) {
            while (history.back().canContinue() &&
                    history.back().getStatus().getTime() != value) {
                tick();
            }
        } else {
            // We will never grow, but C++ requires something just in case
            history.resize(value, history.back());
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
                [this](const Command& command) {
                    return command.time >= history.back().getStatus().getTime();
                });
    }

    void showCommands(const std::vector<std::string>& /*args**/) {
        for (const Command& command : commands) {
            std::cout << "time=" << command.time <<
                    " type=" <<
                    (command.type == CommandType::PlaceTumorFromQueen ?
                            "queen" : "tumor") <<
                    " position=" << command.position << "\n";
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
        commands.push_back({game.getStatus().getTime(), type, id, p});
        std::sort(commands.begin(), commands.end(),
                [](const Command& lhs, const Command& rhs) {
                    return lhs.time < rhs.time;
                });
    }

    void dumpCommands(const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            std::ofstream fs{args[1]};
            dumpCommandsTo(fs);
        } else {
            dumpCommandsTo(std::cout);
        }
    }

    void dumpCommandsTo(std::ostream& stream) {
    for (const Command& command : commands) {
        stream << command.time << " " << command.type << " " <<
                command.id << " " << command.position.x << " " <<
                command.position.y << "\n";
    }
    }

    util::PrefixMap<std::function<
            void(const std::vector<std::string>&)>> userCommands{
            {"goto", std::bind(&ManualSolver::gotoTime,
                    this, std::placeholders::_1)},
            {"tumor", std::bind(&ManualSolver::addCommand,
                    this, CommandType::PlaceTumorFromTumor,
                    std::placeholders::_1)},
            {"queen", std::bind(&ManualSolver::addCommand,
                    this, CommandType::PlaceTumorFromQueen,
                    std::placeholders::_1)},
            {"show", std::bind(&ManualSolver::showCreep,
                    this, std::placeholders::_1)},
            {"clear", std::bind(&ManualSolver::clearCommands,
                    this, std::placeholders::_1)},
            {"commands", std::bind(&ManualSolver::showCommands,
                    this, std::placeholders::_1)},
            {"dump", std::bind(&ManualSolver::dumpCommands,
                    this, std::placeholders::_1)},
            {"next", std::bind(&ManualSolver::nextTime,
                    this, std::placeholders::_1)},
    };

    std::vector<Game> history;
    std::vector<Command> commands;
};

} // unnamed namespace

void solveManually(const GameInfo& gameInfo) {
    ManualSolver manualSolver{gameInfo};
    std::string line;
    while (std::cin.good()) {
        try {
            std::cerr << "> ";
            std::getline(std::cin, line);
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
