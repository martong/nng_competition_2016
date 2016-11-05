#include "Solver.hpp"

#include "Constants.hpp"
#include "Log.hpp"

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
        addQueenAction(game.getStatus().getQueens()[0]);
        tick();
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
                for (const Tumor& tumor : game.getStatus().getTumors()) {
                    pendingPositions.erase(tumor.position);
                }
                for (const Tumor& tumor : game.getStatus().getTumors()) {
                    if (isTumorAddable(tumor)) {
                        LOG << "Adding action to tumor #" << tumor.id <<
                                "\n";
                        addTumorAction(tumor);
                        pendingTumors.insert(tumor.id);
                        didSomething = true;

                    } else if (tumor.cooldown < 0) {
                        LOG << "Found inactive tumor: #" << tumor.id <<
                                "\n";
                        pendingTumors.erase(tumor.id);
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
                pendingTumors.find(tumor.id) == pendingTumors.end();
    }

    bool isQueenAddable(const Queen& queen) {
        return queen.energy >= rules::queenEnertyRequirement;
    }

    auto notPendingPredicate(bool (Status::*function)(Point) const) {
        return
                [this, function](const Status& status, Point p) {
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
        for (Point p : game.getStatus().getSpreadArea(
                    tumor.position, rules::creepSpreadRadius,
                    notPendingPredicate(&Status::isCreep))) {
            heuristicsTable[p].time = game.getStatus().getTime();
        }
        auto spreadPoints = game.getStatus().getSpreadArea(
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(&Status::isFloor));
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
            spreadPoints = newSpreadPoints;
        }
        for (Point p : gameTmp.getStatus().getSpreadArea(
                tumor.position, rules::creepSpreadRadius,
                getPredicate(&Status::isCreep))) {
            float newSpreadSize = gameTmp.getStatus().getSpreadArea(p,
                    rules::creepSpreadRadius,
                    getPredicate(&Status::isFloor)).size();
            heuristicsTable[p].value = newSpreadSize *
                    heuristics.spreadRadiusMultiplier +
                    calculateDistanceValue(p) +
                    (heuristicsTable[p].time - startTime + 1) *
                    heuristics.timeMultiplier;
        }
        //dumpMatrix(LOG, heuristicsTable, "", 0,
                //[](const HeuristicsData& data) {
                    //return (boost::format("%1.2f") % data.value).str();
                //});
        //dumpMatrix(LOG, heuristicsTable, "", 0,
                //[](const HeuristicsData& data) {
                    //return std::to_string(data.time);
                //});
        PointRange range{
                Point{std::max<int>(tumor.position.x -
                              rules::creepSpreadRadius + 1, 0),
                      std::max<int>(tumor.position.y -
                              rules::creepSpreadRadius + 1, 0)},
                Point{std::min<int>(tumor.position.x +
                              rules::creepSpreadRadius,
                              game.getStatus().width() - 1),
                      std::min<int>(tumor.position.y +
                              rules::creepSpreadRadius,
                              game.getStatus().height() - 1)}};
        Point bestPoint = *std::max_element(range.begin(), range.end(),
                    [&heuristicsTable](Point lhs, Point rhs) {
                        return heuristicsTable[lhs] < heuristicsTable[rhs];
                    });
        if (heuristicsTable[bestPoint].time != 0) {
            addCommand(heuristicsTable[bestPoint].time,
                    CommandType::PlaceTumorFromTumor, tumor.id, bestPoint);
        }
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
            currentNode = std::make_shared<Node>(it->second, game.getStatus(),
                    currentNode);
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
                spreadPossibilities[p] = status.getSpreadArea(p,
                        rules::creepSpreadRadius,
                        getPredicate(&Status::isFloor)).size() *
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
        addCommand(game.getStatus().getTime(),
                CommandType::PlaceTumorFromQueen, queen.id, bestPoint);
    }

    void addCommand(int time, CommandType type, int id, Point p) {
        LOG << "Adding command: time=" << time << " type=" << type <<
                " id=" << id << " position=" << p << "\n";
        assert(pendingPositions.find(p) == pendingPositions.end());
        assert(time >= game.getStatus().getTime());
        removeCommandsAfter(time);
        game.addCommand({time, type, id, p});
        pendingPositions.insert(p);
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
            pendingTumors.erase(command.id);
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
    boost::container::flat_set<int> pendingTumors;
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
