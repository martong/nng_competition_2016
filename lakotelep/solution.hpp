#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include "Point.hpp"
#include "Matrix.hpp"
#include "Neighbors.hpp"

std::ostream& operator<<(std::ostream& os, const std::vector<Point>& ps) {
    for (const auto& p : ps) {
        os << p;
    }
    os << "\n";
    return os;
}
std::ostream& operator<<(std::ostream& os, std::stack<Point> ps) {
    while (!ps.empty()) {
        auto p = ps.top();
        ps.pop();
        os << p;
    }
    os << "\n";
    return os;
}

std::vector<Point> solve2impl(std::vector<Point>& st, Matrix<int>& m,
                              std::vector<Point>& result,
                              Point* parent = nullptr) {
    //if (parent) std::cout << "parent: " << *parent << "\n";
    assert(m.size() > 0);
    if (result.size() == m.size()) {
        // std::cout << result;
        return result;
    }

    std::vector<Point> st2;
    while (!st.empty()) {

        auto p = st.back();
        //std::cout << "p:" << p << "\n";
        st2.push_back(p);
        st.pop_back();

        auto stc = st;
        auto mc = m;
        auto resultc = result;

        assert(m[p] == 1);

        m[p] = 0;
        result.push_back(p);

        std::vector<Point> st3;

        auto ns = getNeigbors(m, p);
        for (const auto& n : ns) {
            --m[n];
            if (m[n] == 0) {
                m[n] = 4;
                st.erase(std::remove(st.begin(), st.end(), n), st.end());
                //auto it = std::remove(st.begin(), st.end(), n);
                //std::copy(it, st.end(), std::back_inserter(st3));
                //st.erase(it, st.end());
            }
            if (m[n] < 0) m[n] = 0;
            if (m[n] == 1) st.push_back(n);
        }

        // recurse
        auto r = solve2impl(st, m, result, &p);
        if (!r.empty()) {
            return r;
        }

        // restore
        for (const auto& n : ns) {
            auto& v = m[n];
            if (v >= 1) ++v;
            if (v > 4) {
                v = 1;
                //std::cout << "n1:" << n << "\n";
                //auto it = std::find(st.begin(), st.end(), n);
                // if (!st.empty() && it == std::end(st)) {
                // st.push_back(n);
                //}
                //if (it == std::end(st)) {
                    //if (st.empty()) {
                        //auto it2 = std::find(st2.begin(), st2.end(), n);
                        //if (it2 == std::end(st2)) {
                            //st.push_back(n);
                        //}
                    //} else {
                        //st.push_back(n);
                    //}
                //}
            }
        }
        std::copy(st3.begin(), st3.end(), std::back_inserter(st));

        result.pop_back();
        m[p] = 1;
        st = stc;

        //if (st != stc) {
            //std::cout << "st:" << st << "stc:" << stc << std::endl;
        //}
        assert(st == stc);
        assert(m == mc);
        assert(result == resultc);
    }
    // return result;
    //std::cout << "DEAD "
              //<< "\n";
    return {};
}

std::vector<Point> solve(Matrix<int> m) {
    std::vector<Point> st;
    std::vector<Point> result;
    for (int i = 0; i < m.width(); ++i) {
        for (int j = 0; j < m.height(); ++j) {
            Point p{j, i};
            if (m[p] == 1) st.push_back(p);
        }
    }
    auto r = solve2impl(st, m, result);
    std::reverse(r.begin(), r.end());
    return r;
}

std::vector<Point> solveX(Matrix<int> m) {
    std::stack<Point> stack;
    std::vector<Point> result;

    for (int i = 0; i < m.width(); ++i) {
        for (int j = 0; j < m.height(); ++j) {
            Point ij{i, j};
            if (m[ij] == 1) stack.push(ij);
        }
    }

    while (!stack.empty()) {
        auto p = stack.top();
        stack.pop();
        while (!stack.empty() && m[p] != 1) {
            p = stack.top();
            stack.pop();
        }

        m[p] = 0;
        result.push_back(p);

        auto ns = getNeigbors(m, p);
        for (const auto& n : ns) {
            --m[n];
            if (m[n] == 0) m[n] = 4;
            if (m[n] < 0) m[n] = 0;
            if (m[n] == 1) stack.push(n);
        }
    }

    std::vector<Point> final_result;
    auto b = result.rbegin();
    auto e = result.rbegin() + m.width() * m.height();
    std::copy(b, e, std::back_inserter(final_result));

    return final_result;
}
