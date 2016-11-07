#ifndef CREEP_STATUS_HPP
#define CREEP_STATUS_HPP

#include "Table.hpp"

#include <vector>

struct Tumor {
    Tumor(int id, Point position, int cooldown) :
            id(id), position(position), cooldown(cooldown) {
    }

    int id;
    Point position;
    int cooldown; // >0: cooldown period, 0: active: <0: inactive
};

struct Queen {
    Queen(int id, int energy) : id(id), energy(energy) {
    }

    int id;
    int energy;
};

class Status {
public:
    Status() = default;

    Status(std::istream& stream, std::size_t width, std::size_t height);

    Status(const Status&) = default;
    Status(Status&&) = default;
    Status& operator=(const Status&) = default;
    Status& operator=(Status&&) = default;

    void tick();
    const Tumor& addTumorFromQueen(int id, Point position);
    const Tumor& addTumorFromTumor(int id, Point position);

    std::size_t width() const { return table.width(); }
    std::size_t height() const { return table.width(); }
    int creepTime(Point p) const { return table[p]; }
    bool isFloor(Point p) const {
        return table[p] == MapElement::Floor || table[p] > time;
    }
    bool isBuilding(Point p) const { return table[p] == MapElement::Building; }
    bool isWall(Point p) const { return table[p] == MapElement::Wall; }
    bool isCreepAt(Point p, int t) const {
        return table[p] >= 0 && table[p] <= t;
    }
    bool isCreep(Point p) const { return isCreepAt(p, time); }
    bool isCreepCandidate(Point p) const {
        return table[p] == MapElement::Floor && (
               hasCreep(table[p - p10]) || hasCreep(table[p + p10]) ||
               hasCreep(table[p - p01]) || hasCreep(table[p + p01]));
    }

    const std::vector<Tumor>& getTumors() const { return tumors; }
    const std::vector<Queen>& getQueens() const { return queens; }
    int getTime() const { return time; }
    std::size_t getFloorsRemaining() const { return floorsRemaining; }
    bool canSpread() const;

private:
    void addQueen();
    void spreadCreep();
    bool spreadCreepFrom(Point p, std::size_t hash);
    const Tumor& addTumor(Point position);

    Table table;
    std::vector<Tumor> tumors;
    std::vector<Queen> queens;
    int nextId = 2;
    int time = 0;
    std::size_t floorsRemaining;
};

inline
auto getPredicate(const Status& status, bool (Status::*function)(Point) const) {
    return
            [function, &status](Point p) {
                return (status.*function)(p);
            };
}

#endif // CREEP_STATUS_HPP
