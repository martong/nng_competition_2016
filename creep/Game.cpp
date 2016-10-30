#include "Game.hpp"

#include "Creep.hpp"

#include <boost/range/iterator_range.hpp>

Game::Game(std::istream& stream) {
    std::size_t width = 0;
    std::size_t height = 0;
    stream >> timeLimit >> width >> height;
    while (stream.get() != '\n') {}
    history.emplace_back(stream, width, height);
}

void Game::removeCommand(const Command& command) {
    auto range = commands.equal_range(command.time);
    for (auto iterator = range.first; iterator != range.second; ++iterator) {
        if (iterator->second == command) {
            commands.erase(iterator);
            return;
        }
    }
}

void Game::tick() {
    Status status = getStatus();
    if (nextCommand != commands.end() && status.getTime() ==
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
    history.push_back(status);
}

void Game::print(std::ostream& stream) {
    stream << "Tick " << getStatus().getTime() << "\n";
    for (const Tumor& tumor : getStatus().getTumors()) {
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

    for (const Queen& queen : getStatus().getQueens()) {
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
    stream << getStatus().getTable();
}

bool Game::canContinue() const {
    return hasTime() && (nextCommand != commands.end() ||
            getStatus().canSpread());
}
