#ifndef UTIL_DUMPERFUNCTIONS_HPP
#define UTIL_DUMPERFUNCTIONS_HPP

#include "Matrix.hpp"
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

template<class T>
void dumpMatrix(std::ostream& file, const Matrix<T> table,
        const std::string& title = "", int indent = 0) {
    std::string indentString(indent, ' ');
    if (!title.empty()) {
        file << indentString << title << std::endl;
    }
    Matrix<std::string> txts(table.width(), table.height());
    size_t maxlen = 0;
    for (Point  p: matrixRange(table)) {
        txts[p] = boost::lexical_cast<std::string>(table[p]);
        maxlen = std::max(maxlen, txts[p].size());
    }
    // leave a space between characters
    ++maxlen;
    Point p;
    for (p.y = 0; p.y < static_cast<int>(table.height()); p.y++) {
        file << indentString;
        for (p.x = 0; p.x < static_cast<int>(table.width()); p.x++) {
            file.width(maxlen);
            file << txts[p];
        }
        file << std::endl;
    }
    file << std::endl;
}


#endif // UTIL_DUMPERFUNCTIONS_HPP
