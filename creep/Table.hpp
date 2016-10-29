#ifndef CREEP_TABLE_HPP
#define CREEP_TABLE_HPP

#include <Matrix.hpp>
#include <MatrixIO.hpp>

#include <istream>
#include <ostream>

enum class MapElement : char {
    Wall = '#', Floor = '.', Creep = 'C', Hatchery = 'H',
    TumorCooldown = 'T', TumorActive = 'A', TumorInactive = '*',
};

inline
std::istream& operator>>(std::istream& is, MapElement& mapElement) {
    char c;
    is.get(c);
    mapElement = static_cast<MapElement>(c);
    return is;
}

inline
std::ostream& operator<<(std::ostream& os, MapElement mapElement) {
    return os << static_cast<char>(mapElement);
}

using Table = Matrix<MapElement>;

#endif // CREEP_TABLE_HPP
