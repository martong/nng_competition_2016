#include "Solver.hpp"

#include "Constants.hpp"
#include "Log.hpp"

#include <DumperFunctions.hpp>

#include <boost/asio/strand.hpp>
#include <boost/format.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/min_element.hpp>

#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <random>

namespace {

struct Finished {};

class SolverImpl {
public:
    SolverImpl(const Game& game, const Matrix<float>& heatMap,
            const Solver::Heuristics& heuristics, RandomGenerator& rng) :
            game(game), heatMap(heatMap), heuristics(heuristics), rng(rng) {
    }

    Solver::Solution solve() {
        addQueenAction(game.getStatus().getQueens()[0]);
        tick();
        doSolve();
        Solver::Solution solution;
        std::transform(game.getCommands().begin(), game.getCommands().end(),
                std::back_inserter(solution.commands),
                [](const auto& element) { return element.second; });
        solution.floorsRemaining = game.getStatus().getFloorsRemaining();
        solution.time = game.getStatus().getTime();
        return solution;
    }

private:
    struct CalculatedData {
        CalculatedData(const Game& game, const Matrix<float>& heatMap) :
                game(game), heatMap(heatMap) {
        }

        Game game;
        Matrix<float> heatMap;
    };

    void doSolve() {
        std::size_t iterations = 0;
        bool didSomething = true;
        while (didSomething && game.getStatus().getFloorsRemaining() != 0 &&
                game.hasTime()) {
            try {
                didSomething = false;
                LOG << "Iteration: " << iterations << "\n";
                forwardToNextAvailableTumor();
                LOG << "Moved to time: " << game.getStatus().getTime() << "\n";
                LOG << "Pending actions: ";
                for (int id : pendingActions) {
                    LOG << id << " ";
                }
                LOG << "\nPending positions: ";
                for (Point p : pendingPositions) {
                    LOG << p << " ";
                }
                LOG << "\n";
                for (const Tumor& tumor : game.getStatus().getTumors()) {
                    if (isTumorAddable(tumor)) {
                        LOG << "Adding action to tumor #" << tumor.id <<
                                "\n";
                        addTumorAction(tumor);
                        didSomething = true;

                    } else if (tumor.cooldown < 0) {
                        LOG << "Found inactive tumor: #" << tumor.id <<
                                "\n";
                    }
                }
                for (const Queen queen: game.getStatus().getQueens()) {
                    if (isQueenAddable(queen)) {
                        LOG << "Adding action to queen #" << queen.id <<
                                "\n";
                        addQueenAction(queen);
                        didSomething = true;
                    }
                }
                tick();
            } catch (Finished&) {
                LOG << "Done.\n";
            }
            ++iterations;
        }
    }

    void forwardToNextAvailableTumor() {
        while (
                !std::any_of(game.getStatus().getTumors().begin(),
                        game.getStatus().getTumors().end(),
                        [this](const Tumor& tumor) {
                            return isTumorAddable(tumor);
                        }) &&
                !std::any_of(game.getStatus().getQueens().begin(),
                        game.getStatus().getQueens().end(),
                        [this](const Queen& queen) {
                            return isQueenAddable(queen);
                        }) && game.canContinue()) {
            tick();
        }
    }

    bool isTumorAddable(const Tumor& tumor) {
        return tumor.cooldown == 0 &&
                pendingActions.find(tumor.id) == pendingActions.end();
    }

    bool isQueenAddable(const Queen& queen) {
        return queen.energy >= rules::queenEnertyRequirement &&
                pendingActions.find(queen.id) == pendingActions.end();

    }

    auto notPendingPredicate(bool (Status::*function)(Point) const) {
        return
                [this, function](const Status& status, Point p) {
                    return (status.*function)(p) && isNotPending(p);
                };
    }

    void addTumorAction(const Tumor& tumor) {
        auto calculatedData = calculateData();
        auto consideredPoints = calculatedData.game.getStatus().getSpreadArea(
                tumor.position, rules::creepSpreadRadius,
                getPredicate(&Status::isCreep));
        if (consideredPoints.empty()) {
            LOG << "Tumor " << tumor.id <<
                    " surrounded, cannot add more tumors.";
            pendingActions.insert(tumor.id);
            return;
        }
        Point bestPoint = findBestPoint(calculatedData, consideredPoints);
        addCommand({
                std::max(game.getStatus().getTime(),
                        calculatedData.game.getStatus().creepTime(bestPoint)),
                CommandType::PlaceTumorFromTumor, tumor.id, bestPoint});
    }

    void tick() {
        auto its = game.getCommands().equal_range(game.getStatus().getTime());
        for (auto it = its.first; it != its.second; ++it) {
            const Command& command = it->second;
            pendingActions.erase(command.id);
            pendingPositions.erase(command.position);
        }
        game.tick();
        if (game.getStatus().getFloorsRemaining() == 0) {
            throw Finished{};
        }
    }

    void addQueenAction(const Queen& queen) {
        auto calculatedData = calculateData();
        Point bestPoint = findBestPoint(calculatedData, matrixRange(heatMap));
        addCommand({
                std::max(game.getStatus().getTime(),
                        calculatedData.game.getStatus().creepTime(bestPoint)),
                CommandType::PlaceTumorFromQueen, queen.id, bestPoint});
    }

