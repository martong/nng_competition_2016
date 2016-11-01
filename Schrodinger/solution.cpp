#include <vector>
#include <numeric>
#include <cassert>

using V = std::vector<size_t>;
using TestFun = bool (*)(const std::vector<size_t>& BallsToTest);

/// [low, high]
V range(size_t low, size_t high) {
    assert(low <= high);
    V res(high - low + 1);
    std::iota(res.begin(), res.end(), low);
    return res;
}

V linear(size_t NumberOfBalls, size_t RadioactiveBalls, TestFun testFun) {
    V res;
    for (size_t i = 0; i < NumberOfBalls; ++i) {
        if (testFun(range(i, i))) {
            res.push_back(i);
        }
    }
    return res;
}

struct TestFunW {
    TestFun testFun;
    const V& res;
    TestFunW(TestFun testFun, const V& res) : testFun(testFun), res(res) {}
    bool operator()(V range) {
        // remove all already found radioactives from the range
        for (const auto& r : res) {
            range.erase(std::remove(range.begin(), range.end(), r),
                        range.end());
        }
        return testFun(range);
    }
};

void RlogN_impl(size_t low, size_t high, V& res, TestFunW& testFunW) {
    if (low == high) {
        //if (testFunW(range(low, low))) {
            res.push_back(low);
        //}
        return;
    }
    if (testFunW(range(low, (low + high) / 2))) {
        RlogN_impl(low, (low + high) / 2, res, testFunW);
    } else {
        RlogN_impl((low + high) / 2 + 1, high, res, testFunW);
    }
}

V RlogN(size_t NumberOfBalls, size_t RadioactiveBalls, TestFun testFun) {
    V res;
    TestFunW testFunW{testFun, res};
    assert(NumberOfBalls >= 1);
    for (size_t i = 0; i < RadioactiveBalls; ++i) {
        // find one r ball
        RlogN_impl(0, NumberOfBalls - 1, res, testFunW);
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

    // return linear(NumberOfBalls, RadioactiveBalls, TestFunction);
    return RlogN(NumberOfBalls, RadioactiveBalls, TestFunction);
}

V impl(size_t low, size_t high, size_t& num_ractive, TestFun testFun) {
    if (num_ractive == 0) {
        return {};
    }
    if (low == high) {
        --num_ractive;
        return {low};
    }

    V res;
    if (testFun(range(low, high / 2))) { // ractive
        V subres = impl(low, high / 2, num_ractive, testFun);
        res.insert(res.end(), subres.begin(), subres.end());
    }
    if (num_ractive && testFun(range(high / 2 + 1, high))) { // ractive
        V subres = impl(high / 2 + 1, high, num_ractive, testFun);
        res.insert(res.end(), subres.begin(), subres.end());
    }
    return res;
}

