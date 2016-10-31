#include "Solver.hpp"

#include "Constants.hpp"
#include "Creep.hpp"
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
    SolverImpl(Game& game, const Heuristics& heuristics) :
            game(game), heuristics(heuristics) {
    }

    std::vector<Command> solve() {
        addQueenAction(game.getStatus().getQueens()[0]);
        game.tick();
        doSolve();
        std::vector<Command> result;
        result.reserve(game.getCommands().size());
        for (const auto& element : game.getCommands()) {
            result.push_back(element.second);
        }
        return result;
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

    template<typename Predicate>
    auto notPendingPredicate(const Predicate& predicate) {
        return
                [this, &predicate](const Table& table, Point p) {
                    return predicate(table, p) && isNotPending(p);
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
                game.getStatus().getTable().width(),
                game.getStatus().getTable().height()};
        for (Point p : getSpreadArea(game.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(isCreep))) {
            heuristicsTable[p].time = game.getStatus().getTime();
        }
        auto spreadPoints = getSpreadArea(game.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(isFloor));
        Game gameTmp = game;
        while (gameTmp.canContinue()) {
            gameTmp.tick();
            std::vector<Point> newSpreadPoints;
            for (Point p : spreadPoints) {
                if (gameTmp.getStatus().getTable()[p] == MapElement::Creep) {
                    heuristicsTable[p].time = gameTmp.getStatus().getTime();
                } else if (gameTmp.getStatus().getTable()[p] ==
                        MapElement::Floor) {
                    newSpreadPoints.push_back(p);
                }
            }
            spreadPoints = newSpreadPoints;
        }
        for (Point p : getSpreadArea(gameTmp.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius, isCreep)) {
            float newSpreadSize = getSpreadArea(gameTmp.getStatus().getTable(), p,
                    rules::creepSpreadRadius, isFloor).size();
            float distanceValue = 0.0f;
            for (const Tumor& tumor : gameTmp.getStatus().getTumors()) {
                distanceValue += heuristics.distanceSquareMultiplier /
                        distanceSquare(p, tumor.position);
            }
            for (Point pendingPosition : pendingPositions) {
                distanceValue += heuristics.distanceSquareMultiplier /
                        distanceSquare(p, pendingPosition);
            }
                heuristicsTable[p].value = newSpreadSize *
                    heuristics.spreadRadiusMultiplier +
                    distanceValue +
                    (heuristicsTable[p].time - startTime + 1) *
                    heuristics.timeMultiplier;
        }
        //dumpMatrix(LOG, game.getStatus().getTable());
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
                              game.getStatus().getTable().width() - 1),
                      std::min<int>(tumor.position.y +
                              rules::creepSpreadRadius,
                              game.getStatus().getTable().height() - 1)}};
        Point bestPoint = *std::max_element(range.begin(), range.end(),
                    [&heuristicsTable](Point lhs, Point rhs) {
                        return heuristicsTable[lhs] < heuristicsTable[rhs];
                    });
        if (heuristicsTable[bestPoint].time != 0) {
            addCommand(heuristicsTable[bestPoint].time,
                    CommandType::PlaceTumorFromTumor, tumor.id, bestPoint);
        }
    }

    void tick() {
        game.tick();
        if (game.getStatus().getFloorsRemaining() == 0) {
            throw Finished{};
        }
    }

    void addQueenAction(const Queen& queen) {
        const Table& table = game.getStatus().getTable();
        Matrix<std::size_t> spreadPossibilities{
                table.width(), table.height(), 0};
        std::vector<Point> candidates;
        for (Point p : matrixRange(table)) {
            if (table[p] == MapElement::Creep &&
                    isNotPending(p)) {
                spreadPossibilities[p] = getSpreadArea(table, p,
                        rules::creepSpreadRadius, isFloor).size();
                candidates.push_back(p);
            }
        }
        Point bestPoint = *std::max_element(candidates.begin(), candidates.end(),
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
    const Heuristics heuristics;
    boost::container::flat_set<int> pendingTumors;
    boost::container::flat_set<Point> pendingPositions;
};

} // unnamed namespace

Solution findSolution(Game game, const Heuristics& heuristics) {
    LOG << "Solve: tm=" << heuristics.timeMultiplier <<
            " dsm=" << heuristics.distanceSquareMultiplier <<
            " srm=" << heuristics.spreadRadiusMultiplier << "\n";
    Solution result;
    SolverImpl impl{game, heuristics};
    result.commands = impl.solve();
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
