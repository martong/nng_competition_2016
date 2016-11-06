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

Options parseOptions(int argc, const char* argv[]) {
    namespace po = boost::program_options;
    Options result;
    std::string timeMultiplierFinderString = "0.0,-1.0,-1.0";
    std::string distanceSquareMultiplierFinderString = "0.0,-1.0,-1.0";
    std::string spreadRadiusMultiplierFinderString = "0.0,1.0,1.0";
    po::options_description options;
    options.add_options()
            ("help,h", "Help")
            ("jobs,j", defaultValue(result.numThreads), "Number of threads")
            ("type,t", po::value(&result.type), "simulate or solve")
            ("map,m", po::value(&result.inputFileName), "The input file name")
            ("seed,s", defaultValue(result.seed), "The random seed (0=random)")
            ("max-iterations,i", defaultValue(result.maxIterations),
             "The maximum number of iterations (0=unlimited)")
            ("output,o", po::value(&result.outputFileName),
             "The output file name")
            ("runs-per-iteration",
             defaultValue(result.solverParameters.runsPerIteration),
             "The number of runs per iteration")
            ("time-score",
             defaultValue(result.solverParameters.timeScore),
             "The score awarded for each tick before the maximum when the "
             "simulation is finished")
            ("creep-penalty",
             defaultValue(result.solverParameters.floorPenalty),
             "The penalty for each floor left")
            ("heat-flow-strength",
             defaultValue(result.solverParameters.heatFlowStrength),
             "How much heat flows from the position of tumors")
            ("heat-flow-maximum-distance",
             defaultValue(result.solverParameters.heatFlowMaxDistance),
             "The maximum distance for heat to flow from each tumor")
            ("cooldown-factor",
             defaultValue(result.solverParameters.cooldownFactor),
             "The heat is cooled down by this factor at the end of each iteration")
            ("spread-radius-multiplier",
             defaultValue(result.solverHeuristics.spreadRadiusMultiplier),
             "How much to prefer the increase in maximum creep")
            ("time-multiplier",
             defaultValue(result.solverHeuristics.timeMultiplier),
             "How much to prefer putting down a tumor early.")
            ("minimum-distance-multiplier",
             defaultValue(result.solverHeuristics.minimumDistanceMultiplier),
             "How much to prefer putting tumors far from each other")
            ;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(options).run(), vm);
    po::notify(vm);
    if (vm.count("help") != 0) {
        std::cerr << options;
        std::exit(0);
    }
    return result;
}
