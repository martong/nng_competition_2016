#ifndef CREEP_COMMONPARAMETERS_HPP
#define CREEP_COMMONPARAMETERS_HPP

#include <vector>
#include <string>
#include "util/LazyArgumentEnum.hpp"

LAZY_ARGUMENT_ENUM(LookaheadType, lookaheadTypes, (checkpoint))

struct CommonParameters {
    static std::vector<int> checkDistances() {
        return {1, 2, 5, 8, 10, 12, 15, 20};
    }
    static std::vector<int> neighborOutputDistances() {
        return {1, 2, 5, 10};
    }
    static unsigned inputNeuronCount() {
        return checkDistances().size() * 4 +
                neighborOutputDistances().size() * 3 + 6;
    }
    static unsigned outputNeuronCount() {
        return neighborOutputDistances().size() + 2;
    }

    constexpr static std::size_t outputNeuronActivity = 0;
    constexpr static std::size_t outputNeuronFeedCurrent = 1;
    constexpr static std::size_t outputNeuronFeedNeighborStart = 2;

    float fitnessFloorsCoveredMultiplier = 1.0;
    float fitnessTimeRemainingMultiplier = 1.0;
};

class NeuralNetwork;
void setNeuralNetworkExternalParameters(const CommonParameters& parameters, NeuralNetwork& neuralNetwork);
void getNeuralNetworkExternalParameters(CommonParameters& parameters, const NeuralNetwork& neuralNetwork);

#endif // CREEP_COMMONPARAMETERS_HPP
