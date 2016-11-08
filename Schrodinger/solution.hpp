#include <vector>

std::vector<std::size_t> FindRadioactiveBalls(
    std::size_t NumberOfBalls, std::size_t RadioactiveBalls,
    bool (*TestFunction)(const std::vector<std::size_t>& BallsToTest));
