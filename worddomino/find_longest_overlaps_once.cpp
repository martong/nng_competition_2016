#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

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
    std::size_t matchSize = 16;
    while (!remainingWords.empty() && matchSize > 0) {
        std::vector<std::size_t> wordsFound;
        for (const auto& wordIndex : remainingWords) {
            const std::string& word = words[wordIndex];
            assert(word.size() <= 16);
            if (word.size() < matchSize) {
                continue;
            }
            std::string wordFragment = word.substr(word.size() - matchSize);
            auto matchIt = std::lower_bound(words.begin(), words.end(),
                    wordFragment);
            while (matchIt != words.end() && matchIt->find(wordFragment) == 0) {
                std::size_t matchIndex = std::distance(words.begin(), matchIt);
                if (matchIndex != wordIndex &&
                        wordsAssigned.count(matchIndex) == 0) {
                    matches.emplace_back(wordIndex, matchIndex, matchSize);
                    wordsFound.push_back(wordIndex);
                    wordsAssigned.insert(matchIndex);
                    break;
                }
                ++matchIt;
            }
        }
        for (const auto& elem : wordsFound) {
            remainingWords.erase(elem);
        }
        --matchSize;
    }

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

    std::size_t chainLength = 0;
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
            matchIndex = *remainingMatches.begin();
            chain.push_back(matches[matchIndex].firstWord);
            remainingWords.erase(matches[matchIndex].firstWord);
            chainLength += words[matches[matchIndex].firstWord].size();
        }
        remainingMatches.erase(matchIndex);
        const auto& match = matches[matchIndex];
        if (remainingWords.count(match.secondWord) > 0) {
            chain.push_back(match.secondWord);
            chainLength += words[matches[matchIndex].firstWord].substr(match.matchSize).size();
        }
        remainingWords.erase(match.secondWord);
        matchIndex = indices[match.secondWord];
    }

    std::cerr << "remaining words: " << remainingWords.size() << std::endl;
    for (const auto& elem : remainingWords) {
        chain.push_back(elem);
        chainLength += words[elem].size();
    }
    std::cerr << "chan length: " << chainLength << std::endl;
    //chain.insert(chain.end(), remainingWords.begin(), remainingWords.end());
    return chain;
}

int main() {
    const std::vector<std::string> words = readWords(std::cin);
    std::vector<std::size_t> chain = generateChain(words);

    for (const auto& elem : chain) {
        std::cout << words[elem] << std::endl;
    }
}
