#include <vector>
#include <numeric>
#include <cassert>

using V = std::vector<size_t>;
using TestFun = bool (*)(const std::vector<size_t>& BallsToTest);

/// [low, high]
V range(size_t low, size_t high) {
    assert(low <= high);
    V res(high - low + 1);
    std::iota(res.begin(), res.end(), low);
    return res;
}

V linear(size_t NumberOfBalls, size_t RadioactiveBalls, TestFun testFun) {
    V res;
    for (size_t i = 0; i < NumberOfBalls; ++i) {
        if (testFun(range(i, i))) {
            res.push_back(i);
        }
    }
    return res;
}

std::vector<size_t> FindRadioactiveBalls(
    size_t NumberOfBalls, size_t RadioactiveBalls,
    bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {
    if (NumberOfBalls == RadioactiveBalls) {
        V res(NumberOfBalls);
        std::iota(res.begin(), res.end(), 0);
        return res;
    }
    if (RadioactiveBalls == 0) {
        return {};
    }

    return linear(NumberOfBalls, RadioactiveBalls, TestFunction);
}

V impl(size_t low, size_t high, size_t& num_ractive, TestFun testFun) {
    if (num_ractive == 0) {
        return {};
    }
    if (low == high) {
        --num_ractive;
        return {low};
    }

    V res;
    if (testFun(range(low, high/2))) { // ractive
        V subres = impl(low, high/2, num_ractive, testFun);
        res.insert(res.end(), subres.begin(), subres.end());
    }
    if (num_ractive && testFun(range(high/2 + 1, high))) { // ractive
        V subres = impl(high/2 + 1, high, num_ractive, testFun);
        res.insert(res.end(), subres.begin(), subres.end());
    }
    return res;
}

