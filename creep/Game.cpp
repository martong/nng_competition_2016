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
    if (nextCommand < nodes.size() && status.getTime() == nodes[nextCommand].time) {
        const Command& node = nodes[nextCommand];
        switch (node.type) {
            case CommandType::PlaceTumorFromQueen:
                status.addTumorFromQueen(node.id, node.position);
                break;
            case CommandType::PlaceTumorFromTumor:
                status.addTumorFromTumor(node.id, node.position);
                break;
            default:
                assert(false && "Impossible node type");
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

    if (nextCommand < nodes.size()) {
        const Command& node = nodes[nextCommand];
        stream << "Next node: time=" << node.time << ", type=" << node.type <<
                ", id=" << node.id << ", position=" << node.position << "\n";
    } else {
        stream << "No more nodes.\n";
    }
    stream << status.getTable();
}

bool Game::canContinue() const {
    return hasTime() && (nextCommand < nodes.size() || status.canSpread());
}
