#include "AIGameManager.hpp"

#include "Constants.hpp"
#include "Spread.hpp"
#include "Status.hpp"
#include "Log.hpp"

#include <boost/range/adaptor/transformed.hpp>

#include <sstream>

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

    using result_type = int;

    int operator()(const Tumor& tumor) const {
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
    const auto& status = game->getStatus();
    initialFloorCount = status.getFloorsRemaining();
    potentialCreep.reset(status.width(), status.height(), false);
    areaExpiration.reset(status.width(), status.height(), 0);
    neuronActivity.reset(status.width(), status.height());
}

void AIGameManager::run() {
    LOG << "Start simulation\n";
    while (game->hasTime() && game->getStatus().getFloorsRemaining() > 0) {
        tick();
    }

}
void AIGameManager::tick() {
    LOG << "Tick " << game->getStatus().getTime() << "\n";
    //game->print(LOG);
    pendingPoints.clear();
    pendingTumors.clear();
    pendingQueens.clear();
    do {
        calculatePotentialCreep();
        calculateExpriation();
    } while (addCommandIfPossible());
    game->tick();
}

bool AIGameManager::addCommandIfPossible() {
    const auto& status = game->getStatus();
    auto tumorSpreadPositions = getTumorSpreadPositions();
    neuronActivity = evaluateTable(tumorSpreadPositions);
    std::vector<Point> candidates;
    {
        Point p;
        for (p.y = 0; p.y < static_cast<int>(status.height()); ++p.y) {
            for (p.x = 0; p.x < static_cast<int>(status.width()); ++p.x) {
                if (neuronActivity[p].activity > 0.0f &&
                        status.isCreep(p) && pendingPoints.count(p) == 0) {
                    candidates.push_back(p);
                }
            }
        };
    }
    LOG << "Number of candidates: " << candidates.size() << "\n";
    std::sort(candidates.begin(), candidates.end(),
            [this](Point lhs, Point rhs) {
                return neuronActivity[lhs].activity >
                        neuronActivity[rhs].activity;
            });
    for (Point p : candidates) {
        //LOG << "Candidate point " << p <<
                //" activity=" << neuronActivity[p].activity << "\n";
        if (neuronActivity[p].activity <= 0.0f) {
            return false;
        }
        for (const Tumor* tumor : tumorSpreadPositions[p]) {
            if (pendingTumors.count(tumor) != 0) {
                continue;
            }
            LOG << "Adding from tumor " << tumor->id << " " <<
                    tumor->position << "\n";
            pendingTumors.insert(tumor);
            addCommand({status.getTime(),
                    CommandType::PlaceTumorFromTumor, tumor->id, p});
            return true;
        }
        std::vector<const Queen*> queens(status.getQueens().size());
        std::transform(status.getQueens().begin(), status.getQueens().end(),
                queens.begin(),
                [](const Queen& queen) { return &queen; });
        std::sort(queens.begin(), queens.end(),
                [](const Queen* lhs, const Queen* rhs) {
                    return lhs->energy < rhs->energy;
                });
        for (const Queen* queen : queens) {
            if (queen->energy >= rules::queenEnertyRequirement) {
                if (pendingQueens.count(queen) != 0) {
                    continue;
                }
                LOG << "Adding from queen " << queen->id <<
                        " energy=" << queen->energy << "\n";
                pendingQueens.insert(queen);
                addCommand({status.getTime(),
                        CommandType::PlaceTumorFromQueen, queen->id, p});
                return true;
            }
        }
    }
    return false;
}

