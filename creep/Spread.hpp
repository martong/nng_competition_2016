#ifndef CREEP_SPREAD_HPP
#define CREEP_SPREAD_HPP

template<typename Function>
void iterateSpreadArea(Point max, Point center, int radius, const Function& function) {
    for (Point p : PointRange{
            Point{std::max<int>(center.x - radius + 1, 0),
                  std::max<int>(center.y - radius + 1, 0)},
            Point{std::min<int>(center.x + radius, max.x - 1),
                  std::min<int>(center.y + radius, max.y - 1)}}) {
        if (isInsideCircle(p - center, radius)) {
            function(p);
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
