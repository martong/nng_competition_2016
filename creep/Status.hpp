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

    const std::vector<Tumor>& getTumors() const { return tumors; }
    const std::vector<Queen>& getQueens() const { return queens; }
    const Table& getTable() const { return table; }
    int getTime() const { return time; }
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

#endif // CREEP_STATUS_HPP
