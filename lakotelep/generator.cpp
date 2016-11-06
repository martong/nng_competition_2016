#include "MatrixIO.hpp"
#include "generator.hpp"

constexpr Point p02{0, 2}, p20{2, 0}, p12{1, 2}, p21{2, 1}, p22{2, 2};

int main2() {
    {
        std::vector<Point> ps{{1, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 1},
                              {2, 0}, {0, 2}, {2, 2}, {0, 0}};
        std::cout << build(3, 3, ps);
    }
    {
        std::vector<Point> ps{{1, 1}, {1, 0}, {0, 1}, {0, 0}, {2, 1},
                              {2, 0}, {1, 2}, {0, 2}, {2, 2}};
        std::cout << build(3, 3, ps);
    }
    return 0;
}

int main(int argc, char** argv) {
    auto m = generate(atoi(argv[1]), atoi(argv[2]));
    std::cout << m;
    return 0;
}
