#ifndef POINT_H_
#define POINT_H_

#include "Direction.hpp"
#include "Hash.hpp"

#include <ostream>
#include <stddef.h>

class Point {
public:
    int x, y;
    Point():x(0), y(0) { }
    constexpr Point(int x, int y):x(x), y(y) {}

    constexpr Point(const Point&) = default;
    Point& operator=(const Point&) = default;

    Point& operator+=(Point other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Point& operator-=(Point other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Point& operator*=(int n)
    {
        x *= n;
        y *= n;
        return *this;
    }
};

template<typename Archive>
void serialize(Archive& ar, Point& p, const unsigned int /*version*/) {
    ar & p.x;
    ar & p.y;
}

constexpr Point p00 {0,0}, p10{1, 0}, p01{0, 1}, p11{1, 1};

inline constexpr bool operator==(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

inline constexpr bool operator!=(Point p1, Point p2) {
    return !(p1 == p2);
}

inline constexpr Point operator+(Point p1, Point p2) {
    return Point(p1.x+p2.x, p1.y+p2.y);
}

inline constexpr Point operator-(Point p1, Point p2) {
    return Point(p1.x-p2.x, p1.y-p2.y);
}

inline constexpr Point operator*(Point p, int n) {
    return Point(p.x*n, p.y*n);
}

inline constexpr Point operator-(Point p) {
    return p * -1;
}

inline constexpr bool operator<(Point p1, Point p2) {
    return p1.y < p2.y || (p1.y == p2.y && p1.x < p2.x);
}

inline int distance(Point p1, Point p2) {
    auto d = p1 - p2;
    return abs(d.x) + abs(d.y);
}

inline
int distanceSquare(Point p1, Point p2) {
    Point difference = p1 - p2;
    return difference.x * difference.x + difference.y * difference.y;
}

std::ostream& operator<<(std::ostream& os, Point p);
Direction toDirection(const Point& source, const Point& destination);

namespace std {

template<>
struct hash<Point> {
    size_t operator()(Point p) const
    {
        size_t seed = 0;
        hash_combine(seed, p.x);
        hash_combine(seed, p.y);
        return seed;
    }
};

} // namespace std

#endif /*POINT_H_*/
