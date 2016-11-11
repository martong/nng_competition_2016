#ifndef CREEP_LEARNINGPARAMETERS_HPP
#define CREEP_LEARNINGPARAMETERS_HPP

#include <thread>
#include <boost/optional.hpp>
#include "CommonParameters.hpp"

struct LearningParameters {
    CommonParameters commonParameters;

    //Neural network parameters
    unsigned hiddenLayerCount = 1;
    unsigned neuronPerHiddenLayer = 60;

    unsigned populationSize = 80;
    unsigned generationLimit = 1000000;
    unsigned printoutFrequency = 1;

    //place to save the file for the best AI trained
    std::string bestAIFile = "best.ai";

    //place to save/load the current population
    std::string populationOutputFile;
    std::string populationInputFile;

    unsigned startingPopulations = 1;
    unsigned populationCutoff = 10;
};

#endif // CREEP_LEARNINGPARAMETERS_HPP
