#ifndef CREEP_COMMONPARAMETERS_HPP
#define CREEP_COMMONPARAMETERS_HPP

#include <vector>
#include <string>
#include "CarParameters.hpp"
#include "PerturbationParameters.hpp"
#include "util/LazyArgumentEnum.hpp"

LAZY_ARGUMENT_ENUM(LookaheadType, lookaheadTypes, (checkpoint))

struct CommonParameters {
    constexpr static std::initializer_list<int> checkDistances() {
        return {1, 2, 5, 8, 9, 10, 11, 12, 15, 20};
    }
    constexpr static unsigned inputNeuronCount() {
        return checkDistance().size() * 4 + 5;
    }
    constexpr static unsigned outputNeuronCount() { return 1; }
};

class NeuralNetwork;
inpu
void setNeuralNetworkExternalParameters(const CommonParameters& parameters, NeuralNetwork& neuralNetwork);
void getNeuralNetworkExternalParameters(CommonParameters& parameters, const NeuralNetwork& neuralNetwork);

#endif // CREEP_COMMONPARAMETERS_HPP
