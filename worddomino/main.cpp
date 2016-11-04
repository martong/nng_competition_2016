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

    bool operator<(const Match& rhs) {
        return firstWord < rhs.firstWord;
    }

    std::size_t firstWord;
    std::size_t secondWord;
    int matchSize;
};

std::vector<Match> matchWords(const std::vector<std::string>& words,
        bool verbose) {
    std::vector<Match> matches;
    std::unordered_set<std::string> wordsWithoutMatch(words.begin(),
            words.end());
    std::unordered_set<std::string> wordsAssigned;

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
                        wordsAssigned.count(*matchIterator) == 0) {
                    matches.emplace_back(
                            std::distance(words.begin(), std::lower_bound(
                                    words.begin(), words.end(), word)),
                            std::distance(words.begin(), matchIterator),
                            wordLength);
                    wordsFound.insert(word);
                    wordsAssigned.insert(*matchIterator);
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

    if (verbose) {
        std::cerr << "matches: " << std::endl;
        for (const auto& match : matches) {
            std::cerr << words[match.firstWord] << ' '
                      << words[match.secondWord] << " size: "
                      << match.matchSize << std::endl;
        }

        int i = 0;
        for (const auto& wordNoMatch : wordsWithoutMatch) {
            if (wordsAssigned.count(wordNoMatch) > 0) {
                ++i;
            }
        }

        std::cerr << "number of words: " << words.size() << std::endl;
        std::cerr << "number of words assigned: " << wordsAssigned.size()
                  << " number of words assigned but with no continuing word: "
                  << i << std::endl;
        std::cerr << "number of words without match: "
                  << wordsWithoutMatch.size() << std::endl;

        for (const auto& word : wordsWithoutMatch) {
            std::cerr << word << std::endl;
        }
    }

    //     for (const auto& w : wordsWithoutMatch) {
    //         matches.emplace_back(std::distance(words.begin(), std::lower_bound(
    //                 words.begin(), words.end(), w)), 0, 0);
    //     }

    return matches;
}

void validateMatches(const std::vector<std::string>& words,
        const std::vector<Match>& matches) {

    std::vector<bool> prefixWordsUsed(words.size(), false);
    std::vector<bool> suffixWordsUsed(words.size(), false);
    for (const auto& match : matches) {
        const std::string& firstWord = words[match.firstWord];
        const std::string& secondWord = words[match.secondWord];
        if (secondWord.substr(0, match.matchSize) !=
                firstWord.substr(firstWord.size() - match.matchSize)) {
            std::cerr << "validation failed for: " << firstWord
                      << ' ' << secondWord <<  "size: " << match.matchSize
                      << std::endl;
        }

        if (prefixWordsUsed[match.firstWord]) {
            std::cerr << "prefix word used twice! "
                      << firstWord << std::endl;
        }
        prefixWordsUsed[match.firstWord] = true;

        if (suffixWordsUsed[match.secondWord]) {
            std::cerr << "suffix word used tice! "
                      << secondWord << std::endl;
        }
        suffixWordsUsed[match.secondWord] = true;
    }
}

std::vector<Match> generateWordChain(const std::vector<Match>& matches,
        int numberOfWords) {
    std::vector<Match> wordChain;
    std::vector<int> indices(numberOfWords, -1);
    for (std::size_t i = 0; i < matches.size(); ++i) {
        indices[matches[i].firstWord] = i;
    }

    //std::sort(matches.begin(), matches.end());
    std::unordered_set<int> matchesUnused;
    for (std::size_t i = 0; i < matches.size(); ++i) {
        matchesUnused.insert(matches[i].firstWord);
    }


    int word = *matchesUnused.begin();
    while (!matchesUnused.empty()) {
        matchesUnused.erase(word);
        assert(indices[word] != -1);
        wordChain.push_back(matches[indices[word]]);
        word = matches[indices[word]].secondWord;
        if (matchesUnused.count(word) == 0 && !matchesUnused.empty()) {
            word = *matchesUnused.begin();
        }
    }

    return wordChain;
}

int main(int argc, char* argv[]) {

    bool verbose = false;
    if (argc == 2 && std::string{argv[1]} == "-v") {
        verbose = true;
    }

    const std::vector<std::string> words = readWords(std::cin);
    std::vector<Match> matches = matchWords(words, verbose);
    validateMatches(words, matches);
    std::vector<Match> wordChain = generateWordChain(matches, words.size());
    std::unordered_set<std::string> unusedWords{words.begin(), words.end()};
    for (const auto& match : matches) {
        unusedWords.erase(words[match.firstWord]);
        unusedWords.erase(words[match.secondWord]);
    }

    std::cerr << "Unused words: " << unusedWords.size() << std::endl;

    int noconts = 0;
    auto wordChainIterator = wordChain.begin();
    std::size_t previousWord = wordChainIterator->firstWord;
    std::cout << words[wordChainIterator->firstWord];
    for (; wordChainIterator != wordChain.end(); ++wordChainIterator) {
        if (previousWord != wordChainIterator->firstWord) {
            ++noconts;
            std::cout << words[wordChainIterator->firstWord];
        }
        previousWord = wordChainIterator->secondWord;
        std::cout << words[wordChainIterator->secondWord].substr(wordChainIterator->matchSize);
    }

    for (const auto& word : unusedWords) {
        std::cout << word;
    }
    std::cout << std::endl;

    std::cerr << "The number of individual chains: " << noconts << std::endl;
}
