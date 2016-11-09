#include "CommonParameters.hpp"
#include "NeuralNetwork.hpp"

#include <boost/lexical_cast.hpp>

namespace car {

void setNeuralNetworkExternalParameters(const CommonParameters& parameters, NeuralNetwork& neuralNetwork) {
}

namespace {

template <typename T>
T getParameter(const std::string& s) {
	return s.empty() ? T{} : boost::lexical_cast<T>(s);
}

}

void getNeuralNetworkExternalParameters(CommonParameters& parameters, const NeuralNetwork& neuralNetwork) {
}

}


