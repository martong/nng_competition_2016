#ifndef CREEP_OPTIONS_HPP
#define CREEP_OPTIONS_HPP

#include "Solver.hpp"

#include <string>

struct Options {
    std::string type;
    std::string inputFileName;
    std::string outputFileName;
    Solver::Parameters solverParameters;
    Solver::Heuristics solverHeuristics;
    std::size_t numThreads = 1;
    std::size_t maxIterations = 0;
    int seed = 0;
};

Options parseOptions(int argc, const char* argv[]);

#endif // CREEP_OPTIONS_HPP
