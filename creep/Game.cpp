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
    if (nextNode < nodes.size() && status.getTime() == nodes[nextNode].time) {
        const Node& node = nodes[nextNode];
        switch (node.type) {
            case NodeType::PlaceTumorFromQueen:
                status.addTumorFromQueen(node.id, node.position);
                break;
            case NodeType::PlaceTumorFromTumor:
                status.addTumorFromTumor(node.id, node.position);
                break;
            default:
                assert(false && "Impossible node type");
        }
        ++nextNode;
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

    if (nextNode < nodes.size()) {
        const Node& node = nodes[nextNode];
        stream << "Next node: time=" << node.time << ", type=" << node.type <<
                ", id=" << node.id << ", position=" << node.position << "\n";
    } else {
        stream << "No more nodes.\n";
    }
    stream << status.getTable();
}

bool Game::canContinue() const {
    return hasTime() && (nextNode < nodes.size() || status.canSpread());
}
