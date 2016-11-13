#include <util/ThreadPool.hpp>

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <signal.h>

thread_local std::mt19937 rng{std::random_device{}()};

class ResultHandler {
public:
    explicit ResultHandler(const std::vector<std::string>& words)
            : words(words) {
    }

    void setNewScore(std::size_t length,
            const std::vector<std::size_t>& chain) {
        std::unique_lock<std::mutex> guard{mutex};
        if (length < bestScore) {
            bestScore = length;
            std::ofstream of{"output." +
                                boost::lexical_cast<std::string>(fileNumber++) +
                                ".txt",
                        std::ios::out | std::ios::trunc};
            for (const auto& elem : chain) {
                of << words[elem] << std::endl;
            }
            of.close();
        }
    }

private:
    const std::vector<std::string>& words;
    std::size_t bestScore = std::numeric_limits<std::size_t>::max();
    std::mutex mutex;
    std::size_t fileNumber = 0;
};

std::vector<std::string> readWords(std::istream& inputStream) {
    std::vector<std::string> words;
    std::string word;
    while (inputStream >> word) {
        words.push_back(word);
    }
    std::sort(words.begin(), words.end());
    return words;
}

struct Match {
    Match(std::size_t firstWord, std::size_t secondWord, int matchSize)
            : firstWord(firstWord),
              secondWord(secondWord),
              matchSize(matchSize) {
    }

    bool operator<(const Match& rhs) {
        return firstWord < rhs.firstWord;
    }

    std::size_t firstWord;
    std::size_t secondWord;
    int matchSize;
};

