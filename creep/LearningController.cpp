#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/tss.hpp>

#include "LearningController.hpp"
#include "PopulationRunner.hpp"

LearningController::LearningController(const LearningParameters& parameters,
        std::vector<GameInfo> tracks,
        boost::asio::io_service& ioService) :
    ioService(ioService),
    parameters(parameters),
    tracks(std::move(tracks))
{}

namespace {

bool compareBestFitnesses(const PopulationRunner& lhs, const PopulationRunner& rhs) {
    return lhs.getBestFitness() < rhs.getBestFitness();
}

void printInfo(unsigned generation, float bestFitness,
        const std::vector<float>& populationAverages,
        const std::string& debugInfo) {
    std::stringstream ss;
    ss << "Generation: " << generation << ", ";
    ss << "Current best fitness: " << bestFitness << ", ";
    ss << "Population averages: ";
    ss << boost::algorithm::join(populationAverages | boost::adaptors::transformed(
                boost::lexical_cast<std::string, float>), ", ") << ", ";
    ss << debugInfo;
    //if (isatty(2)) { //if stdout is a terminal
        //std::cerr << "\033[2K\r";
        //std::cerr << ss.str() << std::flush;
    //} else {
        std::cerr << ss.str() << std::endl;
    //}
}

}

void LearningController::run() {

    std::vector<PopulationRunner> populations;
    populations.reserve(parameters.startingPopulations);

    for (std::size_t i = 0; i < parameters.startingPopulations; ++i) {
        populations.emplace_back(parameters, tracks, ioService);
        loadPopulation(populations.back().getPopulation());
    }

    float bestFitness = 0.f;

    for (unsigned generation = 1; generation <= parameters.generationLimit;
            ++generation) {

        std::vector<float> populationAverages;
        for (auto& populationData: populations) {
            populationData.runIteration();
            populationAverages.push_back(populationData.getAverageFitness());
        }

        auto& bestPopulation = *boost::max_element(populations, compareBestFitnesses);
        savePopulation(bestPopulation.getPopulation());
        if (bestPopulation.getBestFitness() > bestFitness) {
            bestFitness = bestPopulation.getBestFitness();
            saveNeuralNetwork(bestPopulation.getBestGenome());
        }

        if (populations.size() > 1 && generation % parameters.populationCutoff == 0) {
            auto worstPopulation = boost::min_element(populations, compareBestFitnesses);
            populations.erase(worstPopulation);
        }
        if (generation % parameters.printoutFrequency == 0) {
            printInfo(generation, bestFitness, populationAverages,
                    bestPopulation.getBestGenome().debugInfo);
        }
    }
}

void LearningController::saveNeuralNetwork(const Genome& genome) {
    //TODO we are reconstucting the same network as above
    NeuralNetwork network(parameters.hiddenLayerCount,
            parameters.neuronPerHiddenLayer,
            parameters.commonParameters.inputNeuronCount(),
            parameters.commonParameters.outputNeuronCount());
    setNeuralNetworkExternalParameters(parameters.commonParameters, network);

    network.setWeights(genome.weights);

    std::ofstream ofs(parameters.bestAIFile);
    boost::archive::text_oarchive oa(ofs);
    oa << network;
}

void LearningController::loadPopulation(GeneticPopulation& population) const {
    if (!parameters.populationInputFile.empty() &&
            boost::filesystem::exists(parameters.populationInputFile)) {
        std::ifstream ifs(parameters.populationInputFile);
        boost::archive::text_iarchive ia(ifs);
        ia >> population.getPopulation();
    }
}

void LearningController::savePopulation(const GeneticPopulation& population) const {
    if (!parameters.populationOutputFile.empty()) {
        std::ofstream ofs(parameters.populationOutputFile);
        boost::archive::text_oarchive oa(ofs);
        oa << population.getPopulation();
    }
}
