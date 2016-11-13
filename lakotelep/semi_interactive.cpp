#include "Neighbors.hpp"
#include "MatrixIO.hpp"
#include "generator.hpp"
#include "auto_predicates.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iostream>

#include <readline/history.h>
#include <readline/readline.h>

bool removeOne(std::pair<Matrix<int>, Matrix<int>>& mxPair,
        const Point& point) {
    if (mxPair.first[point] != 1) {
        std::cout << "Wrong one selected: (" << point.x << ',' <<point.y
                  << std::endl;
        return false;
    }
    auto neighbours = getNeigbors(mxPair.first, point);
    for (const auto& neighbour : neighbours) {
        if (mxPair.second[neighbour] == 1) {
            std::cerr << "You cannot remove an 1 next to another 1\n";
            return false;
        }
    }
    --mxPair.first[point];
    --mxPair.second[point];
    for (const auto& neighbour : neighbours) {
        if (mxPair.first[neighbour] > 0) {
            --mxPair.first[neighbour];
            --mxPair.second[neighbour];
        }
    }
    return true;
}

Point getPoint(const std::string& word) {
    Point point;
    std::size_t sep = word.find(",");
    point.x = boost::lexical_cast<std::size_t>(
            word.substr(0, sep));
    point.y = boost::lexical_cast<std::size_t>(
            word.substr(sep+1));
    return point;
}

bool isEndState(const std::pair<Matrix<int>, Matrix<int>>& mxPair) {
    bool isEnd = true;
    for (const auto& elem : mxPair.first) {
        isEnd &= (elem == 0);
    }
    return isEnd;
}

bool setFive(std::pair<Matrix<int>, Matrix<int>>& mxPair, const Point& point) {
    if (mxPair.first[point] == 5) {
        mxPair.second[point] = 5;
        return true;
    }
    return false;
}

// AUTOMATIC MOVES

std::vector<Point> findEdgeOnes(const Matrix<int>& matrix) {
    std::vector<Point> edgeOnes;
    for (std::size_t i = 0; i < matrix.width(); i += matrix.width() - 1) {
        for (std::size_t j = 1; j < matrix.height() - 1; ++j) {
            Point p(i, j);
            if (matrix[p] == 1) {
                edgeOnes.push_back(p);
            }
        }
    }
    for (std::size_t i = 1; i < matrix.width() - 1; ++i) {
        for (std::size_t j = 0; j < matrix.height(); j += matrix.height() - 1) {
            Point p(i, j);
            if (matrix[p] == 1) {
                edgeOnes.push_back(p);
            }
        }
    }
    std::vector<Point> corners{Point(0,0), Point(matrix.width()-1, 0),
                Point(0, matrix.height()-1),
                Point(matrix.width()-1, matrix.height()-1)};
    for (const auto& c : corners) {
        if (matrix[c] == 1) {
            edgeOnes.push_back(c);
        }
    }
    return edgeOnes;
}

std::vector<Point> findAllOnes(const Matrix<int>& matrix) {
    std::vector<Point> ones;
    for (std::size_t i = 0; i < matrix.width(); ++i) {
        for (std::size_t j = 0; j < matrix.height(); ++j) {
            Point p(i, j);
            if (matrix[p] == 1) {
                ones.push_back(p);
            }
        }
    }
    return ones;
}

