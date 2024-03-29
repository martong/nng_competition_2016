#include <gtest/gtest.h>
#include <gmock/gmock.h>

//#include "generator.hpp"
//#include "Neighbors.hpp"
//#include "solution.hpp"
//#include "MatrixIO.hpp"
#include "all.cpp"

using namespace ::testing;

TEST(getNeighbors, first) {
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

TEST(getAllNeigbors, first) {
    {
        Matrix<int> m{2, 2};
        auto r = getAllNeigbors(m, {0, 0});
        EXPECT_EQ(r.size(), 4);
    }
    {
        Matrix<int> m{2, 2};
        m[p00] = 1;
        m[p10] = 1;
        m[p01] = 1;
        m[p11] = 1;

        auto r = getAllNeigbors(m, {0, 0}, 1);
        EXPECT_EQ(r.size(), 2);
        EXPECT_THAT(
            r, UnorderedElementsAre(Point{0, 1}, Point{1, 0}));

    }
    {
        Matrix<int> m{2, 2};
        m[p00] = 1;
        m[p10] = 1;
        m[p01] = 1;
        m[p11] = 1;

        auto r = getAllNeigbors(m, {0, 0}, 0);
        EXPECT_EQ(r.size(), 2);
        EXPECT_THAT(
            r, UnorderedElementsAre(Point{0, -1}, Point{-1, 0}));
    }
    {
        Matrix<int> m{2, 2};
        m[p00] = 1;
        m[p10] = 0;
        m[p01] = 1;
        m[p11] = 1;

        auto r = getAllNeigbors(m, {0, 0}, 0);
        EXPECT_EQ(r.size(), 3);
        EXPECT_THAT(
            r, UnorderedElementsAre(Point{0, -1}, Point{-1, 0}, Point{1, 0}));
    }
}

TEST(getAllNeigbors_not, first) {
    {
        Matrix<int> m{2, 2};
        auto r = getAllNeigbors_not(m, {0, 0}, 0);
        EXPECT_EQ(r.size(), 0);
    }
    {
        Matrix<int> m{2, 2};
        m[p00] = 1;
        m[p10] = 2;
        m[p01] = 1;
        m[p11] = 1;

        auto r = getAllNeigbors_not(m, {0, 0}, 1);
        EXPECT_EQ(r.size(), 3);
        EXPECT_THAT(
            r, UnorderedElementsAre(Point{1, 0}, Point{0, -1}, Point{-1, 0}));

    }
}

TEST(get_group, first) {
    Matrix<int> m{3, 4};
    m[{0,0}] = 3;
    m[{1,0}] = 1;
    m[{2,0}] = 3;

    m[{0,1}] = 1;
    m[{1,1}] = 1;
    m[{2,1}] = 3;

    m[{0,2}] = 3;
    m[{1,2}] = 1;
    m[{2,2}] = 3;

    m[{0,3}] = 3;
    m[{1,3}] = 1;
    m[{2,3}] = 3;
    auto g = get_group(m, {1,1}, 1);
    EXPECT_EQ(g.size(), 5);
    EXPECT_EQ(g[0], Point(1, 1));
    EXPECT_EQ(g[1], Point(1, 2)); // This must be a neighbor
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

TEST(solution, 2x2) {
    Matrix<int> m{2, 2};
    m[p00] = 1;
    m[p10] = 3;
    m[p01] = 3;
    m[p11] = 1;
    std::cout << m;

    auto ps = solve(m);
    std::cout << ps;

    auto m2 = build(m.width(), m.height(), ps);
    std::cout << m2;

    //EXPECT_TRUE(m2 == m);
    EXPECT_TRUE(check(ps, m));
}

TEST(M2x2, many) {
    for (int i = 0; i < 100; ++i) {
        auto m = generate(2,2);
        std::cout << m;
        auto ps = solve(m);
        if (!check(ps, m)) {
            auto m2 = build(m.width(), m.height(), ps);
            std::cout << ps << m2;
        }
        EXPECT_TRUE(check(ps, m));
    }
}

//TEST(solution, solve2) {
    //Matrix<int> m{2, 2};
    //m[p00] = 1;
    //m[p10] = 3;
    //m[p01] = 3;
    //m[p11] = 1;
    //std::cout << m;

    //auto ps = solve2(m);
    //std::cout << ps;

    //auto m2 = build(m.width(), m.height(), ps);
    //std::cout << m2;

    ////EXPECT_TRUE(m2 == m);
    //EXPECT_TRUE(check(ps, m));
//}

TEST(solution, 3x3) {
    Matrix<int> m{3, 3};
    m[{0,0}] = 1;
    m[{1,0}] = 3;
    m[{2,0}] = 1;

    m[{0,1}] = 3;
    m[{1,1}] = 1;
    m[{2,1}] = 3;

    m[{0,2}] = 1;
    m[{1,2}] = 3;
    m[{2,2}] = 1;
    std::cout << m;

    auto ps = solve(m);
    std::cout << ps;

    auto m2 = build(m.width(), m.height(), ps);
    std::cout << m2;

    EXPECT_TRUE(check(ps, m));
}

TEST(M3x3, segfault) {
    Matrix<int> m{3, 3};
    m[{0,0}] = 3;
    m[{1,0}] = 1;
    m[{2,0}] = 2;

    m[{0,1}] = 1;
    m[{1,1}] = 1;
    m[{2,1}] = 3;

    m[{0,2}] = 2;
    m[{1,2}] = 3;
    m[{2,2}] = 1;
    std::cout << m;

    auto ps = solve(m);
    std::cout << ps;

    auto m2 = build(m.width(), m.height(), ps);
    std::cout << m2;

    EXPECT_TRUE(check(ps, m));
}

TEST(M3x3, fault) {
    Matrix<int> m{3, 3};
    m[{0,0}] = 1;
    m[{1,0}] = 3;
    m[{2,0}] = 1;

    m[{0,1}] = 3;
    m[{1,1}] = 1;
    m[{2,1}] = 2;

    m[{0,2}] = 1;
    m[{1,2}] = 3;
    m[{2,2}] = 2;
    std::cout << m;

    auto ps = solve(m);
    std::cout << ps;

    auto m2 = build(m.width(), m.height(), ps);
    std::cout << m2;

    EXPECT_TRUE(check(ps, m));
}

TEST(M3x3, fault2) {
    Matrix<int> m{3, 3};
    m[{0,0}] = 1;
    m[{1,0}] = 3;
    m[{2,0}] = 1;

    m[{0,1}] = 2;
    m[{1,1}] = 1;
    m[{2,1}] = 2;

    m[{0,2}] = 2;
    m[{1,2}] = 2;
    m[{2,2}] = 3;
    std::cout << m;

    auto ps = solve(m);
    std::cout << ps;

    auto m2 = build(m.width(), m.height(), ps);
    std::cout << m2;

    EXPECT_TRUE(check(ps, m));
}


TEST(M3x3, many) {
    for (int i = 0; i < 1000; ++i) {
        auto m = generate(3,3);
        std::cout << i << " Generated:" << m;
        auto ps = solve(m);
        if (!check(ps, m)) {
            auto m2 = build(m.width(), m.height(), ps);
            std::cout << "Result:" << ps << "Result generates:" << m2;
        }
        ASSERT_TRUE(check(ps, m));
    }
}

TEST(M4x4, many) {
    for (int i = 0; i < 1000; ++i) {
        auto m = generate(4,4);
        std::cout << i << " Generated:" << m;
        auto ps = solve(m);
        if (!check(ps, m)) {
            auto m2 = build(m.width(), m.height(), ps);
            std::cout << "Result:" << ps << "Result generates:" << m2;
        }
        ASSERT_TRUE(check(ps, m));
    }
}

TEST(M5x4, many) {
    for (int i = 0; i < 1000; ++i) {
        auto m = generate(5,4);
        std::cout << i << " Generated:" << m;
        auto ps = solve(m);
        if (!check(ps, m)) {
            auto m2 = build(m.width(), m.height(), ps);
            std::cout << "Result:" << ps << "Result generates:" << m2;
        }
        ASSERT_TRUE(check(ps, m));
    }
}

TEST(Big, xxx) {
    auto m = generate(13, 13);
    std::cout << " Generated:" << m;
    auto ps = solve(m);
    EXPECT_TRUE(check(ps, m));
}

TEST(Big, seq) {
    for (int N = 5; N < 12; ++N) {
        for (int i = 0; i < 10; ++i) {
            auto m = pair::generate(N, N);
            std::cerr << "Unmodified matrix:\n" << m.first << "\nProblem:\n"
                      << m.second << "\n";
            auto ps = solve(m.second, m.first);
            EXPECT_TRUE(check(ps, m.second));
        }
    }
}
