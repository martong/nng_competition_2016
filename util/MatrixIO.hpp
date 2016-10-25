#include "Matrix.hpp"
#include "DumperFunctions.hpp"
#include <ostream>

template<typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix)
{
    dumpMatrix(os, matrix, " ");
    return os;
}




