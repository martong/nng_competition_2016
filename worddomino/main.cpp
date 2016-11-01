#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
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

    std::size_t firstWord;
    std::size_t secondWord;
    int matchSize;
};

int main() {
    std::map<int, std::vector<Match>> matches;
    const std::vector<std::string> words = readWords(std::cin);
    std::unordered_set<std::string> wordsWithoutMatch(words.begin(),
            words.end());
    std::unordered_set<std::string> wordsUsed;

    std::size_t wordLength = 64;
    while (!wordsWithoutMatch.empty() && wordLength > 0) {
        int wordCount = 0;
        std::unordered_set<std::string> wordsFound;
        for (const auto& word : wordsWithoutMatch) {
            if (++wordCount % 1000 == 0) {
                std::cerr << '.';
            }
            if (word.size() < wordLength) {
                continue; // first find only the biggest matches
            }
            std::string wordFragment = word.substr(word.size() - wordLength);
            auto matchIterator = std::lower_bound(words.begin(), words.end(),
                    wordFragment,
                    [wordLength](const std::string elem,
                            const std::string& value) {
                        return elem.substr(0, wordLength) < value;
                    });
            while (matchIterator != words.end() &&
                    0 == matchIterator->find(wordFragment)) {
                if (*matchIterator != word &&
                        wordsUsed.count(*matchIterator) == 0) {
                    matches[wordLength].emplace_back(
                            std::distance(words.begin(), std::find(words.begin(), words.end(), word)),
                            std::distance(words.begin(), matchIterator), wordLength);
                    wordsFound.insert(word);
                    wordsUsed.insert(*matchIterator);
                    break;
                }
                ++matchIterator;
            }
        }
        for (const auto& wordFound : wordsFound) {
            wordsWithoutMatch.erase(wordFound);
        }
        --wordLength;
    }
    std::cerr << std::endl;
    std::cout << "matches: " << std::endl;
    for (const auto& matchCategory : matches) {
        std::cout << "length: " << matchCategory.first << std::endl;
        for (const auto& match : matchCategory.second) {
            std::cout << words[match.firstWord] << ' ' << words[match.secondWord] << " size: "
                      << match.matchSize << std::endl;
        }
    }

    int i = 0;
    for (const auto& wordNoMatch : wordsWithoutMatch) {
        if (wordsUsed.count(wordNoMatch) > 0) {
            ++i;
        }
    }


    std::cout << "number of words: " << words.size() << std::endl;
    std::cout << "number of words used: " << wordsUsed.size()
              << " number of words used but without match: " << i << std::endl;
    std::cout << "number of words without match: " << wordsWithoutMatch.size()
              << std::endl;

    for (const auto& word : wordsWithoutMatch) {
        std::cout << word << std::endl;
    }
}

// There are no duplicated among the words
int main2() {
    std::map<int, std::vector<Match>> matches;
    const std::vector<std::string> words = readWords(std::cin);
    std::unordered_set<std::string> wordsWithoutMatch{words.begin(),
            words.end()};
    std::unordered_set<std::string> wordsUsed;

    for (std::size_t firstWord = 0; firstWord < words.size(); ++firstWord) {
        if (firstWord % 1000 == 0) {
            std::cerr << '.';
        }
        auto wordIterator = words.begin();
        std::advance(wordIterator, firstWord);
        const std::string& word = *wordIterator;
        for (std::size_t i = word.size(); i > 0; --i) {
            std::string wordFragment = word.substr(word.size() - i);
            auto predicate = [i](const std::string& elem,
                    const std::string& value) {
                return elem.substr(0, i) < value;
                //if (0 == elem.find(value)) {
                //    return false;
                //}
                //return elem < value;
            };
            auto matchIterator = std::lower_bound(words.begin(),
                    words.end(), wordFragment, predicate);
            //std::cerr << "match dist: "
            //          << std::distance(words.begin(), matchIterator)
            //          << " word: " << word
            //          << " wordfrag: " << wordFragment << std::endl;
            bool matchFound = false;
            while(matchIterator != words.end() &&
                    0 == matchIterator->find(wordFragment)) {
                if (*matchIterator != word && wordsUsed.count(*matchIterator) == 0) {
                    matches[i].emplace_back(firstWord,
                            std::distance(words.begin(), matchIterator), i);
                    wordsWithoutMatch.erase(word);
                    wordsUsed.insert(*matchIterator);
                    matchFound = true;
                    break;
                }
                ++matchIterator;
            }
            if (matchFound) {
                break;
            }
        }
    }
    std::cout << std::endl;

    std::cout << "matches: " << std::endl;
    for (const auto& matchCategory : matches) {
        std::cout << "length: " << matchCategory.first << std::endl;
        for (const auto& match : matchCategory.second) {
            std::cout << match.firstWord << ' ' << match.secondWord << " size: "
                      << match.matchSize << std::endl;
        }
    }

    int i = 0;
    for (const auto& wordNoMatch : wordsWithoutMatch) {
        if (wordsUsed.count(wordNoMatch) > 0) {
            ++i;
        }
    }

    std::cout << "number of words: " << words.size() << std::endl;
    std::cout << "number of words used: " << wordsUsed.size()
              << " number of words used but without match: " << i << std::endl;
    std::cout << "number of words without match: " << wordsWithoutMatch.size()
              << std::endl;

    for (const auto& word : wordsWithoutMatch) {
        std::cout << word << std::endl;
    }

}
