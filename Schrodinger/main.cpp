#include <vector>
#include <iostream>
#include "worst_case.hpp"

using V = std::vector<size_t>;

bool testFun(const V& ballsToTest) {
    return true;
}

int main() {
    V balls = {0, 1, 2, 3};
    V radioactives = {0, 1, 2, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::cout << "OK\n";
}