std::vector<std::size_t> generateChain(const std::vector<std::string>& words) {
    std::vector<Match> matches;
    std::unordered_set<std::size_t> remainingWords;
    for (std::size_t i = 0; i < words.size(); ++i) {
        remainingWords.insert(i);
    }
    std::unordered_set<std::size_t> wordsAssigned;

    //auto maxElem = std::max_element(words.begin(), words.end(),
    //        [](const std::string& a, const std::string& b) {
    //            return a.size() < b.size();
    //        });
    //std::size_t matchSize = maxElem->size();
    //std::cerr << "hardcode match size!!: " << matchSize << std::endl;
    std::vector<std::size_t> matchSizes{16, 15, 14, 13, 12};
    std::random_shuffle(matchSizes.begin(), matchSizes.end());
    for (std::size_t i = 11; i > 0; --i) {
        matchSizes.push_back(i);
    }
    std::size_t sizeIndex = 0;
    while (!remainingWords.empty() && sizeIndex < matchSizes.size()) {
        std::size_t matchSize = matchSizes[sizeIndex];
        std::vector<std::size_t> wordsFound;
        std::vector<std::size_t> shuffledRemainingWords{remainingWords.begin(),
                    remainingWords.end()};
        std::random_shuffle(shuffledRemainingWords.begin(),
                shuffledRemainingWords.end());
        for (const auto& wordIndex : shuffledRemainingWords) {
            const std::string& word = words[wordIndex];
            assert(word.size() <= 16);
            if (word.size() < matchSize) {
                continue;
            }
            std::string wordFragment = word.substr(word.size() - matchSize);
            auto matchIt = std::lower_bound(words.begin(), words.end(),
                    wordFragment);
            std::vector<std::size_t> possibleMatches;
            while (matchIt != words.end() && matchIt->find(wordFragment) == 0) {
                std::size_t matchIndex = std::distance(words.begin(), matchIt);
                if (matchIndex != wordIndex &&
                        wordsAssigned.count(matchIndex) == 0) {
                    possibleMatches.push_back(matchIndex);
                }
                ++matchIt;
            }
            if (!possibleMatches.empty()) {
                std::size_t chosenMatchIndex = possibleMatches[
                        std::uniform_int_distribution<std::size_t>{
                                0, possibleMatches.size() - 1}(rng)];
                wordsFound.push_back(wordIndex);
                wordsAssigned.insert(chosenMatchIndex);
                matches.emplace_back(wordIndex, chosenMatchIndex, matchSize);
            }
        }
        for (const auto& elem : wordsFound) {
            remainingWords.erase(elem);
        }
        ++sizeIndex;
    }

    std::unordered_set<std::size_t> startingWords;
    for (std::size_t i = 0; i < words.size(); ++i) {
        if (wordsAssigned.count(i) == 0 && remainingWords.count(i) == 0) {
            startingWords.insert(i);
            //std::cerr << "Possible starting word: " << i << std::endl;
        }
    }
    //std::cerr << startingWords.size() << std::endl;

    //int i = 0;
    //for (const auto& elem : remainingWords) {
    //    i += wordsAssigned.count(elem);
    //}
    //std::cerr << "remaining words: " << remainingWords.size() << ' '
    //          << " i: " << i <<  " mathces: " << matches.size() <<std::endl;

    //{
    //    std::unordered_set<std::size_t> firstWords;
    //    std::unordered_set<std::size_t> secondWords;
    //    for (const auto& m : matches) {
    //        firstWords.insert(m.firstWord);
    //        secondWords.insert(m.secondWord);
    //    }
    //    assert(firstWords.size() == matches.size());
    //    assert(secondWords.size() == matches.size());
    //}

    std::unordered_set<std::size_t> remainingMatches;
    const std::size_t notFound = matches.size() + 4;
    std::vector<std::size_t> indices(words.size(), notFound);
    for (std::size_t i = 0; i < matches.size(); ++i) {
        indices[matches[i].firstWord] = i;
        remainingMatches.insert(i);
    }
    for (std::size_t i = 0; i < words.size(); ++i) {
        remainingWords.insert(i);
    }
    std::vector<std::size_t> chain;
    std::size_t matchIndex = notFound; //*remainingMatches.begin();
    while (!remainingMatches.empty()) {
        if (matchIndex == notFound || remainingMatches.count(matchIndex) == 0) {
            if (!startingWords.empty()) {
                std::size_t chosenIndex =
                        std::uniform_int_distribution<std::size_t>{0,
                                startingWords.size() - 1}(rng);
                auto mit = startingWords.begin();
                std::advance(mit, chosenIndex);
                matchIndex = indices[*mit];
                startingWords.erase(*mit);
            } else {
                std::size_t chosenIndex =
                        std::uniform_int_distribution<std::size_t>{0,
                                remainingMatches.size() - 1}(rng);
                auto mit = remainingMatches.begin();
                std::advance(mit, chosenIndex);
                matchIndex = *mit;
            }
            chain.push_back(matches[matchIndex].firstWord);
            remainingWords.erase(matches[matchIndex].firstWord);
        }
        remainingMatches.erase(matchIndex);
        const auto& match = matches[matchIndex];
        if (remainingWords.count(match.secondWord) > 0) {
            chain.push_back(match.secondWord);
        }
        remainingWords.erase(match.secondWord);
        matchIndex = indices[match.secondWord];
    }

    //std::cerr << "remaining words: " << remainingWords.size() << std::endl;
    for (const auto& elem : remainingWords) {
        chain.push_back(elem);
    }
    //chain.insert(chain.end(), remainingWords.begin(), remainingWords.end());
    return chain;
}

std::size_t findRemainingLen(const std::string& prev, const std::string& next) {
    for (std::size_t i = 0; i < prev.size(); ++i) {
        if (next.substr(0, prev.size() - i) == prev.substr(i)) {
            return next.size() - (prev.size() - i);
        }
    }
    return next.size();
}

void calculate(const std::vector<std::string>& words,
        ResultHandler& resultHandler) {
    std::vector<std::size_t> chain = generateChain(words);

    std::string previousWord;
    std::size_t length = 0;
    for (const auto& elem : chain) {
        const std::string& word = words[elem];
        length += findRemainingLen(previousWord, word);
        previousWord = word;
    }
    resultHandler.setNewScore(length, chain);
    std::cerr << "length: " << length << std::endl;
}

int main(int argc, char* argv[]) {
    bool runOnce = false;
    if (argc == 2 && std::string{argv[1]} == "-1") {
        runOnce = true;
    }

    const std::vector<std::string> words = readWords(std::cin);
    ResultHandler resultHandler{words};

    if (runOnce) {
        calculate(words, resultHandler);
        return 0;
    }

    const std::size_t THREAD_COUNT = 4;
    util::ThreadPool threadPool{THREAD_COUNT};
    auto& ioService = threadPool.getIoService();

    threadPool.start();
    for (std::size_t i = 0; i < THREAD_COUNT; ++i) {
        ioService.post(
                [&words, &resultHandler]() {
                    while (true) {
                        calculate(words, resultHandler);
                    }
                });
    }
    threadPool.wait();
}
