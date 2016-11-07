#ifndef CREEP_CIRCLECACHE_HPP
#define CREEP_CIRCLECACHE_HPP

#include <Point.hpp>

#include <boost/container/flat_map.hpp>

#include <mutex>
#include <vector>

class CircleCache {
public:
    const std::vector<Point>& get(int radius);
private:
    std::mutex mutex;
    boost::container::flat_map<int, std::vector<Point>> cache;
};

#endif // CREEP_CIRCLECACHE_HPP
