#include "AIGameManager.hpp"

#include "Constants.hpp"
#include "Spread.hpp"

#include <boost/range/adaptor/transformed.hpp>

namespace {

int findTotalCount(int radius) {
    Point dd{radius, radius};
    result.push_back();
    return countSpreadArea(dd * 2, dd, distance, alwaysTrue);
}

std::vector<int> findTotalCounts() {
    std::veector<int> result;
    for (int radius : CommonParameters::checkDistances()) {
        result.push_back(findTotalCount(radius));
    }
}

static std::vector<int> totalCounts = findTotalCounts();
int spreadRadiusTotalCount = findTotalCount(rules::creepSpreadRadius);

template<class T>
T sigmoidApproximation(T x) {
    return x / (1 + std::abs(x));
}

template<typename Predicate>
Weight calculateDistanceWeight(const Status& status, Point p,
        int distance, int total, const Predicate& predicate) {
    return static_cast<Weight>(countSpreadArea(getMax(status), p,
            distance, predicate)) / total * 10.0f;
}

template<typename Predicate>
Weight calculateDistanceWeight(const Status& status, Point p,
        std::size_t distanceIndex, const Predicate& predicate) {
    return calculateDistanceWeight(status, p,
            CommonParameters::checkDistances[distanceIndex],
            totalCounts[distanceIndex]);
}

Matrix<bool> getPotentialCreep(const Status& status) {
    Matrix<bool> result{status.width(), status.height()};
    for (Point p : matrixRange(result)) {
        result[p] = status.isCreep(p);
    }
    for (const Tumor& tumor : tumors) {
        iterateSpreadArea(getMax(status), tumor.position,
                rules::creepSpreadRadius,
                [&result](Point p) {
                    result[p] = true;
                });
    }
}

auto getTumorSpreadPositions(const Status& status) {
    Matrix<std::vector<const Tumor*>> result{
            status.width(), status.height(), false};
    for (const Tumor& tumor : tumors) {
        iterateSpreadArea(getMax(status), tumor.position,
                rules::creepSpreadRadius,
                [&result, &tumor](Point p) {
                    if (status.isCreep(p)) {
                        result[p].push_back(tumor);
                    }
                });
    }
    return result;
}

struct GetDistanceSquare {
    GetDistanceSquare(Point p) : p(p) {}

    int operator()(const Tumor& tumor) {
        return distanceSquare(tumor.position, p);
    }
    Point p;
};

} // unnamed namespace

void AIGameManager::AIGameManager(const CommonParameters& commonParameters,
        const GameInfo& track) :
        commonParameters(std::move(commonParameters)),
        gameInfo{std::move(gameInfo)} {
};

void AIGameManager::init() {
    game = std::make_unique<Game>(gameInfo);
    initialFloorCount = game->getStatus().getFloorsRemaining();
}

void AIGameManager::run() {
    while (game->canContinue()) {
        tick();
    }
}

void AIGameManager::tick() {
    const auto& status = game->getStatus();
    Matrix<float> tableWeights = evaluateTable();

}

Matrix<float> AIGameManager::evaluateTable() {
    const auto& status = game->getStatus();
    potentialCreep = getPotentialCreep(status);
    Matrix<float> result{status.width(), status.height(), -1.0f};
    bool hasActiveQueen = std::any_of(
            status.getQueens().begin(), status.getQueens.end(),
            [](const Queen& queen) {
                return queen.energy > rules::queenEnertyRequirement;
            });

    for (Point p : matrixRange(result)) {
        if (hasActiveQueen ||
    }
}

Weights AIGameManager::callNeuralNetwork(Point base) {
    const auto& status = game->getStatus();
    Weights inputs;
    inputs.reserve(CommonParameters.inputNeuronCount());
    // what is there at different distances
    for (std::size_t i = 0; i < CommonParameters::checkDistances.size(); ++i) {
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(&Status::isWall)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(&Status::Creep)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(&Status::Floor)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                [&potentialCreep, base](Point p) {
                    return potentialCreep[p];
                }));
    }
    // How much the potential creep area is increased
    inputs.push_back(calculateDistanceWeight(status, base,
            rules::creepSpreadRadius, spreadRadiusTotalCount,
            [&potentialCreep, status, base](Point p) {
                return potentialCreep[p] && !status.isCreep(p);
            }));
    // game time
    inputs.push_back(static_cast<float>(status.getTime()) /
            game.getTimeLimit() * 20.0f);
    // Maximum queen energy
    inputs.push_back(std::max_element(status.queens.begin(),
            status.queens.end(),
            [](const Queen& lhs, const Queen& rhs) {
                return lhs.energy < rhs.energy;
            })->energy);
    // Remaining floors
    inputs.push_back(status.getFloorsRemaining() / initialFloorCount * 10.0f);
    // Distance of nearest tumor
    auto distances = status.getTumors() | boost::adaptors::transformed(
            GetDistanceSquare(base));
    inputs.push_back(*std::min_element(distances.begin(), distances.end()));
    assert(inputs.size() == CommonParameters::inputNeuronCount());
    return neuralNetwork.evaluateIput(inputs);
}
