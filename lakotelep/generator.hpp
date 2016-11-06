#pragma once

#include <random>
#include <vector>
#include "Matrix.hpp"
#include "Point.hpp"
#include "Neighbors.hpp"

Matrix<int> build(int m, int n, const std::vector<Point>& ps) {
    Matrix<int> mx(m, n);
    for (const auto p : ps) {
        mx[p] = 1;

        auto ns = getNeigbors(mx, p);
        for (const auto n : ns) {
            auto& v = mx[n];
            if (v >= 1) ++v;
            if (v > 4) v = 1;
        }
    }
    return mx;
}

Matrix<int> generate(int m, int n) {
    //std::vector<std::vector<int>> result;

    std::vector<Point> ps;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            ps.emplace_back(i, j);
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(ps.begin(), ps.end(), g);

    return build(m, n, ps);
}

bool check(const std::vector<Point>& ps, const Matrix<int>& expected) {
    return build(expected.width(), expected.height(), ps) == expected;
}
