#include "all.cpp"

int main(int /*argc*/, const char* argv[]) {
    auto m = pair::generate(atoi(argv[1]), atoi(argv[2]));
    std::cerr << "Unmodified matrix:\n" << m.first <<
            "\nProblem:\n" << m.second << "\n";
    auto solution = solve(m.second, m.first);
    if (check(solution, m.second)) {
        std::cerr << "OK\n";
    } else {
        std::cerr << "bad\n";
    }

    for (Point p : solution) {
        std::cout << p << " ";
    }
    std::cout << "\n";
}
