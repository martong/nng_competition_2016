#pragma once

#include <vector>
#include <algorithm>
#include "Point.hpp"
#include "Matrix.hpp"

template <typename T>
std::vector<Point> getNeigbors(const Matrix<T>& m, Point p) {
    std::vector<Point> result = {p + p01, p - p01, p + p10, p - p10};
    result.erase(std::remove_if(result.begin(), result.end(), [&m](auto& p) {
                     return !isInsideMatrix(m, p);
                 }), result.end());
    return result;
}

// TODO getDiagonalNeigbors, getAllNeigbors
