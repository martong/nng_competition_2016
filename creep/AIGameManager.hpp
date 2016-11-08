#ifndef CREEP_AIGAMEMANAGER_HPP
#define CREEP_AIGAMEMANAGER_HPP

#include "GameManager.hpp"
#include "Track/TrackCreator.hpp"
#include "IterationParameters.hpp"

class AIGameManager {
public:
    AIGameManager(const CommonParameters& commonParameters,
            const IterationParameters& iterationParameters,
            Table  track);

    void run();

    void setNeuralNetwork(const NeuralNetwork& neuralNetwork);

    void init();
    const GameManager& getGameManager() const { return gameManager; }

private:
};

#endif // CREEP_AIGAMEMANAGER_HPP
