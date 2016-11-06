#include <vector>
#include <stack>
#include <iostream>
#include "Point.hpp"
#include "Matrix.hpp"
#include "Neighbors.hpp"

std::ostream& operator<<(std::ostream& os, const std::vector<Point>& ps){
    for (const auto& p : ps) {
        os << p;
    }
    os << "\n";
    return os;
}

std::vector<Point> solve(Matrix<int> m) {
    std::stack<Point> stack;
    std::vector<Point> result;

    for (int i = 0; i < m.width(); ++i) {
        for (int j = 0; j < m.height(); ++j) {
            Point ij{i,j};
            if (m[ij] == 1 ) stack.push(ij);
        }
    }

    while (!stack.empty()) {
        auto p = stack.top();
        stack.pop();
        while (!stack.empty() && m[p] != 1) {
            p = stack.top(); stack.pop();
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
