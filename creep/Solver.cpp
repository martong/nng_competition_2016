#include "Solver.hpp"

#include "Constants.hpp"
#include "Creep.hpp"

#include <DumperFunctions.hpp>

#include <boost/format.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <algorithm>

namespace {

struct Finished {};

class SolverImpl {
public:
    SolverImpl(Game&& game) : game(std::move(game)) {
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
                std::cerr << "Iteration: " << iterations << "\n";
                forwardToNextAvailableTumor();
                std::cerr << "Moved to time: " << game.getStatus().getTime() << "\n";
                for (const Tumor& tumor : game.getStatus().getTumors()) {
                    pendingPositions.erase(tumor.position);
                    if (isTumorAddable(tumor)) {
                        std::cerr << "Adding action to tumor #" << tumor.id <<
                                "\n";
                        addTumorAction(tumor);
                        pendingTumors.insert(tumor.id);
                        didSomething = true;

                    } else if (tumor.cooldown < 0) {
                        std::cerr << "Found inactive tumor: #" << tumor.id <<
                                "\n";
                        pendingTumors.erase(tumor.id);
                    }
                }
                for (const Queen queen: game.getStatus().getQueens()) {
                    if (isQueenAddable(queen)) {
                        std::cerr << "Adding action to queen #" << queen.id <<
                                "\n";
                        addQueenAction(queen);
                        didSomething = true;
                    }
                }
                tick();
            } catch (Finished&) {
                std::cerr << "Done." << std::endl;
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
        struct VelocityData {
            float velocity = 0.0;
            int time = 0;

            bool operator<(const VelocityData& other) {
                return velocity < other.velocity || (
                        velocity == other.velocity &&
                        time == other.time);
            }
        };

        int startTime = game.getStatus().getTime();
        Matrix<VelocityData> velocities{game.getStatus().getTable().width(),
                game.getStatus().getTable().height()};
        for (Point p : getSpreadArea(game.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(isCreep))) {
            velocities[p].time = game.getStatus().getTime();
        }
        auto spreadPoints = getSpreadArea(game.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius,
                notPendingPredicate(isFloor));
        while (game.canContinue()) {
            tick();
            std::vector<Point> newSpreadPoints;
            for (Point p : spreadPoints) {
                if (game.getStatus().getTable()[p] == MapElement::Creep) {
                    velocities[p].time = game.getStatus().getTime();
                } else if (game.getStatus().getTable()[p] == MapElement::Floor) {
                    newSpreadPoints.push_back(p);
                }
            }
            spreadPoints = newSpreadPoints;
        }
        for (Point p : getSpreadArea(game.getStatus().getTable(),
                tumor.position, rules::creepSpreadRadius, isCreep)) {
            float newSpreadSize = getSpreadArea(game.getStatus().getTable(), p,
                    rules::creepSpreadRadius, isFloor).size();
            velocities[p].velocity = newSpreadSize / (
                    velocities[p].time - startTime + 1);
        }
        game.rewind(startTime);
        dumpMatrix(std::cerr, game.getStatus().getTable());
        dumpMatrix(std::cerr, velocities, "", 0,
                [](const VelocityData& data) {
                    return (boost::format("%1.2f") % data.velocity).str();
                });
        dumpMatrix(std::cerr, velocities, "", 0,
                [](const VelocityData& data) {
                    return std::to_string(data.time);
                });
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
                    [&velocities](Point lhs, Point rhs) {
                        return velocities[lhs] < velocities[rhs];
                    });
        if (velocities[bestPoint].time != 0) {
            addCommand(velocities[bestPoint].time,
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
        std::cerr << "Adding command: time=" << time << " type=" << type <<
                " id=" << id << " position=" << p << "\n";
        assert(pendingPositions.find(p) == pendingPositions.end());
        assert(time >= game.getStatus().getTime());
        game.addCommand({time, type, id, p});
        pendingPositions.insert(p);
    }

    bool isNotPending(Point p) {
        return pendingPositions.find(p) == pendingPositions.end();
    }

    Game game;
    boost::container::flat_set<int> pendingTumors;
    boost::container::flat_set<Point> pendingPositions;
};

} // unnamed namespace

std::vector<Command> findSolution(Game game) {
    return SolverImpl{std::move(game)}.solve();
}

