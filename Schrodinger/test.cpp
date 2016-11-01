#include <gtest/gtest.h>
#include "solution.hpp"

using V = std::vector<size_t>;

V radioactives;

/// Returns true if there is at least one radioactive ball in the set
bool testFun(const V& ballsToTest) {
    for (const auto& b: ballsToTest) {
        auto it = std::find(radioactives.begin(), radioactives.end(), b);
        if (it != std::end(radioactives)) {
            return true;
        }
    }
    return false;
}

TEST(Result, AllRadioactive) {
    V balls = {0, 1, 2, 3};
    radioactives = {0, 1, 2, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST(Result, ZeroRadioactive) {
    V balls = {0, 1, 2, 3};
    radioactives = {};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST(Range, range) {
    V v = range(0,3);
    V exp = {0,1,2,3};
    EXPECT_EQ(v, exp);
    v = range(1,1);
    exp = {1};
    EXPECT_EQ(v, exp);
}

TEST(Result, SizeFour) {
    V balls = {0, 1, 2, 3};
    radioactives = {0, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);

    balls = {0, 1, 2, 3};
    radioactives = {1, 2};
    res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);

    balls = {0, 1, 2, 3};
    radioactives = {1};
    res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);

    balls = {0, 1, 2, 3};
    radioactives = {0, 1, 3};
    res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}
