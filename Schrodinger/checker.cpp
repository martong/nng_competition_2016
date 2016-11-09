#include "solution.hpp"

#include <boost/lexical_cast.hpp>

#include <random>
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>


std::set<std::size_t> solution;
std::size_t called = 0;

bool testFunction(const std::vector<std::size_t>& balls) {
    ++called;
    return std::any_of(balls.begin(), balls.end(),
            [](std::size_t ball) { return solution.count(ball) != 0; });
}

int main(int argc, const char* argv[]) {
    assert(argc == 4);
    std::size_t numberOfBalls = boost::lexical_cast<std::size_t>(argv[1]);
    std::size_t radioactiveBalls = boost::lexical_cast<std::size_t>(argv[2]);
    std::mt19937 rng{boost::lexical_cast<std::size_t>(argv[3])};
    std::uniform_int_distribution<std::size_t> dist{0, numberOfBalls - 1};
    while (solution.size() != radioactiveBalls) {
        solution.insert(dist(rng));
    }
    std::vector<std::size_t> solutionVector;
    std::copy(solution.begin(), solution.end(),
            std::back_inserter(solutionVector));
    for (std::size_t i : solutionVector) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    auto answer = FindRadioactiveBalls(numberOfBalls, radioactiveBalls,
            &testFunction);
    std::sort(answer.begin(), answer.end());
    for (std::size_t i : answer) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    if (solutionVector == answer) {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "bad" << std::endl;
    }
    std::cout << "Called: " << called << std::endl;
}
