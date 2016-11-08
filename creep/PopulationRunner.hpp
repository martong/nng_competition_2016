#ifndef CREEP_POPULATIONRUNNER_HPP
#define CREEP_POPULATIONRUNNER_HPP

#include <functional>
#include <vector>
#include <string>
#include <boost/asio/io_service.hpp>
#include "LearningParameters.hpp"
#include "GeneticPopulation.hpp"
#include "AIGameManager.hpp"
#include "NeuralNetwork.hpp"
#include "Table.hpp"

class Genome;
class FitnessCalculator;

class PopulationRunner {
public:
    PopulationRunner(const LearningParameters& parameters,
        const std::vector<Table>& tracks,
        FitnessCalculator& fitnessCalculator,
        boost::asio::io_service& ioService);

    PopulationRunner(const PopulationRunner&) = delete;
    PopulationRunner& operator=(const PopulationRunner&) = delete;
    PopulationRunner(PopulationRunner&&) = default;
    PopulationRunner& operator=(PopulationRunner&&) = default;

    void runIteration();

    float getBestFitness() const { return bestFitness; }
    float getAverageFitness() const { return fitnessSum / population.getPopulation().size(); }
    const Genome& getBestGenome() const { return bestGenome; }
    const GeneticPopulation& getPopulation() const { return population; }
    GeneticPopulation& getPopulation() { return population; }
private:
    struct LearningControllerData {
        NeuralNetwork network;
        std::vector<AIGameManager> managers;
    };

    FitnessCalculator* fitnessCalculator;
    boost::asio::io_service* ioService;

    GeneticPopulation population;
    std::vector<LearningControllerData> controllerDatas;
    float fitnessSum = 0.f; // Updated by updateBestFitness
    float bestFitness = 0.f; // Updated by updateBestFitness
    Genome bestGenome;

    void runSimulation(Genome& genome, LearningControllerData& data);
    void updateBestFitness();
};

#endif // CREEP_POPULATIONRUNNER_HPP
