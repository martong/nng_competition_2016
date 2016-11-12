#include "PopulationRunner.hpp"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <boost/range/adaptor/transformed.hpp>
#include "Genome.hpp"
#include "AsyncHelper.hpp"
#include "Table.hpp"

PopulationRunner::PopulationRunner(const LearningParameters& parameters,
        const std::vector<GameInfo>& tracks,
        boost::asio::io_service& ioService):
            ioService(&ioService),
            population{parameters.populationSize,
                NeuralNetwork::getWeightCountForNetwork(
                    parameters.hiddenLayerCount,
                    parameters.neuronPerHiddenLayer,
                    parameters.commonParameters.inputNeuronCount(),
                    parameters.commonParameters.outputNeuronCount())} {
    controllerDatas.reserve(parameters.populationSize);
    for (std::size_t i = 0; i < parameters.populationSize; ++i) {
        controllerDatas.push_back(LearningControllerData{
            {
                parameters.hiddenLayerCount,
                parameters.neuronPerHiddenLayer,
                parameters.commonParameters.inputNeuronCount(),
                parameters.commonParameters.outputNeuronCount()
            },
            {}
        });

        auto& controllerData = controllerDatas.back();
        setNeuralNetworkExternalParameters(parameters.commonParameters, controllerData.network);
        controllerData.managers.reserve(tracks.size());
        for (const auto& track: tracks) {
            controllerData.managers.emplace_back(parameters.commonParameters,
                    track);
        }
    }
}

void PopulationRunner::runIteration() {
    Genomes& genomes = population.getPopulation();
    assert(genomes.size() == controllerDatas.size());

    std::condition_variable conditionVariable;
    std::mutex mutex;
    std::size_t tasksLeft{controllerDatas.size()};

    for (std::size_t i = 0; i < controllerDatas.size(); ++i) {
        auto& genome = genomes[i];
        auto& data = controllerDatas[i];

        ioService->post([this, &genome, &data, &tasksLeft, &conditionVariable, &mutex]() {
                runSimulation(genome, data);
                std::cerr << ".";
                {
                    std::unique_lock<std::mutex> lock{mutex};
                    if (--tasksLeft == 0) {
                        conditionVariable.notify_all();
                    }
                }
            });
    }

    {
        std::unique_lock<std::mutex> lock{mutex};
        while (tasksLeft != 0) {
            conditionVariable.wait(lock);
        }
        std::cerr << "\n";
    }

    updateBestFitness();
    population.evolve();
}

void PopulationRunner::runSimulation(Genome& genome, LearningControllerData& data) {
    data.network.setWeights(genome.weights);

    for (auto& manager: data.managers) {
        manager.setNeuralNetwork(data.network);
        manager.init();
        manager.run();
    }

    genome.fitness = 0.0f;
    genome.debugInfo.clear();
    for (const AIGameManager& manager : data.managers) {
        genome.fitness += manager.getFitness();
        genome.debugInfo += manager.getDebugInfo() + "  ";
    }
}

void PopulationRunner::updateBestFitness() {
    fitnessSum = 0.f;
    for (Genome& genome : population.getPopulation()) {
        fitnessSum += genome.fitness;
        if (genome.fitness > bestFitness) {
            bestFitness = genome.fitness;
            bestGenome = genome;
        }
    }
}
