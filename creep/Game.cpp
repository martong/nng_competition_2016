#include "Game.hpp"

#include <boost/range/iterator_range.hpp>

Game::Game(std::istream& stream) {
    std::size_t width = 0;
    std::size_t height = 0;
    stream >> timeLimit >> width >> height;
    while (stream.get() != '\n') {}
    status = Status{stream, width, height};
}

void Game::removeCommand(const Command& command) {
    auto range = commands.equal_range(command.time);
    for (auto iterator = range.first; iterator != range.second; ++iterator) {
        if (iterator->second == command) {
            commands.erase(iterator);
            calculateNextCommand();
            return;
        }
    }
}

void Game::removeCommands(Commands::const_iterator from,
        Commands::const_iterator to) {
    commands.erase(from, to);
    calculateNextCommand();
}

void Game::tick() {
    while (nextCommand != commands.end() && status.getTime() ==
            nextCommand->first) {
        const Command& command = nextCommand->second;
        switch (command.type) {
            case CommandType::PlaceTumorFromQueen:
                status.addTumorFromQueen(command.id, command.position);
                break;
            case CommandType::PlaceTumorFromTumor:
                status.addTumorFromTumor(command.id, command.position);
                break;
            default:
                assert(false && "Impossible command type");
        }
        ++nextCommand;
    }
    status.tick();

}

void Game::print(std::ostream& stream) {
    stream << "Tick " << status.getTime() << "\n";
    for (const Tumor& tumor : status.getTumors()) {
        stream << "Tumor #" << tumor.id << ": " << tumor.position <<
                " ";
        if (tumor.cooldown > 0) {
            stream << "cooldown " << tumor.cooldown;
        } else if (tumor.cooldown == 0) {
            stream << "active";
        } else {
            stream << "inactive";
        }
        stream << "\n";
    }

    for (const Queen& queen : status.getQueens()) {
        stream << "Queen #" << queen.id << ": energy=" << queen.energy << "\n";
    }

    stream << "Floors remaining: " << getStatus().getFloorsRemaining() << "\n";

    if (nextCommand != commands.end()) {
        const Command& command = nextCommand->second;
        stream << "Next command: time=" << command.time <<
                ", type=" << command.type << ", id=" << command.id <<
                ", position=" << command.position << "\n";
    } else {
        stream << "No more commands.\n";
    }
    //stream << status.getTable();
}

bool Game::canContinue() const {
    const auto& tumors = status.getTumors();
    return hasTime() && (nextCommand != commands.end() ||
            status.canSpread() || std::find_if(
                    tumors.begin(), tumors.end(),
                    [](const Tumor& tumor) {
                        return tumor.cooldown > 0;
                    }) != tumors.end());
}
