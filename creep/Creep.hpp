#ifndef CREEP_CREEP_HPP
#define CREEP_CREEP_HPP

#include "Table.hpp"

#include <vector>

inline bool isInsideCircle(Point p, int radius) {
    // magic
    int dx_q1=2*p.x+(0<p.x?1:-1);
    int dy_q1=2*p.y+(0<p.y?1:-1);
    int d2_q2=dx_q1*dx_q1+dy_q1*dy_q1;
    return d2_q2<=radius*radius*4;
}

template<typename Predicate>
std::vector<Point> getSpreadArea( const Table& table, Point center, int radius,
        const Predicate& predicate) {
    std::vector<Point> result;
    for (Point p : PointRange{
            Point{std::max<int>(center.x - radius + 1, 0),
                  std::max<int>(center.y - radius + 1, 0)},
            Point{std::min<int>(center.x + radius, table.width() - 1),
                  std::min<int>(center.y + radius, table.height() - 1)}}) {
        if (isInsideCircle(p - center, radius) && predicate(table, p)) {
            result.push_back(p);
        }
    }
    return result;
}

inline bool isFloor(const Table& table, Point p) {
    return table[p] == MapElement::Floor;
}

inline bool isCreep(const Table& table, Point p) {
    return table[p] == MapElement::Creep;
}

inline bool hasCreep(MapElement element) {
    return element != MapElement::Wall && element != MapElement::Floor;
}

inline bool isCreepCandidate(const Table& table, Point p) {
    return table[p] == MapElement::Floor && (
           hasCreep(table[p - p10]) || hasCreep(table[p + p10]) ||
           hasCreep(table[p - p01]) || hasCreep(table[p + p01]));
}

#endif // CREEP_CREEP_HPP
