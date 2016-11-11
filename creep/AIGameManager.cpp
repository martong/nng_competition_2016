#include "AIGameManager.hpp"

#include "Constants.hpp"
#include "Spread.hpp"
#include "Status.hpp"

#include <boost/range/adaptor/transformed.hpp>

namespace {

int findTotalCount(int radius) {
    Point dd{radius, radius};
    return countSpreadArea(dd * 2, dd, radius, alwaysTrue);
}

std::vector<int> findTotalCounts() {
    std::vector<int> result;
    for (int radius : CommonParameters::checkDistances()) {
        result.push_back(findTotalCount(radius));
    }
    return result;
}

static std::vector<int> totalCounts = findTotalCounts();
int spreadRadiusTotalCount = findTotalCount(rules::creepSpreadRadius);

template<typename Predicate>
Weight calculateDistanceWeight(const Status& status, Point p,
        int distance, int total, const Predicate& predicate) {
    return sigmoidApproximation(static_cast<Weight>(
            countSpreadArea(getMax(status), p, distance, predicate)) /
            total * 10.0f);
}

template<typename Predicate>
Weight calculateDistanceWeight(const Status& status, Point p,
        std::size_t distanceIndex, const Predicate& predicate) {
    return calculateDistanceWeight(status, p,
            CommonParameters::checkDistances()[distanceIndex],
            totalCounts[distanceIndex], predicate);
}

struct GetDistanceSquare {
    GetDistanceSquare(Point p) : p(p) {}

    int operator()(const Tumor& tumor) {
        return distanceSquare(tumor.position, p);
    }
    Point p;
};

} // unnamed namespace

AIGameManager::AIGameManager(const CommonParameters& commonParameters,
        const GameInfo& gameInfo) :
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
    potentialTumors.clear();
    while (Point p = addCommandIfPossible(tumorSpreadPositions)) {
        pendingTumors.insert(p);
    }
    game.tick();
}

boost::optional<Point> addCommandIfPossible() {
    auto tumorSpreadPositions = getTumorSpreadPositions(status);
    neuronActivity = evaluateTable(tumorSpreadPositions);
    auto range = matrixRange(neuronActivity);
    std::vector<Point> candidates;
    std::copy_if(range.begin(), range.end(), std::back_inserter(candidates),
            [this, &status](Point p) {
                return neuronActivity[p].activity > 0.0f &&
                        status.isCreep(p) && potentialTumors.count(p) == 0;
            });
    std::sort(candidates.begin(), candidates.end(),
            [this](Point lhs, Point rhs) {
                return neuronActivity[lhs].activity >
                        neuronActivity[rhs].activity;
            });
    for (Point p : candidates) {
        if (neuronActivity[p].activity <= 0.0f) {
            return;
        }
        for (const Tumor* tumor : tumorSpreadPositions[p]) {
            if (tumor->position == p) {
                game.addCommand({status.getTime(),
                        CommandType::PlaceTumorFromTumor, tumor->id, p});
                return p;
            }
        }
        std::vector<const Queen*> queens(status.getQueens().size());
        std::copy(status.getQueens().begin(), status.getQueens.end(),
                queens.begin());
        std::sort(queens.begin(), queens.end(),
                [](const Queen& lhs, const Queen& rhs) {
                    return lhs.energy < rhs.energy;
                });
        for (const Queen* queen : queens) {
            if (queen.energy > rules::queenEnertyRequirement) {
                game.addCommand({status.getTime(),
                        CommandType::PlaceTumorFromQueen, queen->id, p});
                return p;
            }
        }
    }
    return boost::none;
}

auto AIGameManager::evaluateTable(
        Matrix<std::vector<const Tumor*>> tumorSpreadPositions) ->
        Matrix<NeuronActivity> {
    const auto& status = game->getStatus();
    Matrix<NeuronActivity> result{status.width(), status.height()};
    bool hasActiveQueen = std::any_of(
            status.getQueens().begin(), status.getQueens().end(),
            [](const Queen& queen) {
                return queen.energy > rules::queenEnertyRequirement;
            });

    for (Point p : matrixRange(result)) {
        if (status.isCreep(p) && (hasActiveQueen ||
                !tumorSpreadPositions[p].empty())) {
            result[p] = callNeuralNetwork(p);
        }
    }
    return result;
}

auto AIGameManager::callNeuralNetwork(Point base) -> NeuronActivity {
    const auto& status = game->getStatus();
    Weights inputs;
    inputs.reserve(CommonParameters::inputNeuronCount());
    auto potentialCreep = getPotentialCreep(status);
    // what is there at different distances
    for (std::size_t i = 0; i < CommonParameters::checkDistances().size(); ++i) {
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isWall)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isCreep)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isFloor)));
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
    inputs.push_back(sigmoidApproximation(static_cast<float>(status.getTime()) /
            game->getTimeLimit() * 20.0f));
    // Maximum queen energy
    inputs.push_back(sigmoidApproximation(std::max_element(
            status.getQueens().begin(), status.getQueens().end(),
            [](const Queen& lhs, const Queen& rhs) {
                return lhs.energy < rhs.energy;
            })->energy));
    // Remaining floors
    inputs.push_back(sigmoidApproximation(status.getFloorsRemaining() /
            initialFloorCount * 10.0f));
    // Distance of nearest tumor
    auto distances = status.getTumors() | boost::adaptors::transformed(
            GetDistanceSquare(base));
    inputs.push_back(sigmoidApproximation(*std::min_element(
                distances.begin(), distances.end())));
    // Inputs from previous round
    inputs.push_back(neuronActivity[base].feedCurrent);
    inputs.push_back((
            matrixAt(neuronActivity, base + p10, {}).feedNeighbor +
            matrixAt(neuronActivity, base - p10, {}).feedNeighbor +
            matrixAt(neuronActivity, base + p01, {}).feedNeighbor +
            matrixAt(neuronActivity, base - p01, {}).feedNeighbor) / 4.0f);
    assert(inputs.size() == CommonParameters::inputNeuronCount());
    auto result = neuralNetwork.evaluateInput(inputs);
    assert(result.size() == CommonParameters::outputNeuronCount());
    return {result[CommonParameters::outputNeuronActivity],
            result[CommonParameters::outputNeuronFeedCurrent],
            result[CommonParameters::outputNeuronFeedNeighbor]};
}

Matrix<bool> AIGameManager::getPotentialCreep() {
    const auto& status = game->getStatus();
    Matrix<bool> result{status.width(), status.height()};
    for (Point p : matrixRange(result)) {
        result[p] = status.isCreep(p);
    }
    auto setToTrue = [&result](Point p) { result[p] = true; };
    for (const Tumor& tumor : status.getTumors()) {
        iterateSpreadArea(getMax(status), tumor.position,
                rules::creepSpreadRadius, setToTrue);
    }
    for (Point p : potentialTumors) {
        iterateSpreadArea(getMax(status), p, rules::creepSpreadRadius,
                setToTrue);
    }
    return result;
}

TumorSpreadPositions AIGameManager::getTumorSpreadPositions() {
    const auto& status = game->getStatus();
    Matrix<std::vector<const Tumor*>> result{
            status.width(), status.height()};
    for (const Tumor& tumor : status.getTumors()) {
        if (tumor.cooldown == 0) {
            iterateSpreadArea(getMax(status), tumor.position,
                    rules::creepSpreadRadius,
                    [&result, &tumor, &status](Point p) {
                        if (status.isCreep(p)) {
                            result[p].push_back(&tumor);
                        }
                    });
        }
    }
    return result;
}

