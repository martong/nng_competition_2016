#ifndef CREEP_AIGAMEMANAGER_HPP
#define CREEP_AIGAMEMANAGER_HPP

#include "GameManager.hpp"
#include "GameInfo.hpp"
#include "CommonParameters.hpp"
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
    const GameManager& getGameManager() const { return gameManager; }
private:
    Weights callNeuralNetwork();
    void tick();
    Matrix<float> evaluateTable();

    CommonParameters commonParameters;
    GameInfo gameInfo;
    std::unique_ptr<Game> game;
    int initialFloorCount = 0;
    NeuralNetwork neuralNetwork;
    Matrix<bool> potentialCreep;
};

#endif // CREEP_AIGAMEMANAGER_HPP
