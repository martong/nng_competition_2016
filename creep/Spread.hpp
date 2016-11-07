#ifndef CREEP_SPREAD_HPP
#define CREEP_SPREAD_HPP

#include "CircleCache.hpp"

#include <Point.hpp>

#include <boost/container/flat_map.hpp>

#include <vector>

template<typename Function>
void iterateSpreadArea(Point bound, Point center, int radius,
        const Function& function) {
    static CircleCache circleCache;
    for (Point p : circleCache.get(radius)) {
        Point pp = p + center;
        if (pp.x >= 0 && pp.x < bound.x && pp.y >= 0 && pp.y < bound.y) {
            function(pp);
        }
    }
}

template<typename Predicate>
std::size_t countSpreadArea(Point max, Point center, int radius,
        const Predicate& predicate) {
    std::size_t result = 0;
    iterateSpreadArea(max, center, radius,
            [&result, &predicate](Point p) {
                if (predicate(p)) {
                    ++result;
                }
            });
    return result;
}

template<typename Predicate>
std::vector<Point> findSpreadArea(Point max, Point center, int radius,
        const Predicate& predicate) {
    std::vector<Point> result;
    iterateSpreadArea(max, center, radius,
            [&result, &predicate](Point p) {
                if (predicate(p)) {
                    result.push_back(p);
                }
            });
    return result;
}

constexpr bool alwaysTrue(Point) {
    return true;
}

template<typename T>
Point getMax(const T& t) {
    return Point{static_cast<int>(t.width()), static_cast<int>(t.height())};
}

#endif // CREEP_SPREAD_HPP
