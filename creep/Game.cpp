#include "Game.hpp"

#include "Creep.hpp"

Game::Game(std::istream& stream) {
    std::size_t width = 0;
    std::size_t height = 0;
    stream >> timeLimit >> width >> height;
    while (stream.get() != '\n') {}
    status = Status{stream, width, height};
}

void Game::tick() {
    if (nextCommand < commands.size() && status.getTime() == 
            commands[nextCommand].time) {
        const Command& command = commands[nextCommand];
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

    if (nextCommand < commands.size()) {
        const Command& command = commands[nextCommand];
        stream << "Next command: time=" << command.time << ", type=" << command.type <<
                ", id=" << command.id << ", position=" << command.position << "\n";
    } else {
        stream << "No more commands.\n";
    }
    stream << status.getTable();
}

bool Game::canContinue() const {
    return hasTime() && (nextCommand < commands.size() || status.canSpread());
}
