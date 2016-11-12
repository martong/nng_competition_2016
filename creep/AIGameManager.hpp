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
    std::vector<Command> getCommands() const;
private:
    struct NeuronActivity {
        NeuronActivity() : feedNeighbor(
                CommonParameters::neighborOutputDistances().size(), 0.0f) {}

        NeuronActivity(float activity, float feedCurrent,
                std::vector<float> feedNeighbor) :
                activity(activity), feedCurrent(feedCurrent),
                feedNeighbor(std::move(feedNeighbor)) {}

        float activity = -1.0f;
        float feedCurrent = 0.0f;
        std::vector<float> feedNeighbor;
    };

    using TumorSpreadPositions = Matrix<std::vector<const Tumor*>>;

    NeuronActivity callNeuralNetwork(Point base);
    void tick();
    Matrix<NeuronActivity> evaluateTable(
            Matrix<std::vector<const Tumor*>> tumorSpreadPositions);
    void calculatePotentialCreep();
    TumorSpreadPositions getTumorSpreadPositions();
    bool addCommandIfPossible();
    void calculateExpriation();
    void addCommand(const Command& command);

    CommonParameters commonParameters;
    GameInfo gameInfo;
    std::unique_ptr<Game> game;
    int initialFloorCount = 0;
    NeuralNetwork neuralNetwork;
    Matrix<NeuronActivity> neuronActivity;
    Matrix<bool> potentialCreep;
    Matrix<int> areaExpiration;
    boost::container::flat_set<Point> pendingPoints;
    boost::container::flat_set<const Tumor*> pendingTumors;
    boost::container::flat_set<const Queen*> pendingQueens;
};

#endif // CREEP_AIGAMEMANAGER_HPP
