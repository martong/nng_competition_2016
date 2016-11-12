#include "generator.hpp"
#include "solution.hpp"
#include "MatrixIO.hpp"

#include <iostream>

int main(int /*argc*/, const char* argv[]) {
    auto m = generate(atoi(argv[1]), atoi(argv[2]));
    std::cerr << "Unmodified matrix:\n" << m.first <<
            "\nProblem:\n" << m.second << "\n";
    auto solution = solve(m.second);
    if (check(solution, m.second)) {
        std::cerr << "OK\n";
    } else {
        std::cerr << "bad\n";
    }

    std::cout << solution << "\n";
}
