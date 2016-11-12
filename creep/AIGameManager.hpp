#ifndef CREEP_AIGAMEMANAGER_HPP
#define CREEP_AIGAMEMANAGER_HPP

#include "CommonParameters.hpp"
#include "GameInfo.hpp"
#include "Game.hpp"
#include "NeuralNetwork.hpp"

#include <boost/container/flat_set.hpp>
#include <boost/optional.hpp>

class AIGameManager {
public:
    AIGameManager(const CommonParameters& commonParameters,
            const GameInfo& track);

    void run();

    void setNeuralNetwork(const NeuralNetwork& neuralNetwork) {
        this->neuralNetwork = neuralNetwork;
    }

    void init();
    float getFitness() const;
    std::string getDebugInfo() const;
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
    using TumorSpreadPositions = Matrix<std::vector<const Tumor*>>;

    NeuronActivity callNeuralNetwork(Point base);
    void tick();
    Matrix<NeuronActivity> evaluateTable(
            Matrix<std::vector<const Tumor*>> tumorSpreadPositions);
    void calculatePotentialCreep();
    TumorSpreadPositions getTumorSpreadPositions();
    boost::optional<Point> addCommandIfPossible();

    CommonParameters commonParameters;
    GameInfo gameInfo;
    std::unique_ptr<Game> game;
    int initialFloorCount = 0;
    NeuralNetwork neuralNetwork;
    Matrix<NeuronActivity> neuronActivity;
    Matrix<bool> potentialCreep;
    boost::container::flat_set<Point> pendingPoints;
    boost::container::flat_set<const Tumor*> pendingTumors;
    boost::container::flat_set<const Queen*> pendingQueens;

    static constexpr float fitnessFloorsCoveredMultiplier = 20.0;
    static constexpr float fitnessTimeRemainingMultiplier = 1.0;
};

#endif // CREEP_AIGAMEMANAGER_HPP
