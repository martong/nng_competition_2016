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
    static unsigned inputNeuronCount() {
        return checkDistances().size() * 4 + 7;
    }
    constexpr static unsigned outputNeuronCount() { return 3; }

    constexpr static std::size_t outputNeuronActivity = 0;
    constexpr static std::size_t outputNeuronFeedCurrent = 0;
    constexpr static std::size_t outputNeuronFeedNeighbor = 0;
};

class NeuralNetwork;
void setNeuralNetworkExternalParameters(const CommonParameters& parameters, NeuralNetwork& neuralNetwork);
void getNeuralNetworkExternalParameters(CommonParameters& parameters, const NeuralNetwork& neuralNetwork);

#endif // CREEP_COMMONPARAMETERS_HPP