    CalculatedData calculateData() {
        CalculatedData calculatedData{game,
                Matrix<float>{heatMap.width(), heatMap.height(), 0.0f}};
        while(calculatedData.game.canContinue()) {
            calculatedData.game.tick();
        }
        const Status& status = calculatedData.game.getStatus();
        for (Point p : matrixRange(heatMap)) {
            auto spreadPoints = game.getStatus().getSpreadArea(
                    p, rules::creepSpreadRadius,
                    notPendingPredicate(&Status::isFloor));
            if (status.isCreep(p)) {
                int minDistance = std::numeric_limits<int>::max();
                for (Point pendingPosition : pendingPositions) {
                    minDistance = std::min(minDistance,
                            distanceSquare(pendingPosition, p));

                }
                for (const Tumor& tumor : game.getStatus().getTumors()) {
                    minDistance = std::min(minDistance,
                            distanceSquare(tumor.position, p));

                }
                calculatedData.heatMap[p] = heatMap[p] +
                        heuristics.spreadRadiusMultiplier *
                                spreadPoints.size() -
                        heuristics.timeMultiplier * std::max(0,
                                status.creepTime(p) -
                                game.getStatus().getTime()) -
                        heuristics.minimumDistanceMultiplier * minDistance;
            }
        }
        return calculatedData;
    }

    template<typename Range>
    Point findBestPoint(const CalculatedData& calculatedData,
            const Range& range) {
        std::vector<Point> points;
        std::copy_if(range.begin(), range.end(), std::back_inserter(points),
                [&calculatedData](Point p) {
                    return calculatedData.heatMap[p] != 0;
                });
        auto weights = points | boost::adaptors::transformed(
                [&calculatedData](Point p) {
                    return calculatedData.heatMap[p];
                });

        std::discrete_distribution<std::size_t> distribution{weights.begin(),
                weights.end()};
        Point point = points[distribution(rng)];
        LOG << "Using point " << point <<
                ": score=" << calculatedData.heatMap[point] <<
                " creepTime=" << calculatedData.game.getStatus().
                creepTime(point) << "\n";
        return point;
    }

    void addCommand(const Command& command) {
        LOG << "Adding command: time=" << command.time <<
                " type=" << command.type <<
                " id=" << command.id <<
                " position=" << command.position << "\n";
        assert(pendingPositions.find(command.position) ==
                pendingPositions.end());
        assert(command.time >= game.getStatus().getTime());
        removeCommandsAfter(command.time);
        game.addCommand(command);
        pendingActions.insert(command.id);
        pendingPositions.insert(command.position);
    }

    void removeCommandsAfter(int time) {
        const auto& commands = game.getCommands();
        auto from = commands.upper_bound(time);
        auto to = commands.end();
        for (const auto& element : boost::make_iterator_range(from, to)) {
            const Command& command = element.second;
            LOG << "Removing command: time=" << command.time <<
                    " type=" << command.type << " id=" << command.id <<
                    " position=" << command.position << "\n";
            pendingActions.erase(command.id);
            pendingPositions.erase(command.position);
        }
        game.removeCommands(from, to);
    }

    bool isNotPending(Point p) {
        return pendingPositions.find(p) == pendingPositions.end();
    }

    Game game;
    const Matrix<float>& heatMap;
    const Solver::Heuristics& heuristics;
    boost::container::flat_set<int> pendingActions;
    boost::container::flat_set<Point> pendingPositions;
    RandomGenerator& rng;
};

} // unnamed namespace

auto Solver::iterate(boost::asio::io_service& ioService) -> Solution {
    std::mutex mutex;
    std::condition_variable conditionVariable;
    boost::asio::strand strand{ioService};
    std::vector<Solution> solutions;
    solutions.reserve(parameters.runsPerIteration);
    auto onFinished =
            [&solutions, &conditionVariable, &mutex](const Solution& solution) {
                std::cerr << ".";
                std::unique_lock<std::mutex> lock{mutex};
                solutions.push_back(solution);
                conditionVariable.notify_one();
            };
    for (std::size_t i = 0; i < parameters.runsPerIteration; ++i) {
            ioService.post([this, &ioService, &strand, &onFinished]() {
                auto solution = SolverImpl{
                        initialGame, heatMap, heuristics, rng}.solve();
                solution.score = - solution.floorsRemaining *
                        parameters.floorPenalty +
                        (initialGame.getTimeLimit() - solution.time) *
                        parameters.timeScore;
                ioService.post(strand.wrap(
                        [&onFinished, solution]() { onFinished(solution); }));
            });
    }
    {
        std::unique_lock<std::mutex> lock{mutex};
        while (solutions.size() < parameters.runsPerIteration) {
            conditionVariable.wait(lock);
        }
    }
    std::cerr << "\n";
    for (const Solution& solution : solutions) {
        LOG << "Solution: floors=" << solution.floorsRemaining <<
                " time=" << solution.time << " score=" << solution.score <<
                "\n";
        updateHeatMap(solution);
    }
    coolDownHeatMap();
    return *std::max_element(solutions.begin(), solutions.end(),
            [](const Solution& lhs, const Solution& rhs) {
                return lhs.score < rhs.score;
            });
}

void Solver::updateHeatMap(const Solution& solution) {
    for (const Command& command : solution.commands) {
        for (Point p : initialGame.getStatus().getSpreadArea(command.position,
                    parameters.heatFlowMaxDistance,
                    getNegativePredicate(&Status::isWall))) {
            if (p == command.position) {
                heatMap[p] += solution.score;
            } else {
                heatMap[p] += solution.score * parameters.heatFlowStrength /
                        distanceSquare(p, command.position);
            }
        }
    }
    //dumpMatrix(std::cerr, heatMap);
}

void Solver::coolDownHeatMap() {
    for (float& heat : heatMap) {
        heat *= parameters.cooldownFactor;
    }
}
