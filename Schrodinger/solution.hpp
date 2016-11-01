#include <vector>

using V = std::vector<size_t>;

std::vector<size_t> FindRadioactiveBalls(
    size_t NumberOfBalls, size_t RadioactiveBalls,
    bool (*TestFunction)(const std::vector<size_t>& BallsToTest));

V range(size_t low, size_t high);