std::vector<Point> findFives(const Matrix<int>& matrix) {
    std::vector<Point> ones = findAllOnes(matrix);
    std::vector<Point> fives;
    for (const auto& p : ones) {
        if (isEdgePoint(matrix, p)) {
            if (p.x == 0) {
                Point five(1, p.y);
                if (matrix[five] == 1) {
                    fives.push_back(five);
                }
            } else if (p.x == matrix.width() - 1) {
                Point five(p.x - 1, p.y);
                if (matrix[five] == 1) {
                    fives.push_back(five);
                }
            } else if (p.y == 0) {
                Point five(p.x, 1);
                if (matrix[five] == 1) {
                    fives.push_back(five);
                }
            } else if (p.y == matrix.height() - 1) {
                Point five(p.x, p.y - 1);
                if (matrix[five] == 1) {
                    fives.push_back(five);
                }
            }
        } else {
            auto neighbours = getNeigbors(matrix, p);
            if (hasEdgeNeighbourOne(matrix, neighbours)) {
                fives.push_back(p);
            } else if (hasOneNeighbourZeroNeighbour(matrix, neighbours)) {
                fives.push_back(p);
            } else if (true || hasNeighbourOne(matrix, neighbours)) {
                for (const auto& in : neighbours) {
                    if (matrix[in] != 1) {
                        continue;
                    }
                    auto ns = getNeigbors(matrix, in);
                    if (hasNeighbourPlacedBefore(matrix, ns)) {
                        fives.push_back(p);
                        return fives;
                    }
                }
            } else if (hasNeighbourOne(matrix, neighbours)) {
                for (const auto& in : neighbours) {
                    if (matrix[in] == 1 && in != p &&
                            hasTooLowValueNeighbour(matrix,
                                    getNeigbors(matrix, in), p)) {
                        fives.push_back(p);
                        return fives;
                    }
                }
                // this does not seem to work
                for (const auto& in : neighbours) {
                    if (matrix[in] == 1 && in != p) {
                        auto ns = getNeigbors(matrix, in);
                        if (std::any_of(ns.begin(), ns.end(),
                                [&matrix](const Point& n){
                                    auto nns = getNeigbors(matrix, n);
                                    return hasNeighbourPlacedBefore(matrix, nns);
                                })) {
                            fives.push_back(p);
                            return fives;
                        }
                    }
                }
            }
            //if (hasEdgeNeighbourTwo(matrix, neighbours) &&
            //        hasNeighbourOne(matrix, neighbours)) {
            //    auto oneNeighb = findNeighboursOne(matrix, neighbours);
            //    if (oneNeighb.size() == 1) {
            //        fives.push_back(oneNeighb.front());
            //    } else {
            //        fives.push_back(p);
            //    }
            //}
        }
    }
    return fives;
}

void doAutomaticMove(std::pair<Matrix<int>, Matrix<int>>& mxPair,
        std::vector<Point>& pointsRemoved) {
    bool moveDone = false;
    std::vector<Point> fives = findFives(mxPair.second);
    for (const auto& p : fives) {
        if (setFive(mxPair, p)) {
            std::cout << "Found a 5 on (" << p.x << ',' << p.y << ")"
                      << std::endl;
            moveDone = true;
        } else {
            std::cout << "Invalid 5 found on (" << p.x << ',' << p.y << ")"
                      << std::endl;
        }
    }
    if (moveDone) return;
    std::vector<Point> edgeOnes = findEdgeOnes(mxPair.second);
    for (const auto& p : edgeOnes) {
        std::cout << "Removing edge one (" << p.x << ',' << p.y << ")"
                  << std::endl;
        pointsRemoved.push_back(p);
        removeOne(mxPair, p);
        moveDone = true;
    }
    if (moveDone) return;
    std::vector<Point> ones = findAllOnes(mxPair.second);
    if (ones.size() == 1) {
        Point p = ones.front();
        std::cout << "Removing the only one (" << p.x << ',' << p.y << ")"
                  << std::endl;
        pointsRemoved.push_back(p);
        removeOne(mxPair, p);
    } else {
        for (const auto& point : ones) {
            std::cout << "considering (" << point.x << ','
                      << point.y << ")" << std::endl;
            auto neighbours = getNeigbors(mxPair.second, point);
            if (hasZeroNeighbour(mxPair.second, neighbours)) {
                std::cout << "Removing zero neighbour one (" << point.x << ','
                          << point.y << ")" << std::endl;
                pointsRemoved.push_back(point);
                removeOne(mxPair, point);
                return;
                //} else if (hasEdgeNeigbourThree(mxPair.second, neighbours)  &&
                //        !hasNeighbourOne(mxPair.second, neighbours)) {
                //
            } else if (hasEdgeNeighbourAtLeastTwo(mxPair.second, neighbours) &&
                    !hasNeighbourOne(mxPair.second, neighbours)) {
                //if (std::all_of(neighbours.begin(), neighbours.end(),
                //                [&mxPair, &point](const Point& n) {
                //                    return point != n &&
                //                            !hasNeighbourOne(mxPair.second,
                //                                    getNeigbors(mxPair.second, n));
                //                })) {
                    std::cout << "Two point edge neighbour one (" << point.x << ','
                              << point.y << ")" << std::endl;
                    pointsRemoved.push_back(point);
                    removeOne(mxPair, point);
                    return;
                //}
            } else if (hasNeighbourFive(mxPair.second, neighbours)) {
                std::cout << "Removing 5 neighbour one (" << point.x << ','
                          << point.y << ")" << std::endl;
                pointsRemoved.push_back(point);
                removeOne(mxPair, point);
                return;
            } else if (hasNeighbourPlacedBefore(mxPair.second, neighbours)) {
                std::cout << "Has neighbour placed before (" << point.x << ','
                          << point.y << ")" << std::endl;
                pointsRemoved.push_back(point);
                removeOne(mxPair, point);
                return;
            } else if (!hasNeighbourOne(mxPair.second, neighbours)) {
                auto ns = getNeigbors(mxPair.second, point);
                if (std::all_of(neighbours.begin(), neighbours.end(),
                                [&mxPair](const Point& n) {
                                    return !hasNeighbourOne(mxPair.second,
                                            getNeigbors(mxPair.second, n));
                                })) {
                    std::cout << "Neighbours neighbour doesn't have 1, "
                            "removing (" << point.x << ','
                              << point.y << ")" << std::endl;
                    pointsRemoved.push_back(point);
                    removeOne(mxPair, point);
                    return;
                }
            } else if (!hasNeighbourOne(mxPair.second, neighbours) &&
                    hasTooLowValueNeighbour(mxPair.second, neighbours, point)) {
                std::cout << "Has too low value neighbour: (" << point.x << ','
                          << point.y << ")" << std::endl;
                pointsRemoved.push_back(point);
                removeOne(mxPair, point);
            } else if (!hasNeighbourOne(mxPair.second, neighbours)) {
                for (const auto& in : neighbours) {
                    auto ns = getNeigbors(mxPair.second, in);
                    if (hasNeighbourPlacedBefore(mxPair.second, ns)) {
                        std::cout << "Neighbour's neighbour placed before ("
                                  << point.x << ',' << point.y << ")" << std::endl;
                        pointsRemoved.push_back(point);
                        removeOne(mxPair, point);
                    }
                }
            } else if (!hasNeighbourOne(mxPair.second, neighbours)) {
                if (std::any_of(neighbours.begin(), neighbours.end(),
                                [&mxPair](const Point& in) {
                                    auto ns = getNeigbors(mxPair.second, in);
                                    return !hasNeighbourOne(mxPair.second, ns) &&
                                            hasNeighbourPlacedBefore(mxPair.second, ns);
                                })) {
                    std::cout << "Neighbour has a neighbour placed before ("
                              << point.x << ',' << point.y << ")" << std::endl;
                    pointsRemoved.push_back(point);
                    removeOne(mxPair, point);
                }
            }
        }
    }
}

