#include "Neighbors.hpp"
#include "MatrixIO.hpp"
#include "generator.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iostream>
#include <memory>

#include <readline/history.h>
#include <readline/readline.h>

void removeOne(std::pair<Matrix<int>, Matrix<int>>& mxPair, const Point& point) {
    mxPair.first[point] = 0;
    mxPair.second[point] = 0;
    auto neighbours = getNeigbors(mxPair.first, point);
    for (const auto& neighbour : neighbours) {
        if (mxPair.first[neighbour] > 0) {
            --mxPair.first[neighbour];
            if (--mxPair.second[neighbour] == 0) {
                mxPair.second[neighbour] = 4;
            }
        }
    }
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

int main(int argc, char* argv[]) {
    assert(argc == 3);

    auto mxPair = generate(boost::lexical_cast<int>(argv[1]),
            boost::lexical_cast<int>(argv[2]));

    std::cout << "q: quit; b: back; r x,y: remove 1 from x,y; 5 x,y; "
            "x,y is identified as a 5" << std::endl;

    std::vector<std::string> words;
    std::vector<std::pair<Matrix<int>, Matrix<int>>> states;
    states.push_back(mxPair);
    std::vector<Point> pointsRemoved;
    do {
        if (!words.empty()) {
            if (words[0] == "q") {
                return 0;
            } else if (words[0] == "b" && states.size() > 0) {
                mxPair = states.back();
                states.pop_back();
            } else if (words[0] == "r") {
                Point point = getPoint(words[1]);
                if (mxPair.first[point] != 1) {
                    std::cout << point.x << ',' << point.y
                              << "  is not 1!" << std::endl;
                    continue;
                }
                removeOne(mxPair, point);
                states.push_back(mxPair);
                pointsRemoved.push_back(point);
            } else if (words[0] == "5") {
                Point point = getPoint(words[1]);
                if (!setFive(mxPair, point)) {
                    std::cout << point.x << ',' << point.y << " is not 5!"
                              << std::endl;
                }
            } else {
                std::cout << "Unknown command: " << words[0] << std::endl;
                continue;
            }
        }

        //states.push_back(mxPair);
        //std::cerr << mxPair.first;
        std::cout << mxPair.second;
        if (isEndState(mxPair)) {
            std::cout << "Solved with points removed: ";
            for (const auto& point : pointsRemoved) {
                std::cout << '(' << point.x << ',' << point.y << ") ";
            }
            std::cout << std::endl;
            return 0;
        }
        std::unique_ptr<char[], void(*)(void*)> rawLine{
                readline("> "), free};
        if (!rawLine) {
            break;
        }
        if (rawLine[0] != 0) {
            add_history(rawLine.get());
        }
        std::string line = rawLine.get();
        boost::algorithm::split(words, line, boost::algorithm::is_space());
    } while (std::cin.good());
}
