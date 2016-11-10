
#ifndef CREEP_LEARNINGCONTROLLER_HPP
#define CREEP_LEARNINGCONTROLLER_HPP

#include <functional>
#include "GameInfo.hpp"
#include "LearningParameters.hpp"

namespace boost { namespace asio {

class io_service;

}}


class GeneticPopulation;
class Genome;

class LearningController {
public:
    LearningController(const LearningParameters& parameters,
            std::vector<GameInfo> tracks,
            boost::asio::io_service& ioService);
    void run();

private:
    void loadPopulation(GeneticPopulation& population) const;
    void savePopulation(const GeneticPopulation& population) const;

    boost::asio::io_service& ioService;
    LearningParameters parameters;
    std::vector<GameInfo> tracks;

    void saveNeuralNetwork(const Genome& genome);
};

#endif // CREEP_LEARNINGCONTROLLER_HPP

