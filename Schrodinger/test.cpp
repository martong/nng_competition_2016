#include <gtest/gtest.h>
#include "solution.hpp"

using V = std::vector<size_t>;

V radioactives;

/// Returns true if there is at least one radioactive ball in the set
bool testFun(const V& ballsToTest) {
    for (const auto& b: ballsToTest) {
        auto it = std::find(radioactives.begin(), radioactives.end(), b);
        if (it != std::end(ballsToTest)) {
            return true;
        }
    }
    return false;
}

TEST(Foo, AllRadioactive) {
    V balls = {0, 1, 2, 3};
    V radioactives = {0, 1, 2, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST(Foo, ZeroRadioactive) {
    V balls = {0, 1, 2, 3};
    V radioactives = {};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}
