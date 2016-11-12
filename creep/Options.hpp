#ifndef CREEP_OPTIONS_HPP
#define CREEP_OPTIONS_HPP

#include "LearningParameters.hpp"

#include <string>

struct Options {
    std::string type;
    std::string inputFileName;
    std::string aiFileName;
    std::size_t numThreads = 1;
    LearningParameters learningParameters;
    CommonParameters commonParameters;
};

Options parseOptions(int argc, const char* argv[]);

#endif // CREEP_OPTIONS_HPP
