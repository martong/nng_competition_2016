#include "CircleCache.hpp"

namespace {

bool isInsideCircle(Point p, int radius) {
    // magic
    int dx_q1=2*p.x+(0<p.x?1:-1);
    int dy_q1=2*p.y+(0<p.y?1:-1);
    int d2_q2=dx_q1*dx_q1+dy_q1*dy_q1;
    return d2_q2<=radius*radius*4;
}

std::vector<Point> calculateCircle(int radius) {
    std::vector<Point> result;
    Point p;
    for (p.y = -radius; p.y < radius; ++p.y) {
        for (p.x = -radius; p.x < radius; ++p.x) {
            if (isInsideCircle(p, radius)) {
                result.push_back(p);
            }
        }
    }
    return result;
}

} // unnamed namespace

const std::vector<Point>& CircleCache::get(int radius) {
    std::unique_lock<std::mutex> lock{mutex};
    auto it = cache.find(radius);
    if (it == cache.end()) {
        it = cache.emplace(radius, calculateCircle(radius)).first;
    }
    return it->second;
}
