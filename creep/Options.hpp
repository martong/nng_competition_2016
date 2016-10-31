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
};

Options parseOptions(int argc, const char* argv[]);

#endif // CREEP_OPTIONS_HPP
