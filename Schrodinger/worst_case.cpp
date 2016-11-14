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

V range(const V& indexes, size_t low, size_t high) {
    assert(low <= high);
    V res(high - low);
    std::copy(indexes.begin() + low, indexes.begin() + high, res.begin());
    return res;
}

std::vector<V> splitProblem(const V& indexes, std::size_t n,
        bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {
    std::vector<V> result;
    std::size_t bucketSize = indexes.size() / n + (
            indexes.size() % n == 0 ? 0 : 1);
    for (std::size_t i = 0; i < n; ++i) {
        V balls = range(indexes, i * bucketSize,
                std::min(indexes.size(), (i + 1) * bucketSize));
        if (TestFunction(balls)) {
            result.push_back(balls);
        }
    }
    return result;
}

std::size_t binarySearch(const V& bucket,
        bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {
    assert(!bucket.empty());
    if (bucket.size() == 1) {
        return bucket.front();
    }
    if (TestFunction(range(bucket, 0, bucket.size() / 2))) {
        return binarySearch(range(bucket, 0, bucket.size() / 2), TestFunction);
    } else {
        return binarySearch(range(bucket, bucket.size() / 2, bucket.size()),
                TestFunction);
    }
}

std::vector<size_t> evilSolve(
    size_t NumberOfBalls, size_t RadioactiveBalls,
    bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {
    V indexes(NumberOfBalls);
    std::iota(indexes.begin(), indexes.end(), 0);
    auto buckets = splitProblem(indexes, RadioactiveBalls, TestFunction);
    assert(buckets.size() <= RadioactiveBalls);
    V result;
    while(buckets.size() != RadioactiveBalls) {
        const auto& bucket = buckets.front();
        if (bucket.size() == 1) {
            result.push_back(bucket.front());
            buckets.erase(buckets.begin());
            --RadioactiveBalls;
            continue;
        }
        auto newBuckets = splitProblem(bucket,
                std::min(RadioactiveBalls - buckets.size() + 1,
                        buckets.front().size()), TestFunction);
        buckets.erase(buckets.begin());
        std::copy(newBuckets.begin(), newBuckets.end(),
                std::back_inserter(buckets));
    }

    for (const auto& bucket : buckets) {
        result.push_back(binarySearch(bucket, TestFunction));
    }
    return result;
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

    return evilSolve(NumberOfBalls, RadioactiveBalls, TestFunction);
}
