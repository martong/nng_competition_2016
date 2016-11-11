#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

std::vector<std::string> readWords(std::string fileName) {
    std::ifstream inputFile(fileName);
    std::vector<std::string> words;
    std::string word;
    while (inputFile >> word) {
        words.push_back(word);
    }
    inputFile.close();
    return words;
}

std::size_t findRemainingLen(const std::string& prev, const std::string& next) {
    for (std::size_t i = 0; i < prev.size(); ++i) {
        if (next.substr(0, prev.size() - i) == prev.substr(i)) {
            return next.size() - (prev.size() - i);
        }
    }
    return next.size();
}

int main(int argc, char* argv[]) {
    assert(argc == 2);
    char* chainFile = argv[1];

    const std::vector<std::string> inputWords =
            readWords("words_final.txt");
    std::unordered_set<std::string> inputSet{inputWords.begin(),
                inputWords.end()};
    const std::vector<std::string> chainWords = readWords(chainFile);
    std::unordered_set<std::string> chainSet{chainWords.begin(),
                chainWords.end()};

    std::size_t length = 0;
    std::string previousWord;
    for (const auto& elem : chainWords) {
        std::size_t length += findRemainingLen(previousWord, elem);
        previousWord = elem;
    }
    std::cout << "lenth of word chain: " << length << std::endl;

    for (const auto& elem : inputSet) {
        assert(1 == chainSet.erase(elem));
    }
    assert(chainSet.empty());
}
