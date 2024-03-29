#include <util/ThreadPool.hpp>

#include <boost/optional.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <random>
#include <fstream>

thread_local std::mt19937 rng{std::random_device{}()};

std::vector<std::string> readWords(std::istream& inputStream) {
    std::vector<std::string> words;
    std::string word;
    while (inputStream >> word) {
        words.push_back(word);
    }
    std::sort(words.begin(), words.end());
    return words;
}

boost::optional<std::size_t> findBestMatch(std::size_t wordIndex,
        const std::vector<std::string>& words,
        const std::unordered_set<std::size_t>& unusedWords,
        std::size_t& matchLength) {
    const std::string& word = words[wordIndex];

    for (std::size_t i = 0; i < word.size(); ++i) {
        matchLength = word.size() - i;
        std::string wordFragment = word.substr(i);
        auto matchIterator = std::lower_bound(words.begin(),
                words.end(), wordFragment);
        std::vector<std::size_t> possibleMatches;
        while (matchIterator != words.end() &&
                0 == matchIterator->find(wordFragment)) {
            std::size_t index = std::distance(words.begin(), matchIterator);
            if (*matchIterator != word && unusedWords.count(index) > 0) {
                possibleMatches.push_back(index);
            }
            ++matchIterator;
        }
        if (!possibleMatches.empty()) {
            return possibleMatches[std::uniform_int_distribution<std::size_t>{
                    0, possibleMatches.size() - 1}(rng)];
        }
    }

    return boost::none;
}

bool verbose = false;

struct Result {
    std::vector<std::pair<std::size_t, std::size_t>> wordSequence;
    std::string output;
};

Result calculate(const std::vector<std::string>& words) {
    std::unordered_set<std::size_t> unusedWords;
    for (std::size_t i = 0; i < words.size(); ++i) {
        unusedWords.insert(i);
    }

    std::size_t wordIndex = std::uniform_int_distribution<std::size_t>{
            0, words.size() - 1}(rng);
    std::size_t matchLength = 0;
    Result result;
    std::size_t noMatch = 0;
    while (!unusedWords.empty()) {
        if (verbose) {
            std::cerr << words[wordIndex] << '(' << matchLength << ')' << " + ";
        }
        result.output += words[wordIndex].substr(matchLength);
        unusedWords.erase(wordIndex);
        result.wordSequence.push_back(std::make_pair(wordIndex, matchLength));
        auto nextWord = findBestMatch(wordIndex, words, unusedWords, matchLength);
        if (nextWord) {
            wordIndex = *nextWord;
        } else if (!unusedWords.empty()) {
            ++noMatch;
            std::vector<std::size_t> remainingWords;
            remainingWords.reserve(unusedWords.size());
            std::copy(unusedWords.begin(), unusedWords.end(),
                    std::back_inserter(remainingWords));
            wordIndex = remainingWords[std::uniform_int_distribution<
                    std::size_t>{0, remainingWords.size() - 1}(rng)];
            matchLength = 0; // just to be sure
        }
    }
    std::cerr << std::endl << "number of no matches: " << noMatch <<
            ". Length: " << result.output.size() << std::endl;

    assert(result.wordSequence.size() == words.size());
    std::vector<std::pair<std::size_t, std::size_t>> uniqueSequence;
    std::unique_copy(result.wordSequence.begin(), result.wordSequence.end(),
            std::back_inserter(uniqueSequence),
            [](std::pair<std::size_t, std::size_t> a,
                    std::pair<std::size_t, std::size_t> b) {
                return a.first == b.first;
            });
    assert(uniqueSequence.size() == result.wordSequence.size());

    std::size_t pos = 0;
    for (const auto& elem : result.wordSequence) {
        const std::string& word = words[elem.first];
        std::size_t wordSize = word.size();
        pos -= elem.second;
        assert(word == result.output.substr(pos, wordSize));
        pos += wordSize;
    }

    assert(100000 == words.size());
    assert(100000 == result.wordSequence.size());

    return result;
}

int main(int argc, char* argv[]) {
    bool runOne = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string{argv[i]} == "-v") {
            verbose = true;
        } else if (std::string{argv[i]} == "-1") {
            runOne = true;
        }
    }

    const std::vector<std::string> words = readWords(std::cin);
    if (runOne) {
        auto result = calculate(words);
        std::ofstream of{"output.txt",
                std::ios::out | std::ios::trunc};
        for (const auto& element : result.wordSequence) {
            of << words[element.first] << "\n";
        }
        return 0;
    }

    std::size_t bestSize = 0;
    util::ThreadPool threadPool{8};
    auto& ioService = threadPool.getIoService();
    while (true) {
        threadPool.start();
        std::mutex mutex;
        for (std::size_t i = 0; i < words.size(); ++i) {
            ioService.post(
                    [&bestSize, &words, &mutex]() {
                        auto result = calculate(words);
                        std::unique_lock<std::mutex> lock{mutex};
                        if (bestSize == 0 || bestSize > result.output.size()) {
                            bestSize = result.output.size();
                            std::ofstream of{"output.txt",
                                    std::ios::out | std::ios::trunc};
                            for (const auto& element : result.wordSequence) {
                                of << words[element.first] << "\n";
                            }
                        }
                    });
        }
        threadPool.wait();
    }
}
