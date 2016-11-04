#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
