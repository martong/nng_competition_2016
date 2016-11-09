#ifndef SRC_COMMONPARAMETERS_HPP
#define SRC_COMMONPARAMETERS_HPP

#include <vector>
#include <string>
#include "CarParameters.hpp"
#include "PerturbationParameters.hpp"
#include "util/LazyArgumentEnum.hpp"

namespace car {

LAZY_ARGUMENT_ENUM(LookaheadType, lookaheadTypes, (checkpoint))

struct CommonParameters {
	// for speed
	constexpr static unsigned extraGlobalInputNeuronCount() { return  1; }
	// 2 for orientation, 2*(2+1) for left/right edge orientation/distance
	constexpr static unsigned inputNeuronCountPerCheckpoint() { return  8; }
	constexpr static unsigned outputNeuronCount() { return 3; }

	unsigned getInputNeuronCount() const {
		return rayCount + extraGlobalInputNeuronCount() + checkpointLookAhead * inputNeuronCountPerCheckpoint();
	}

};

class NeuralNetwork;

void setNeuralNetworkExternalParameters(const CommonParameters& parameters, NeuralNetwork& neuralNetwork);
void getNeuralNetworkExternalParameters(CommonParameters& parameters, const NeuralNetwork& neuralNetwork);

}


#endif /* SRC_COMMONPARAMETERS_HPP */
