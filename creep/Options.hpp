#ifndef CREEP_OPTIONS_HPP
#define CREEP_OPTIONS_HPP

#include <string>

struct Finder {
    float begin, end, delta;
};

Finder parseFinder(const std::string& s);

template<typename Function>
void iterateFinder(const Finder& finder, const Function& function) {
    float difference = finder.end - finder.begin;
    if (difference == 0.0f) {
        difference = 1.0f;
    }
    for (float value = finder.begin;
            value * difference <= finder.end * difference;
            value += finder.delta) {
        function(value);
    }
}

struct Options {
    std::string type;
    std::string inputFileName;
    Finder timeMultiplierFinder;
    Finder distanceSquareMultiplierFinder;
    Finder spreadRadiusMultiplierFinder;
    std::size_t numThreads = 1;
};

Options parseOptions(int argc, const char* argv[]);

#endif // CREEP_OPTIONS_HPP
