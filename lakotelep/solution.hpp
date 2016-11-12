#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include "Point.hpp"
#include "Matrix.hpp"
#include "Neighbors.hpp"

bool flood(std::vector<Point> st, Matrix<int>& m,
                std::vector<Point>& path) {
    std::vector<Point> nst;
    while (!st.empty()) {
        auto p = st.back();
        st.pop_back();
        if (m[p] != 1 ) { continue; }

        m[p] = 0;
        path.push_back(p);
        auto ns = getNeigbors(m, p);
        for (const auto& n : ns) {
            --m[n];
            if (m[n] == 0) {
                m[n] = 4;
            }
            if (m[n] < 0) m[n] = 0;
            if (m[n] == 1) nst.push_back(n);
        }

        // Cut offs
        for (const auto& n : ns) {
            auto nns = getNeigbors(m, n);
            auto sum_nns = 0;
            for (const auto& nn : nns) {
                sum_nns += m[nn];
            }
            if (sum_nns < m[n] - 1) {
                //std::cout << "CUT1\n";
                return false;
            }
        }

        auto fallback = !flood(nst, m, path);
        if (fallback) { return false; }
    }
    return true;
}

std::vector<Point> concat(const std::vector<Point>& v1,
                          const std::vector<Point>& v2) {
    std::vector<Point> result;
    result.reserve(v1.size() + v2.size());
    result.insert(result.end(), v1.begin(), v1.end());
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
}

// not used, flood first looks better
bool solve_exp_flood_last(std::vector<Point> S, Matrix<int> m,
                          std::vector<Point> path, std::vector<Point>& result) {
    // std::cout << S;
    // std::cout << m;
    if (path.size() == m.size()) {
        result = path;
        return true;
    }
    if (S.empty()) return false;
    auto p = S.back();
    S.pop_back();

    if (!solve_exp_flood_last(S, m, path, result)) {
        std::vector<Point> floodpath;
        if (flood({p}, m, floodpath)) {
            return solve_exp_flood_last(S, m, concat(path, floodpath), result);
        }
    }
    return false;
}

bool solve_exp_flood_first(std::vector<Point> S, Matrix<int> m,
                           std::vector<Point> path,
                           std::vector<Point>& result) {
    // std::cout << S;
    // std::cout << m;
    if (path.size() == m.size()) {
        result = path;
        return true;
    }
    if (S.empty()) return false;

    auto p = S.back();
    S.pop_back();

    auto mc = m;

    std::vector<Point> floodpath;

    if (!flood({p}, m, floodpath)) {
        return solve_exp_flood_first(S, mc, path, result);
    }
    if (!solve_exp_flood_first(S, m, concat(path, floodpath), result)) {
        return solve_exp_flood_first(S, mc, path, result);
    }
    return true;
}

std::vector<Point> solve(Matrix<int> m) {
    std::vector<Point> st;
    std::vector<Point> path;
    std::vector<Point> result;

    auto flood_ones_from_edges = [&]() {
        // gather 1s at the edges
        for (int i = 0; i < m.width(); ++i) {
            {
                Point p0{i, 0};
                if (m[p0] == 1) st.push_back(p0);
            }
            {
                Point pN(i, m.height() - 1);
                if (m[pN] == 1) st.push_back(pN);
            }
        }
        for (int j = 1; j < m.height() - 1; ++j) {
            {
                Point p0{0, j};
                if (m[p0] == 1) st.push_back(p0);
            }
            {
                Point pN(m.width() - 1, j);
                if (m[pN] == 1) st.push_back(pN);
            }
        }
        flood(st, m, path);
    };
    flood_ones_from_edges();
    //std::cout << m;

    // get the remaining ones from the middle
    st.clear();
    for (int i = 1; i < m.width() - 1; ++i) {
        for (int j = 1; j < m.height() - 1; ++j) {
            Point p{j, i};
            if (m[p] == 1) st.push_back(p);
        }
    }

    //st.clear();
    // get all ones
    //for (int i = 0; i < m.width(); ++i) {
        //for (int j = 0; j < m.height(); ++j) {
            //Point p{j, i};
            //if (m[p] == 1) st.push_back(p);
        //}
    //}

    solve_exp_flood_first(st, m, path, result);

    std::reverse(result.begin(), result.end());
    return result;
}
