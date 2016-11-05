#include "Status.hpp"

#include "Constants.hpp"
#include "Log.hpp"

#include <MatrixIO.hpp>

#include <algorithm>
#include <assert.h>

namespace {

template<typename Ts>
typename Ts::reference findObjectWithId(Ts& ts, int id) {
    return *std::find_if(ts.begin(), ts.end(),
            [id](const auto& element) { return element.id == id; });
}

} // unnamed namespace

Status::Status(std::istream& stream, std::size_t width, std::size_t height) :
        table{width, height} {
    auto matrix = loadMatrix(stream, '#', width, height, true);
    for (Point p : matrixRange(matrix)) {
        table[p] = matrix[p] == '.' ? MapElement::Floor : MapElement::Wall;
    }
    Point hatcheryPosition;
    stream >> hatcheryPosition.x >> hatcheryPosition.y;

    // place hatchery
    for (Point p : PointRange{hatcheryPosition,
            hatcheryPosition + p11 * rules::hatcherySize}) {
        table[p] = MapElement::Building;
    }
    Point hatcheryCenter = hatcheryPosition + p11 * rules::hatcheryCenterOffset;
    tumors.emplace_back(1, hatcheryCenter, -1);

    // place a creep tile
    for (Point p : PointRange{hatcheryPosition - p11,
            hatcheryPosition + p11 * 5}) {
        if (table[p] == MapElement::Floor) {
            table[p] = 0;
            break;
        }
    }

    floorsRemaining = std::count(table.begin(), table.end(), MapElement::Floor);
    // spread creep until possible
    while (spreadCreepFrom(hatcheryPosition + p11, 0)) {}

    addQueen();
}

void Status::tick() {
    spreadCreep();
    for (Tumor& tumor : tumors) {
        if (tumor.cooldown > 0) {
            --tumor.cooldown;
        }
    }
    for (Queen& queen : queens) {
        if (queen.energy < rules::queenMaximumEnergy) {
            ++queen.energy;
        }
    }
    ++time;
    if (time % rules::queenSpawnTime == 0) {
        addQueen();
    }
}

const Tumor& Status::addTumorFromQueen(int id, Point position) {
    Queen& queen = findObjectWithId(queens, id);
    assert(queen.energy >= rules::queenEnertyRequirement);
    queen.energy -= rules::queenEnertyRequirement;
    return addTumor(position);
}

const Tumor& Status::addTumorFromTumor(int id, Point position) {
    Tumor& tumor = findObjectWithId(tumors, id);
    assert(tumor.cooldown == 0);
    tumor.cooldown = -1;
    return addTumor(position);
}

bool Status::canSpread() const {
    return std::any_of(tumors.begin(), tumors.end(),
            [this](const Tumor& tumor) {
                return !getSpreadArea(tumor.position, rules::creepSpreadRadius,
                        getPredicate(&Status::isCreepCandidate)).empty();
            });
    return true;
}

void Status::addQueen() {
    queens.emplace_back(nextId++, rules::queenStartingEnergy);
}

void Status::spreadCreep() {
    std::size_t hash = time * time + 37;
    for (const Tumor& tumor : tumors) {
        spreadCreepFrom(tumor.position, hash);
    }
}

bool Status::spreadCreepFrom(Point p, std::size_t hash) {
    std::vector<Point> candidates = getSpreadArea(p, rules::creepSpreadRadius,
            getPredicate(&Status::isCreepCandidate));
    if (!candidates.empty()) {
        table[candidates[hash % candidates.size()]] = time;
        --floorsRemaining;
        return true;
    }
    return false;
}

const Tumor& Status::addTumor(Point position) {
    assert(isCreep(position));
    tumors.emplace_back(nextId++, position, rules::tumorCooldownTime);
    table[position] = MapElement::Building;
    const Tumor& result = tumors.back();
    LOG << "Adding tumor. time=" << time << " id=" << result.id <<
            " position=" << result.position << "\n";
    return result;
}
