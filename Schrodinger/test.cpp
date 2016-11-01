#include <iostream>
#include <gtest/gtest.h>
#include "solution.cpp"

using V = std::vector<size_t>;

V radioactives;
static int called = 0;

std::ostream& operator<<(std::ostream& os, const V& v) {
    for (const auto& b : v) {
        os << b << " ";
    }
    os << "\n";
    return os;
}

/// Returns true if there is at least one radioactive ball in the set
bool testFun(const V& ballsToTest) {
    ++called;
    std::cout << ballsToTest;
    for (const auto& b : ballsToTest) {
        auto it = std::find(radioactives.begin(), radioactives.end(), b);
        if (it != std::end(radioactives)) {
            return true;
        }
    }
    return false;
}

struct Result : ::testing::Test {
    Result() { called = 0; }
    ~Result() {
        std::cout << "TestFun called " << called << " times." << std::endl;
    }
};

TEST(Range, range) {
    V v = range(0, 3);
    V exp = {0, 1, 2, 3};
    EXPECT_EQ(v, exp);
    v = range(1, 1);
    exp = {1};
    EXPECT_EQ(v, exp);
}

TEST(TestFunW, trueCase) {
    radioactives = {0, 1, 2, 3};
    V res = {0, 3};
    TestFunW testFunW{testFun, res};
    EXPECT_EQ(testFunW(range(0, 3)), true);
}

TEST(TestFunW, oneInRes) {
    radioactives = {0, 3};
    V res = {0};
    TestFunW testFunW{testFun, res};
    EXPECT_EQ(testFunW(range(0, 0)), false);
    {
        V res = {};
        TestFunW testFunW{testFun, res};
        EXPECT_EQ(testFunW(range(0, 0)), true);
    }
}

TEST_F(Result, AllRadioactive) {
    V balls = {0, 1, 2, 3};
    radioactives = {0, 1, 2, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, ZeroRadioactive) {
    V balls = {0, 1, 2, 3};
    radioactives = {};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeFour) {
    V balls = {0, 1, 2, 3};
    radioactives = {0, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeFour1) {
    V balls = {0, 1, 2, 3};
    radioactives = {1, 2};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeFour2) {
    V balls = {0, 1, 2, 3};
    radioactives = {1};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeFour3) {
    V balls = {0, 1, 2, 3};
    radioactives = {0, 1, 3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeEight) {
    V balls = range(0, 7);
    radioactives = {0, 4};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
}

TEST_F(Result, SizeEight1) {
    V balls = range(0, 7);
    radioactives = {3};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 3);
}
TEST_F(Result, SizeEight2) {
    V balls = range(0, 7);
    radioactives = {0, 7};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 6);
}
TEST_F(Result, SizeEight3) {
    V balls = range(0, 7);
    radioactives = {0,2,4};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 8);
}
TEST_F(Result, SizeEight4) {
    V balls = range(0, 7);
    radioactives = {0,2,4,6};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 8);
}

TEST_F(Result, Size7) {
    V balls = range(0, 6);
    radioactives = {0,2,4,6};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 7);
}
TEST_F(Result, Size9) {
    V balls = range(0, 8);
    radioactives = {0,2,4,6};
    auto res = FindRadioactiveBalls(balls.size(), radioactives.size(), testFun);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(res, radioactives);
    EXPECT_EQ(called, 9);
}

