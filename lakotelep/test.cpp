#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "generator.hpp"
#include "Neighbors.hpp"

using namespace ::testing;

TEST(Neighbor, first) {
    {
        Matrix<int> m{2, 2};
        auto r = getNeigbors(m, {0, 0});
        EXPECT_EQ(r.size(), 2);
        // EXPECT_THAT(r, Contains(Point{0,0}));
        EXPECT_THAT(r, UnorderedElementsAre(Point{1, 0}, Point{0, 1}));
    }
    {
        Matrix<int> m{3, 3};
        auto r = getNeigbors(m, {2, 2});
        EXPECT_EQ(r.size(), 2);
        EXPECT_THAT(r, UnorderedElementsAre(Point{1, 2}, Point{2, 1}));
    }
    {
        Matrix<int> m{3, 3};
        auto r = getNeigbors(m, {1, 1});
        EXPECT_EQ(r.size(), 4);
        EXPECT_THAT(r, UnorderedElementsAre(Point{2, 1}, Point{1, 2},
                                            Point{0, 1}, Point{1, 0}));
    }
}

TEST(check, first) {
    Matrix<int> m{2, 2};
    m[p00] = 1;
    m[p10] = 3;
    m[p01] = 3;
    m[p11] = 1;
    {
        std::vector<Point> ps = {p01, p10, p11, p00};
        EXPECT_TRUE(check(ps, m));
    }
    {
        std::vector<Point> ps = {p10, p01, p11, p00};
        EXPECT_TRUE(check(ps, m));
    }
    {
        std::vector<Point> ps = {p10, p01, p00, p11};
        EXPECT_TRUE(check(ps, m));
    }
    {
        std::vector<Point> ps = {p01, p10, p00, p11};
        EXPECT_TRUE(check(ps, m));
    }
    {
        std::vector<Point> ps = {p00, p10, p01, p11};
        EXPECT_FALSE(check(ps, m));
    }
}
