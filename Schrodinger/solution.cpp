#include <vector>
#include <numeric>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <set>

using V = std::vector<size_t>;
using S = std::set<size_t>;
using TestFun = bool (*)(const std::vector<size_t>& BallsToTest);

/// [low, high]
V range(size_t low, size_t high) {
    assert(low <= high);
    V res(high - low + 1);
    std::iota(res.begin(), res.end(), low);
    return res;
}

V range(const V& indexes, size_t low, size_t high) {
    assert(low <= high);
    V res(high - low + 1);
    std::copy(indexes.begin() + low, indexes.begin() + high + 1, res.begin());
    return res;
}

V linear(size_t NumberOfBalls, size_t RadioactiveBalls, TestFun testFun) {
    V res;
    for (size_t i = 0; i < NumberOfBalls && res.size() < RadioactiveBalls; ++i) {
        if (testFun(range(i, i))) {
            res.push_back(i);
        }
    }
    return res;
}

struct TestFunW {
    TestFun testFun;
    const V& res;
    TestFunW(TestFun testFun, const V& res) :
            testFun(testFun), res(res) {
    }
    bool operator()(V range) {
        // remove all already found radioactives from the range
        for (const auto& r : res) {
            range.erase(std::remove(range.begin(), range.end(), r),
                        range.end());
        }
        return testFun(range);
    }
};

void printLevel(int level) {
    for (int i = 0; i < level; ++i) {
        std::cerr << " ";
    }
}

void RlogN_impl(size_t low, size_t high, V& res, const V& remaining,
        S& toRemove, TestFunW& testFunW, int level = 0) {
    if (low == high) {
        res.push_back(remaining[low]);
        return;
    }
    std::size_t half = (low + high) / 2;
    //printLevel(level);
    //std::cerr << "low=" << low << " high=" << high << " toRemove=" << toRemove.size() << "\n";
    //printLevel(level);
    if (testFunW(range(remaining, low, half))) {
        //std::cerr << "True branch\n";
        RlogN_impl(low, half, res, remaining, toRemove, testFunW, level + 1);
    } else {
        //std::cerr << "False branch\n";
        std::copy(remaining.begin() + low, remaining.begin() + half + 1,
                std::inserter(toRemove, toRemove.end()));
        RlogN_impl(half + 1, high, res, remaining, toRemove, testFunW, level + 1);
    }
}

V RlogN(size_t NumberOfBalls, size_t RadioactiveBalls, TestFun testFun) {
    V res;
    V remaining(NumberOfBalls);
    std::iota(remaining.begin(), remaining.end(), 0);
    TestFunW testFunW{testFun, res};
    assert(NumberOfBalls >= 1);
    while (res.size() < RadioactiveBalls) {
        S toRemove;
        //std::cerr << "remaining: " << remaining.size() << "\n";
        RlogN_impl(0, remaining.size() - 1, res, remaining, toRemove, testFunW);
        //std::cerr << "to remove: " << toRemove.size() << "\n";
        //std::cerr << "result: " << res.size() << "\n";
        for (std::size_t i : toRemove) {
            remaining.erase(std::remove(remaining.begin(), remaining.end(), i),
                    remaining.end());
        }
    }
    return res;
}

std::vector<size_t> FindRadioactiveBalls(
    size_t NumberOfBalls, size_t RadioactiveBalls,
    bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {
    if (NumberOfBalls == RadioactiveBalls) {
        V res(NumberOfBalls);
        std::iota(res.begin(), res.end(), 0);
        return res;
    }
    if (RadioactiveBalls == 0) {
        return {};
    }

    if (NumberOfBalls < RadioactiveBalls * log2(NumberOfBalls)) {
        return linear(NumberOfBalls, RadioactiveBalls, TestFunction);
    }
    else {
        return RlogN(NumberOfBalls, RadioactiveBalls, TestFunction);
    }
}
