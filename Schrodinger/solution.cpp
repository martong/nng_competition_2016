#include <vector>
#include <numeric>

using V = std::vector<size_t>;

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

    return {};
}

