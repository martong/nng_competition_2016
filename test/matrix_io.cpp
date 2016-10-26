#include <Matrix.hpp>
#include <MatrixIO.hpp>

#include <iostream>

int main() {
    while (std::cin.good()) {
        auto matrix = loadMatrix(std::cin, '\\');
        std::cout << matrix << std::endl;
    }
}
