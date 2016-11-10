#ifndef CREEP_AIGAMEMANAGER_HPP
#define CREEP_AIGAMEMANAGER_HPP

#include "CommonParameters.hpp"
#include "GameInfo.hpp"
#include "Game.hpp"
#include "NeuralNetwork.hpp"

class AIGameManager {
public:
    AIGameManager(const CommonParameters& commonParameters,
            const GameInfo& track);

    void run();

    void setNeuralNetwork(const NeuralNetwork& neuralNetwork) {
        this->neuralNetwork = neuralNetwork;
    }

    void init();
private:
    struct NeuronActivity {
        NeuronActivity() = default;
        NeuronActivity(float activity, float feedCurrent, float feedNeighbor) :
                activity(activity), feedCurrent(feedCurrent),
                feedNeighbor(feedNeighbor) {}

        float activity = -1.0f;
        float feedCurrent = 0.0f;
        float feedNeighbor = 0.0f;
    };

    NeuronActivity callNeuralNetwork(Point base);
    void tick();
    Matrix<NeuronActivity> evaluateTable(
            Matrix<std::vector<const Tumor*>> tumorSpreadPositions);

    CommonParameters commonParameters;
    GameInfo gameInfo;
    std::unique_ptr<Game> game;
    int initialFloorCount = 0;
    NeuralNetwork neuralNetwork;
    Matrix<NeuronActivity> neuronActivity;
};

#endif // CREEP_AIGAMEMANAGER_HPP
