#ifndef CREEP_AIGAMEMANAGER_HPP
#define CREEP_AIGAMEMANAGER_HPP

#include "GameManager.hpp"
#include "GameInfo.hpp"
#include "CommonParameters.hpp"

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
    CommonParameters commonParameters;
    GameInfo gameInfo;
    std::unique_ptr<Game> game;
    NeuralNetwork neuralNetwork;
};

#endif // CREEP_AIGAMEMANAGER_HPP
