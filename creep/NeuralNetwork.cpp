
#include "NeuralNetwork.hpp"
#include <cassert>
#include <cmath>
#include <fstream>
#include <boost/archive/text_iarchive.hpp>

NeuralNetwork::NeuralNetwork(
        unsigned hiddenLayerCount,
        unsigned hiddenLayerNeuronCount,
        unsigned inputNeuronCount,
        unsigned outputNeuronCount) :
            hiddenLayerCount(hiddenLayerCount),
            hiddenLayerNeuronCount(hiddenLayerNeuronCount),
            inputNeuronCount(inputNeuronCount),
            outputNeuronCount(outputNeuronCount)
{
    std::shared_ptr<Weights> w = std::make_shared<Weights>();

    std::size_t numberOfWeights = getWeightCountForNetwork(hiddenLayerCount,
            hiddenLayerNeuronCount, inputNeuronCount, outputNeuronCount);

    w->reserve(numberOfWeights);
    for (std::size_t i = 0; i < numberOfWeights; ++i) {
        w->push_back(randomReal(-1, 1));
    }

    weights = w;
}

unsigned NeuralNetwork::getWeightCountForNetwork(
        unsigned hiddenLayerCount,
        unsigned hiddenLayerNeuronCount,
        unsigned inputNeuronCount,
        unsigned outputNeuronCount)
{
    return (hiddenLayerCount == 0) ?
                (outputNeuronCount * (inputNeuronCount + 1)) :
                (hiddenLayerNeuronCount * (inputNeuronCount + outputNeuronCount +
                        (hiddenLayerCount - 1) * hiddenLayerNeuronCount) +
                    (hiddenLayerCount * hiddenLayerNeuronCount + outputNeuronCount));
}

Weights NeuralNetwork::evaluateInput(Weights input) {
    assert(input.size() == inputNeuronCount);
    assert(weights);

    Weights output;
    const auto& w = *weights;

    unsigned weightIndex = 0;
    for (unsigned layer = 0; layer <= hiddenLayerCount; ++layer) {
        unsigned neuronCount = (layer == hiddenLayerCount) ?
                outputNeuronCount :
                hiddenLayerNeuronCount;

        output.clear();
        output.reserve(neuronCount);
        for (unsigned neuron = 0; neuron < neuronCount; ++neuron) {
            Weight netInput = 0;

            for (auto value: input) {
                netInput += w[weightIndex++]*value;
            }
            netInput += -1.f * w[weightIndex++];

            Weight sigmoid = sigmoidApproximation(netInput);
            output.push_back(sigmoid);

        }
        input = output; //we could move here probably
    }
    return output;
}

NeuralNetwork loadNeuralNetworkFromFile(const std::string& fileName) {
    NeuralNetwork result;

    std::ifstream ifs(fileName);
    boost::archive::text_iarchive ia(ifs);
    ia >> result;

    return result;
}

std::string NeuralNetwork::getExternalParameter(const std::string& key) const {
    auto it = externalParameters.find(key);
    return it == externalParameters.end() ? "" : it->second;
}

void NeuralNetwork::setExternalParameter(std::string key, std::string value) {
    externalParameters[std::move(key)] = std::move(value);
}
