#ifndef CREEP_SPREAD_HPP
#define CREEP_SPREAD_HPP

#include <Point.hpp>

#include <vector>

inline bool isInsideCircle(Point p, int radius) {
    // magic
    int dx_q1=2*p.x+(0<p.x?1:-1);
    int dy_q1=2*p.y+(0<p.y?1:-1);
    int d2_q2=dx_q1*dx_q1+dy_q1*dy_q1;
    return d2_q2<=radius*radius*4;
}

template<typename Function>
void iterateSpreadArea(Point bound, Point center, int radius,
        const Function& function) {
    Point min{std::max<int>(center.x - radius + 1, 0),
              std::max<int>(center.y - radius + 1, 0)};
    Point max{std::min<int>(center.x + radius, bound.x - 1),
              std::min<int>(center.y + radius, bound.y - 1)};
    Point p;
    for (p.y = min.y; p.y < max.y; ++p.y) {
        for (p.x = min.x; p.x < max.x; ++p.x) {
            if (isInsideCircle(p - center, radius)) {
                function(p);
            }
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
