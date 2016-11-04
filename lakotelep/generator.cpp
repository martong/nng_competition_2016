#include "MatrixIO.hpp"
#include "generator.hpp"

int main(int argc, char** argv) {
    auto m = generate(atoi(argv[1]), atoi(argv[2]));
    std::cout << m << std::endl;
    return 0;
}
