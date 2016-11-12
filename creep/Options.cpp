#include "Options.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>

#include <cmath>
#include <iostream>
#include <vector>

namespace {

template<typename T>
boost::program_options::typed_value<T>* defaultValue(T& value) {
    return boost::program_options::value(&value)->default_value(value);
}

}

Finder parseFinder(const std::string& s) {
    std::vector<std::string> tokens;
    boost::algorithm::split(tokens, s, boost::algorithm::is_any_of(","));
    Finder result;
    result.begin = boost::lexical_cast<float>(tokens[0]);
    result.end = boost::lexical_cast<float>(tokens[1]);
    result.delta = std::abs(boost::lexical_cast<float>(tokens[2]));
    if (result.end < result.begin) {
        result.delta *= -1.0f;
    }
    return result;
}

Options parseOptions(int argc, const char* argv[]) {
    namespace po = boost::program_options;
    Options result;
    std::string timeMultiplierFinderString = "0.0,-1.0,-1.0";
    std::string distanceSquareMultiplierFinderString = "0.0,-1.0,-1.0";
    std::string spreadRadiusMultiplierFinderString = "0.0,1.0,1.0";
    po::options_description commonOptions{"Common options"};
    commonOptions.add_options()
            ("help,h", "Help")
            ("jobs,j", defaultValue(result.numThreads), "Number of threads")
            ("type,t", po::value(&result.type), "simulate or solve")
            ("map,m", po::value(&result.inputFileName), "The input file name")
            ;
    po::options_description solveOptions{"Options for heuristic solving"};
    solveOptions.add_options()
            ("distance-square-multiplier",
                     defaultValue(distanceSquareMultiplierFinderString),
                     "Values of distance square multiplier: min,max,delta")
            ("time-multiplier",
                     defaultValue(timeMultiplierFinderString),
                     "Values of time multiplier: min,max,delta")
            ("spread-radius-multiplier",
                     defaultValue(spreadRadiusMultiplierFinderString),
                     "Values of spread radius multiplier: min,max,delta")
            ;
    po::options_description neuralOptions{"Options for neural solving"};
    neuralOptions.add_options()
            ("hidden-layer-count",
             defaultValue(result.learningParameters.hiddenLayerCount),
             "Hidden layer count")
            ("neuron-per-hidden-layer",
             defaultValue(result.learningParameters.neuronPerHiddenLayer),
             "Neurons per hidden layer")
            ("population-size",
             defaultValue(result.learningParameters.populationSize),
             "The size of the population")
            ("generation-limit",
             defaultValue(result.learningParameters.generationLimit),
             "Stop after this many generations")
            ("printout-frequency",
             defaultValue(result.learningParameters.printoutFrequency),
             "Print diagnostic after each n generations")
            ("best-ai-file",
             defaultValue(result.learningParameters.bestAIFile),
             "The file to save the best neural network")
            ("population-input-file,i",
             defaultValue(result.learningParameters.populationInputFile),
             "Load initial population from here")
            ("population-output-file,o",
             defaultValue(result.learningParameters.populationOutputFile),
             "The file to save the best neural network")
            ("starting-populations",
             defaultValue(result.learningParameters.startingPopulations),
             "The number of independent populations to start the learning with.")
            ("population-cutoff",
             defaultValue(result.learningParameters.populationCutoff),
             "The number of generations after the worst population is dropped (if there are more than one).")
            ;
    po::options_description generateOptions{
            "Options for generating solution from neural network"};
    generateOptions.add_options()
            ("ai-file-name,f",
             defaultValue(result.aiFileName),
             "The file where the neural network is stored.")
            ;
    po::options_description options;
    options.add(commonOptions);
    options.add(solveOptions);
    options.add(neuralOptions);
    options.add(generateOptions);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(options).run(), vm);
    po::notify(vm);
    if (vm.count("help") != 0) {
        std::cerr << options;
        std::exit(0);
    }
    result.timeMultiplierFinder = parseFinder(timeMultiplierFinderString);
    result.distanceSquareMultiplierFinder =
            parseFinder(distanceSquareMultiplierFinderString);
    result.spreadRadiusMultiplierFinder =
            parseFinder(spreadRadiusMultiplierFinderString);
    return result;
}