auto AIGameManager::evaluateTable(
        Matrix<std::vector<const Tumor*>> tumorSpreadPositions) ->
        Matrix<NeuronActivity> {
    const auto& status = game->getStatus();
    Matrix<NeuronActivity> result{status.width(), status.height()};
    bool hasActiveQueen = std::any_of(
            status.getQueens().begin(), status.getQueens().end(),
            [](const Queen& queen) {
                return queen.energy >= rules::queenEnertyRequirement;
            });

    Point p;
    for (p.y = 0; p.y < static_cast<int>(status.height()); ++p.y) {
        for (p.x = 0; p.x < static_cast<int>(status.width()); ++p.x) {
            if (status.isCreep(p) && areaExpiration[p] >= 0 &&
                    (hasActiveQueen || !tumorSpreadPositions[p].empty())) {
                result[p] = callNeuralNetwork(p);
            }
        }
    }
    return result;
}

auto AIGameManager::callNeuralNetwork(Point base) -> NeuronActivity {
    const auto& status = game->getStatus();
    Weights inputs;
    inputs.reserve(CommonParameters::inputNeuronCount());
    // what is there at different distances
    for (std::size_t i = 0; i < CommonParameters::checkDistances().size(); ++i) {
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isWall)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isCreep)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                getPredicate(status, &Status::isFloor)));
        inputs.push_back(calculateDistanceWeight(status, base, i,
                [this, base](Point p) {
                    return potentialCreep[p];
                }));
    }
    // How much the potential creep area is increased
    inputs.push_back(calculateDistanceWeight(status, base,
            rules::creepSpreadRadius, spreadRadiusTotalCount,
            [this, status, base](Point p) {
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

void AIGameManager::calculatePotentialCreep() {
    const auto& status = game->getStatus();
    Point p;
    for (p.y = 0; p.y < static_cast<int>(status.height()); ++p.y) {
        for (p.x = 0; p.x < static_cast<int>(status.width()); ++p.x) {
            potentialCreep[p] = status.isCreep(p);
        }
    }
    auto setToTrue = [this](Point p) { potentialCreep[p] = true; };
    for (const Tumor& tumor : status.getTumors()) {
        iterateSpreadArea(getMax(status), tumor.position,
                rules::creepSpreadRadius, setToTrue);
    }
    for (Point p : pendingPoints) {
        iterateSpreadArea(getMax(status), p, rules::creepSpreadRadius,
                setToTrue);
    }
}

auto AIGameManager::getTumorSpreadPositions() -> TumorSpreadPositions {
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

float AIGameManager::getFitness() const {
    const auto& status = game->getStatus();
    return static_cast<float>(game->getTimeLimit() - status.getTime()) /
            game->getTimeLimit() * commonParameters.fitnessTimeRemainingMultiplier +
            static_cast<float>(initialFloorCount - status.getFloorsRemaining()) /
            initialFloorCount * commonParameters.fitnessFloorsCoveredMultiplier;
}

std::string AIGameManager::getDebugInfo() const {
    const auto& status = game->getStatus();
    std::ostringstream ss;
    ss << "time=" << status.getTime() <<
            " floors=" << status.getFloorsRemaining();
    return ss.str();
}

void AIGameManager::calculateExpriation() {
    const auto& status = game->getStatus();
    Point p;
    for (p.y = 0; p.y < static_cast<int>(status.height()); ++p.y) {
        for (p.x = 0; p.x < static_cast<int>(status.width()); ++p.x) {
            if (areaExpiration[p] > 0) {
                --areaExpiration[p];
                continue;
            }
            if (areaExpiration[p] == 0 && status.isCreep(p)) {
                std::size_t count = countSpreadArea(getMax(status), p,
                        rules::creepSpreadRadius,
                        getPredicate(status, &Status::isFloor));
                areaExpiration[p] = count - 1;
            }
        }
    }
}


void AIGameManager::addCommand(const Command& command) {
    LOG << "Adding command: time=" << command.time <<
            " type=" << command.type <<
            " id=" << command.id <<
            " position=" << command.position << "\n";
    game->addCommand(command);
    pendingPoints.insert(command.position);
}

std::vector<Command> AIGameManager::getCommands() const {
    std::vector<Command> result;
    std::transform(game->getCommands().begin(), game->getCommands().end(),
            std::back_inserter(result),
            [](const auto& element) { return element.second; });
    return result;
}
