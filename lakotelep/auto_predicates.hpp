#include "Neighbors.hpp"
#include "Matrix.hpp"

bool isEdgePoint(const Matrix<int>& mx, const Point& point) {
    return point.x == 0 || point.x == mx.width() - 1 ||
            point.y == 0 || point.y == mx.height() -1;
}

bool hasZeroNeighbour(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& n : neighbours) {
        if (mx[n] == 0) {
            return true;
        }
    }
    return false;
}

// This also checks whether the point is surely needed to make
// the edge two, three or four a one.
bool hasEdgeNeighbourAtLeastTwo(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& in : neighbours) {
        if (mx[in] >= 2 && isEdgePoint(mx, in)) {
            auto ns = getNeigbors(mx, in);
            int nonZeros = 0;
            for (const auto& n : ns) {
                if (mx[n] > 0) {
                    ++nonZeros;
                }
            }
            if (nonZeros < mx[in]) {
                return true;
            }
        }
    }
    return false;
}

bool hasEdgeNeighbourOne(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& n : neighbours) {
        if (mx[n] == 1 && isEdgePoint(mx, n)) {
            return true;
        }
    }
    return false;
}

bool hasNeighbourFive(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& n : neighbours) {
        if (mx[n] == 5 && !isEdgePoint(mx, n)) {
            return true;
        }
    }
    return false;
}

bool hasNeighbourOne(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& n : neighbours) {
        if (mx[n] == 1 && !isEdgePoint(mx, n)) {
            return true;
        }
    }
    return false;
}

bool hasOneNeighbourZeroNeighbour(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& in : neighbours) {
        if (mx[in] != 1) {
            continue;
        }
        auto ns = getNeigbors(mx, in);
        for (const auto& n : ns) {
            if (mx[n] == 0) {
                return true;
            }
        }
    }
    return false;
}

std::vector<Point> findNeighboursOne(const Matrix<int>& mx,
        std::vector<Point> neighbours) {
    assert(hasNeighbourOne(mx, neighbours));
    neighbours.erase(std::remove_if(neighbours.begin(), neighbours.end(),
                    [&mx](const Point& n) {
                        return mx[n] != 1;
                    }), neighbours.end());
    return neighbours;
}

bool hasNeighbourPlacedBefore(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    for (const auto& in : neighbours) {
        auto ns = getNeigbors(mx, in);
        int nonZeroNeighbours = 0;
        for (const auto& n : ns) {
            if (mx[n] != 0) {
                ++nonZeroNeighbours;
            }
        }
        if (mx[in] > nonZeroNeighbours) {
            return true;
        }
    }
    return false;
}

bool hasTooLowValueNeighbour(const Matrix<int>& mx,
        const std::vector<Point>& neighbours, const Point& p) {
    for (const auto& in : neighbours) {
        if (mx[in] == 1) {
            continue;
        }
        auto ns = getNeigbors(mx, in);
        bool areGreater = true;
        for (const auto& n : ns) {
            if (n == p) {
                continue;
            }
            // FIXME: >=?
            areGreater &= mx[in] >= mx[n] || mx[n] == 0;
        }
        if (areGreater) {
            return true;
        }
    }
    return false;
}

// Has a neighbour that has the number equals to its non-zero
// neighbours
// 0 2 2 2
// 0 2 1 2
// 0 0 3 3
// 1,1 depends on the 1 on 3,2
bool hasNeighbourDependingOnThis(const Matrix<int>& mx,
        const std::vector<Point>& neighbours) {
    return false;
}
