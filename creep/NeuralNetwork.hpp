#ifndef CREEP_NEURALNETWORK_HPP
#define CREEP_NEURALNETWORK_HPP

#include <vector>
#include <map>
#include <memory>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include "NeuronWeights.hpp"

class NeuralNetwork {
public:
    NeuralNetwork() = default;

    NeuralNetwork(
            unsigned hiddenLayerCount,
            unsigned hiddenLayerNeuronCount,
            unsigned inputNeuronCount,
            unsigned outputNeuronCount);

    static unsigned getWeightCountForNetwork(
            unsigned hiddenLayerCount,
            unsigned hiddenLayerNeuronCount,
            unsigned inputNeuronCount,
            unsigned outputNeuronCount);

    NeuralNetwork(const NeuralNetwork&) = default;
    NeuralNetwork& operator=(const NeuralNetwork&) = default;
    NeuralNetwork(NeuralNetwork&&) = default;
    NeuralNetwork& operator=(NeuralNetwork&&) = default;

    const Weights& getWeights() const { return weights; }
    void setWeights(Weights weights) {
        this->weights = std::move(weights);
    }
    unsigned getWeightCount() const {
        return weights.size();
    }

    unsigned getInputNeuronCount() const { return inputNeuronCount; }
    unsigned getOutputNeuronCount() const { return outputNeuronCount; }

    Weights evaluateInput(Weights input);

    std::string getExternalParameter(const std::string& key) const;
    void setExternalParameter(std::string key, std::string value);

private:
    unsigned hiddenLayerCount;
    unsigned hiddenLayerNeuronCount;
    unsigned inputNeuronCount;
    unsigned outputNeuronCount;

    Weights weights;

    /*
     * We want to achive the maximum flexibility here. Since this is only read
     * when the neural network is loaded, efficiency is not a concern here.  A
     * map is used because the neural network doesn't care about what the
     * actual parameters are, it only stores values to be used by the front
     * end. String is used as the key to increase readability of the front end
     * code.
     */
    std::map<std::string, std::string> externalParameters;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void load(Archive& ar, const unsigned version) {
        assert(version >= 2);
        ar >> hiddenLayerCount;
        ar >> hiddenLayerNeuronCount;
        ar >> inputNeuronCount;
        ar >> outputNeuronCount;
        ar >> weights;
        ar >> externalParameters;
    }

    template<class Archive>
    void save(Archive& ar, const unsigned /*version*/) const {
        ar << hiddenLayerCount;
        ar << hiddenLayerNeuronCount;
        ar << inputNeuronCount;
        ar << outputNeuronCount;
        ar << weights;
        ar << externalParameters;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();

};

template<class T>
T sigmoidApproximation(T x) {
    return x / (1 + std::abs(x));
}

NeuralNetwork loadNeuralNetworkFromFile(const std::string& fileName);

BOOST_CLASS_VERSION(NeuralNetwork, 2)

#endif // CREEP_NEURALNETWORK_HPP
