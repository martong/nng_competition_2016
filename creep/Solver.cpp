#include "Solver.hpp"

#include "Constants.hpp"
#include "Log.hpp"
#include "Spread.hpp"

#include <DumperFunctions.hpp>

#include <boost/format.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>

namespace {

struct Finished {};

class SolverImpl {
public:
    SolverImpl(Game& game, const Heuristics& heuristics,
            std::shared_ptr<Node> startingNode) :
            game(game), currentNode(std::move(startingNode)),
            heuristics(heuristics) {
    }

    std::shared_ptr<Node> solve() {
        if (!currentNode) {
            addQueenAction(game.getStatus().getQueens()[0]);
            tick();
        } else {
            for (auto node = currentNode;
                    node && node->command.time == game.getStatus().getTime();
                    node = node->ancestor) {
                addCommand(node->command);
            }
        }
        doSolve();
        return currentNode;
    }

private:
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

    auto notPendingPredicate(const Status& status,
            bool (Status::*function)(Point) const) {
        return
                [this, function, &status](Point p) {
                    return (status.*function)(p) && isNotPending(p);
                };
    }

    void addTumorAction(const Tumor& tumor) {
        struct HeuristicsData {
            float value = 0.0;
            int time = 0;

            bool operator<(const HeuristicsData& other) {
                return value < other.value || (
                        value == other.value &&
                        time == other.time);
            }
        };

        int startTime = game.getStatus().getTime();
        Matrix<HeuristicsData> heuristicsTable{
                game.getStatus().width(), game.getStatus().height()};
        iterateSpreadArea(getMax(game.getStatus()), tumor.position,
                rules::creepSpreadRadius,
                [&heuristicsTable, this](Point p) {
                    if (game.getStatus().isCreep(p) && isNotPending(p)) {
                        heuristicsTable[p].time = game.getStatus().getTime();
                    }
                });
        auto spreadPoints = findSpreadArea(getMax(game.getStatus()),
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(game.getStatus(), &Status::isFloor));
        Game gameTmp = game;
        while (gameTmp.canContinue()) {
            gameTmp.tick();
            std::vector<Point> newSpreadPoints;
            for (Point p : spreadPoints) {
                if (gameTmp.getStatus().isCreep(p)) {
                    heuristicsTable[p].time = gameTmp.getStatus().getTime();
                } else if (gameTmp.getStatus().isFloor(p)) {
                    newSpreadPoints.push_back(p);
                }
            }
            spreadPoints = std::move(newSpreadPoints);
        }
        auto consideredPoints = findSpreadArea(getMax(game.getStatus()),
                tumor.position, rules::creepSpreadRadius,
                getPredicate(gameTmp.getStatus(), &Status::isCreep));
        if (consideredPoints.empty()) {
            LOG << "Tumor " << tumor.id <<
                    " surrounded, cannot add more tumors.";
            pendingActions.insert(tumor.id);
            return;
        }
        for (Point p : consideredPoints) {
            float newSpreadSize = countSpreadArea(getMax(game.getStatus()), p,
                    rules::creepSpreadRadius,
                    getPredicate(gameTmp.getStatus(), &Status::isFloor));
            heuristicsTable[p].value = newSpreadSize *
                    heuristics.spreadRadiusMultiplier +
                    calculateDistanceValue(p) +
                    (heuristicsTable[p].time - startTime + 1) *
                    heuristics.timeMultiplier;
        }

        Point bestPoint = *std::max_element(
                consideredPoints.begin(), consideredPoints.end(),
                [&heuristicsTable](Point lhs, Point rhs) {
                    return heuristicsTable[lhs] < heuristicsTable[rhs];
                });
        assert(heuristicsTable[bestPoint].time != 0);
        addCommand({heuristicsTable[bestPoint].time,
                CommandType::PlaceTumorFromTumor, tumor.id, bestPoint});
    }

    float calculateDistanceValue(Point p) {
        float distanceValue = 0.0f;
        for (const Tumor& tumor : game.getStatus().getTumors()) {
            distanceValue += heuristics.distanceSquareMultiplier /
                    distanceSquare(p, tumor.position);
        }
        for (Point pendingPosition : pendingPositions) {
            distanceValue += heuristics.distanceSquareMultiplier /
                    distanceSquare(p, pendingPosition);
        }
        return distanceValue;
    }

    void tick() {
        auto its = game.getCommands().equal_range(game.getStatus().getTime());
        for (auto it = its.first; it != its.second; ++it) {
            LOG << "Setting new node: time=" << game.getStatus().getTime() <<
                    "\n";
            const Command& command = it->second;
            currentNode = std::make_shared<Node>(command, game.getStatus(),
                    currentNode);
            pendingActions.erase(command.id);
            pendingPositions.erase(command.position);
        }
        game.tick();
        if (game.getStatus().getFloorsRemaining() == 0) {
            throw Finished{};
        }
    }

    void addQueenAction(const Queen& queen) {
        const Status& status = game.getStatus();
        Matrix<float> spreadPossibilities{status.width(), status.height(), 0};
        std::vector<Point> candidates;
        for (Point p : matrixRange(spreadPossibilities)) {
            if (status.isCreep(p) && isNotPending(p)) {
                spreadPossibilities[p] = countSpreadArea(
                        getMax(game.getStatus()), p, rules::creepSpreadRadius,
                        getPredicate(status, &Status::isFloor)) *
                        heuristics.spreadRadiusMultiplier +
                        calculateDistanceValue(p);
                candidates.push_back(p);
            }
        }
        Point bestPoint = *std::max_element(
                candidates.begin(), candidates.end(),
                [&spreadPossibilities](const Point& lhs, const Point& rhs) {
                    return spreadPossibilities[lhs] < spreadPossibilities[rhs];
                });
        addCommand({game.getStatus().getTime(),
                CommandType::PlaceTumorFromQueen, queen.id, bestPoint});
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

    Game& game;
    std::shared_ptr<Node> currentNode;
    const Heuristics heuristics;
    boost::container::flat_set<int> pendingActions;
    boost::container::flat_set<Point> pendingPositions;
};

} // unnamed namespace

Solution findSolution(Game game, const Heuristics& heuristics,
        std::shared_ptr<Node> startingNode) {
    LOG << "Solve: tm=" << heuristics.timeMultiplier <<
            " dsm=" << heuristics.distanceSquareMultiplier <<
            " srm=" << heuristics.spreadRadiusMultiplier << "\n";
    Solution result;
    SolverImpl impl{game, heuristics, std::move(startingNode)};
    result.node = impl.solve();
    result.time = game.getStatus().getTime();
    result.floorsRemaining = game.getStatus().getFloorsRemaining();
    result.heuristics = heuristics;
    LOG << "Solution: tm=" << heuristics.timeMultiplier <<
            " dsm=" << heuristics.distanceSquareMultiplier <<
            " srm=" << heuristics.spreadRadiusMultiplier <<
            " floors=" << result.floorsRemaining <<
            " time=" << result.time << "\n";
    return result;
}
