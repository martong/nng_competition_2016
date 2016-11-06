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

std::size_t findBestMatch(std::size_t wordIndex,
        const std::vector<std::string>& words,
        const std::unordered_set<std::size_t>& unusedWords,
        std::size_t& matchLength) {
    const std::string& word = words[wordIndex];

    for (std::size_t i = 0; i < word.size(); ++i) {
        matchLength = word.size() - i;
        std::string wordFragment = word.substr(i);
        auto matchIterator = std::lower_bound(words.begin(),
                words.end(), wordFragment);
        while (matchIterator != words.end() &&
                0 == matchIterator->find(wordFragment)) {
            std::size_t index = std::distance(words.begin(), matchIterator);
            if (*matchIterator != word && unusedWords.count(index) > 0) {
                return index;
            }
            ++matchIterator;
        }
    }

    return 0; // 0 is the first element we used, so it means a no match here
}

int main(int argc, char* argv[]) {

    bool verbose = false;
    if (argc == 2 && std::string{argv[1]} == "-v") {
        verbose = true;
    }

    const std::vector<std::string> words = readWords(std::cin);
    std::unordered_set<std::size_t> unusedWords;
    for (std::size_t i = 0; i < words.size(); ++i) {
        unusedWords.insert(i);
    }

    std::size_t wordIndex = 0;
    std::size_t matchLength = 0;
    std::string output;
    while (!unusedWords.empty()) {
        if (!verbose) {
            std::cout << words[wordIndex].substr(matchLength);
        } else {
            output += words[wordIndex].substr(matchLength);
            std::cerr << words[wordIndex] << '(' << matchLength << ')' << " + ";
        }
        unusedWords.erase(wordIndex);
        wordIndex = findBestMatch(wordIndex, words, unusedWords, matchLength);
        if (wordIndex == 0 && !unusedWords.empty()) {
            wordIndex = *unusedWords.begin();
            matchLength = 0; // just to be sure
        }
    }
    std::cerr << std::endl;
    std::cout << output << std::endl;
}