bool readLine(std::vector<std::string>& words) {
    std::unique_ptr<char[], void(*)(void*)> rawLine{
            readline("> "), free};
    if (!rawLine) {
        return false;
    }
    if (rawLine[0] != 0) {
        add_history(rawLine.get());
    }
    std::string line = rawLine.get();
    boost::algorithm::split(words, line, boost::algorithm::is_space());
    return true;
}

int main(int argc, char* argv[]) {
    assert(argc == 3);

    auto mxPair = generate(boost::lexical_cast<int>(argv[1]),
            boost::lexical_cast<int>(argv[2]));

    std::cout << "q: quit; b: back; r x,y: remove 1 from x,y; 5 x,y; "
            "x,y is identified as a 5; a: automatic move" << std::endl;

    std::vector<std::string> words;
    std::vector<std::pair<Matrix<int>, Matrix<int>>> states;
    states.push_back(mxPair);
    std::vector<Point> pointsRemoved;
    do {
        if (words.empty()) {
            if (words[0] == "q") {
                return 0;
            } else if (words[0] == "b" && states.size() > 0) {
                states.pop_back();
                mxPair = states.back();
            } else if (words[0] == "r") {
                Point point = getPoint(words[1]);
                if (mxPair.first[point] != 1) {
                    std::cout << point.x << ',' << point.y
                              << "  is not 1!" << std::endl;
                    continue;
                }
                if (removeOne(mxPair, point)) {
                    states.push_back(mxPair);
                    pointsRemoved.push_back(point);
                }
            } else if (words[0] == "5") {
                Point point = getPoint(words[1]);
                if (!setFive(mxPair, point)) {
                    std::cout << point.x << ',' << point.y << " is not 5!"
                              << std::endl;
                }
            } else if (words[0] == "a") {
                doAutomaticMove(mxPair, pointsRemoved);
                states.push_back(mxPair);
            } else {
                std::cout << "Unknown command: " << words[0] << std::endl;
                continue;
            }
        }

        //states.push_back(mxPair);
        std::cerr << mxPair.first;
        std::cout << mxPair.second;
        if (isEndState(mxPair)) {
            std::cout << "Solved with points removed: ";
            for (const auto& point : pointsRemoved) {
                std::cout << '(' << point.x << ',' << point.y << ") ";
            }
            std::cout << std::endl;
            return 0;
        }
    } while (readLine(words));
}
