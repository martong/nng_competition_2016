#include "all.cpp"

int main(int /*argc*/, const char* argv[]) {
    auto m = pair::generate(atoi(argv[1]), atoi(argv[2]));
    std::cerr << "Unmodified matrix:\n" << m.first <<
            "\nProblem:\n" << m.second << "\n";
    std::vector<int> numbers(6, 0);
    for (int i : m.first) {
        ++numbers[i];
    }
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        std::cerr << i << ": " << numbers[i] << "\n";
    }
    auto solution = solve(m.second);
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

