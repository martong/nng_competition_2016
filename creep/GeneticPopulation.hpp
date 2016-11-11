#ifndef CREEP_GENETICPOPULATION_HPP
#define CREEP_GENETICPOPULATION_HPP

#include <vector>

#include "NeuralNetwork.hpp"
#include "Genome.hpp"

//This class implements Genetic algorithms to mutate its population
class GeneticPopulation {
public:

    GeneticPopulation() = default;
    GeneticPopulation(unsigned populationSize, unsigned numberOfWeights);

    GeneticPopulation(const GeneticPopulation&) = default;
    GeneticPopulation(GeneticPopulation&&) = default;
    GeneticPopulation& operator=(const GeneticPopulation&) = default;
    GeneticPopulation& operator=(GeneticPopulation&&) = default;

    const Genomes& getPopulation() const;
    Genomes& getPopulation();

    void evolve();

private:
    void mutate(Weights& weights) const;

    Genome pickRoulette() const;
    void pickBest(unsigned topN, unsigned copies, Genomes& newPopulation);

    void crossover(
        const Weights& parent1,
        const Weights& parent2,
        Weights& child1,
        Weights& child2) const;

    void calculateStats();
    void randomize();

    Genomes population;

    unsigned bestFitnessIndex; //updated by calculateStats()
    unsigned worstFitnessIndex; //updated by calculateStats()
    float totalFitness; //updated by calculateStats()

    //constants
    static constexpr float mutationRate = 0.1;
    static constexpr float crossoverRate = 0.7;
    static constexpr float maxPerturbation = 0.3;
    static constexpr unsigned bestTopN = 8;
    static constexpr unsigned bestCopies = 1;
};

#endif // CREEP_GENETICPOPULATION_HPP
