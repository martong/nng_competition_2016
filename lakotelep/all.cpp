#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <algorithm>
#include <unordered_map>
#include <cassert>

// Direction.hpp
#include <iosfwd>

enum class Direction {
    left,
    right,
    up,
    down
};

std::ostream& operator<<(std::ostream& out, const Direction& direction);

// Hash.hpp
#include <functional>

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}


// Point.hpp
#include <ostream>
#include <stddef.h>

class Point {
public:
    int x, y;
    Point():x(0), y(0) { }
    constexpr Point(int x, int y):x(x), y(y) {}

    constexpr Point(const Point&) = default;
    Point& operator=(const Point&) = default;

    Point& operator+=(Point other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Point& operator-=(Point other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Point& operator*=(int n)
    {
        x *= n;
        y *= n;
        return *this;
    }
};

template<typename Archive>
void serialize(Archive& ar, Point& p, const unsigned int /*version*/) {
    ar & p.x;
    ar & p.y;
}

constexpr Point p00 {0,0}, p10{1, 0}, p01{0, 1}, p11{1, 1};

inline constexpr bool operator==(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

inline constexpr bool operator!=(Point p1, Point p2) {
    return !(p1 == p2);
}

inline constexpr Point operator+(Point p1, Point p2) {
    return Point(p1.x+p2.x, p1.y+p2.y);
}

inline constexpr Point operator-(Point p1, Point p2) {
    return Point(p1.x-p2.x, p1.y-p2.y);
}

inline constexpr Point operator*(Point p, int n) {
    return Point(p.x*n, p.y*n);
}

inline constexpr Point operator-(Point p) {
    return p * -1;
}

inline constexpr bool operator<(Point p1, Point p2) {
    return p1.y < p2.y || (p1.y == p2.y && p1.x < p2.x);
}

inline int distance(Point p1, Point p2) {
    auto d = p1 - p2;
    return abs(d.x) + abs(d.y);
}

inline
int distanceSquare(Point p1, Point p2) {
    Point difference = p1 - p2;
    return difference.x * difference.x + difference.y * difference.y;
}

std::ostream& operator<<(std::ostream& os, Point p);
Direction toDirection(const Point& source, const Point& destination);
std::ostream& operator<<(std::ostream& os, Point p) {
    os << '(' << p.x << ", " << p.y << ')';
    return os;
}

Direction toDirection(const Point& source, const Point& destination) {
    Point diff = destination - source;
    if (diff == p10) {
        return Direction::right;
    } else if (diff == p01) {
        return Direction::down;
    } else if (diff == p11) {
        assert(false);
    } else if (diff*-1 == p10) {
        return Direction::left;
    } else if (diff*-1 == p01) {
        return Direction::up;
    } else if (diff*-1 == p11) {
        assert(false);
    } else {
        assert(false);
    }
    assert(false);
}

namespace std {

template<>
struct hash<Point> {
    size_t operator()(Point p) const
    {
        size_t seed = 0;
        hash_combine(seed, p.x);
        hash_combine(seed, p.y);
        return seed;
    }
};

} // namespace std

// PointRange.hpp
#include <boost/iterator/iterator_facade.hpp>
#include <boost/exception/all.hpp>

#include <stdexcept>

class PointRangeIterator;

class PointRange {
public:
    typedef PointRangeIterator iterator;
private:
    Point begin_;
    Point end_;
public:
    typedef Point value_type;
    friend class PointRangeIterator;

    PointRange(Point  begin, Point  end):
        begin_(begin),
        end_(end) {
    }

    iterator begin() const;
    iterator end() const;
    Point  front() const
    {
        if (begin_ == end_) {
            BOOST_THROW_EXCEPTION(std::out_of_range("front() cannot be called on empty PointRange."));
        }
        return begin_;
    }
    Point back() const
    {
        if (begin_ == end_) {
            BOOST_THROW_EXCEPTION(std::out_of_range("back() cannot be called on empty PointRange."));
        }
        return Point(end_.x - 1, end_.y - 1);
    }
}; // class PointRange

class PointRangeIterator: public boost::iterator_facade<
        PointRangeIterator,
        Point,
        boost::bidirectional_traversal_tag,
        Point > {
public:
    PointRangeIterator(const PointRangeIterator&) = default;
    PointRangeIterator(PointRangeIterator&&) = default;

    PointRangeIterator& operator=(const PointRangeIterator&) = default;
    PointRangeIterator& operator=(PointRangeIterator&&) = default;

private:
    const PointRange* owner_;
    Point p_;

    friend class PointRange;
    friend class boost::iterator_core_access;

    PointRangeIterator(const PointRange& owner, Point  p):
        owner_(&owner),
        p_(p)
    {}
    Point  dereference() const { return p_; }
    void increment()
    {
        ++p_.x;
        if (p_.x == owner_->end_.x) {
            p_.x = owner_->begin_.x;
            ++p_.y;
        }
    }
    void decrement()
    {
        if (p_.x == owner_->begin_.x) {
            p_.x = owner_->end_.x - 1;
            --p_.y;
        } else {
            --p_.x;
        }
    }

    bool equal(const PointRangeIterator& other) const
    {
        return p_ == other.p_;
    }
}; // class PointRangeIterator

inline
PointRange::iterator PointRange::begin() const
{
    return iterator(*this, begin_);
}

inline
PointRange::iterator PointRange::end() const
{
    return iterator(*this, Point(begin_.x, end_.y));
}

// Matrix.hpp
#include <boost/range/algorithm.hpp>
#include <boost/serialization/vector.hpp>

#include <assert.h>
#include <type_traits>
#include <vector>

template <typename Matrix>
inline bool isInsideMatrix(const Matrix& matrix, Point p) {
    return p.x >= 0 && p.y >= 0 && p.x < static_cast<int>(matrix.width()) &&
            p.y < static_cast<int>(matrix.height());
}

template<typename T>
class Matrix {
    typedef std::vector<T> Data;
    std::size_t width_, height_;
    Data data_;
public:
    typedef T valueType;
    typedef typename Data::reference reference;
    typedef typename Data::const_reference const_reference;
    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;

    Matrix(): width_(0), height_(0) {}
    Matrix(std::size_t width, std::size_t height, const T& defValue = T()):
        width_(width), height_(height), data_(width * height, defValue)
    {}
    Matrix(const Matrix& ) = default;
    Matrix(Matrix&& ) = default;
    Matrix& operator=(const Matrix& ) = default;
    Matrix& operator=(Matrix&& ) = default;

    reference operator[](std::size_t pos) {
        return data_[pos];
    }
    const_reference operator[](std::size_t pos) const {
        assert(pos >= 0 && pos < size());
        return data_[pos];
    }
    reference operator[](Point p) {
        assert(isInsideMatrix(*this, p));
        return data_[p.y*width_ + p.x];
    }
    const_reference operator[](Point p) const {
        assert(isInsideMatrix(*this, p));
        return data_[p.y*width_ + p.x];
    }
    std::size_t size() const { return data_.size(); }
    std::size_t width() const { return width_; }
    std::size_t height() const { return height_; }
    void reset(std::size_t newWidth, std::size_t newHeight,
            const T& defValue = T())
    {
        width_ = newWidth;
        height_ = newHeight;
        data_.resize(width_ * height_);
        fill(defValue);
    }
    void fill(const T& value)
    {
        boost::fill(data_, value);
    }
    void clear()
    {
        data_.clear();
        width_ = 0;
        height_ = 0;
    }

    bool operator==(const Matrix<T>& other) const
    {
        return data_ == other.data_;
    }

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator begin() const { return data_.begin(); }
    const_iterator end() const { return data_.end(); }
    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int /*version*/) {
        ar & width_;
        ar & height_;
        ar & data_;
    }
};

template<typename T>
inline const typename Matrix<T>::const_reference matrixAt(const Matrix<T> &arr,
        Point p, const T& def) {
    return isInsideMatrix(arr, p) ? arr[p] : def;
}

template <typename Matrix>
inline PointRange matrixRange(const Matrix& matrix) {
    return PointRange(Point(0,0), Point(matrix.width(), matrix.height()));
}

template<typename T>
inline bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    return !(lhs == rhs);
}

namespace std {
template<typename T>
struct hash<Matrix<T>> {
    size_t operator()(const Matrix<T>& arr) const {
        size_t seed = 0;
        for (size_t i = 0; i < arr.size(); ++i) {
            hash_combine(seed, arr[i]);
        }
        return seed;
    }
};

template <>
struct hash<std::vector<Point>> {
    size_t operator()(const std::vector<Point>& arr) const {
        size_t seed = 0;
        for (size_t i = 0; i < arr.size(); ++i) {
            hash_combine(seed, arr[i]);
        }
        return seed;
    }
};

}

#include <iostream>
#include <string>

struct ToString {
    template<typename T>
    std::string operator()(T&& t) const {
        using std::to_string;
        return to_string(std::forward<T>(t));
    }

    std::string operator()(char c) const {
        return std::string{c};
    }
};

template<typename T, typename Converter = ToString>
void dumpMatrix(std::ostream& file, const Matrix<T> table,
        const std::string& title = "", int indent = 0,
        const Converter& converter = Converter{}) {
    std::string indentString(indent, ' ');
    if (!title.empty()) {
        file << indentString << title << std::endl;
    }
    Matrix<std::string> txts(table.width(), table.height());
    size_t maxlen = 0;
    for (Point  p: matrixRange(table)) {
        txts[p] = converter(table[p]);
        maxlen = std::max(maxlen, txts[p].size());
    }
    // leave a space between characters
    ++maxlen;
    Point p;
    for (p.y = 0; p.y < static_cast<int>(table.height()); p.y++) {
        file << indentString;
        for (p.x = 0; p.x < static_cast<int>(table.width()); p.x++) {
            if (p.x % 10 == 0 || p.y % 10 == 0) {
                file << "\e[0;31m";
            } else if (p.x % 5 == 0 || p.y % 5 == 0) {
                file << "\e[0;33m";
            }
            file.width(maxlen);
            file << txts[p];
            file.width(0);
            if (p.x % 5 == 0 || p.y % 5 == 0) {
                file << "\e[0m";
            }
        }
        file << std::endl;
    }
    file << std::endl;
}

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/max_element.hpp>

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace detail {

template<typename T>
void readValue(std::istream& s, T& t) {
    s >> t;
}

inline
void readValue(std::istream& s, char& c) {
    s.get(c);
}

}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
    dumpMatrix(os, matrix, " ");
    return os;
}

template<typename T>
Matrix<T> loadMatrix(std::istream& is, const T& defaultValue = T{},
        std::size_t width = 0, std::size_t height = 0, bool flip = false,
        char delimiter = '\n') {
    std::vector<std::string> lines;
    std::string line;
    while (is.good() && (height == 0 || lines.size() != height)) {
        std::getline(is, line, delimiter);
        std::cerr << "--> " << line << "\n";
        if (line.empty()) {
            break;
        }
        lines.push_back(line);
    }
    if (flip) {
        std::reverse(lines.begin(), lines.end());
    }

    height = lines.size();
    if (width == 0) {
        std::function<std::size_t(const std::string&)> sizeGetter =
                [](const std::string& s) { return s.size(); };
        width = height != 0 ?
                *boost::max_element(lines | boost::adaptors::transformed(
                        sizeGetter)) : 0;
    }
    std::cerr << width << ":" << height << "\n";
    Matrix<T> result{width, height, defaultValue};
    Point p;
    for (p.y = 0; p.y < static_cast<int>(height); ++p.y) {
        std::istringstream ss{lines[p.y]};
        for (p.x = 0; p.x < static_cast<int>(width) && ss.good(); ++p.x) {
            detail::readValue(ss, result[p]);
        }
    }

    return result;
}

template<typename T>
std::istream& operator>>(std::istream& is, Matrix<T>& matrix) {
    matrix = loadMatrix<T>(is);
    return is;
}

// Return all neigbors, those otuside the matrix too
std::array<Point, 4> getAllNeigbors(Point p) {
    return {{p + p01, p - p01, p + p10, p - p10}};
}

std::ostream& operator<<(std::ostream& os, const std::vector<Point>& ps) {
    for (const auto& p : ps) {
        os << p;
    }
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, std::stack<Point> ps) {
    while (!ps.empty()) {
        auto p = ps.top();
        ps.pop();
        os << p;
    }
    os << "\n";
    return os;
}

#include <memory>
#include <utility>

namespace util {

template <typename Function>
class Finally {
public:
    Finally(const Function& function):
        function(new Function(function))
    {}
    ~Finally()
    {
        if (function) {
            (*function)();
        }
    }

    // no copy is allowed
    Finally(const Finally&) = delete;
    Finally& operator=(const Finally&) = delete;

    Finally(Finally&& other):
        function(std::move(other.function))
    {}
    Finally& operator=(Finally&& other)
    {
        function = std::move(other.function);
        return *this;
    }
private:
    std::unique_ptr<Function> function;
};

template <typename Function>
Finally<Function> finally(const Function& function)
{
    return Finally<Function>(function);
}
} // util

// generator.hpp
#include <random>
#include <vector>

template<typename Iterator>
void doBuild(Matrix<int>& mx, Iterator begin, Iterator end) {
    for (;begin != end; ++begin) {
        auto p = *begin;
        mx[p] = 1;

        for (const auto n : getAllNeigbors(p)) {
            if (isInsideMatrix(mx, n)) {
                auto& v = mx[n];
                if (v >= 1) ++v;
                if (v > 4) v = 1;
            }
        }
    }
}

Matrix<int> build(int m, int n, const std::vector<Point>& ps) {
    Matrix<int> mx(m, n);
    doBuild(mx, ps.begin(), ps.end());
    return mx;
}

Matrix<int> generate(int m, int n) {
    //std::vector<std::vector<int>> result;

    std::vector<Point> ps;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            ps.emplace_back(i, j);
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(ps.begin(), ps.end(), g);

    return build(m, n, ps);
}

namespace pair {

Matrix<int> build(int m, int n, const std::vector<Point>& ps) {
    Matrix<int> mx(m, n);
    for (const auto p : ps) {
        mx[p] = 1;

        for (const auto n : getAllNeigbors(p)) {
            if (isInsideMatrix(mx, n)) {
                auto& v = mx[n];
                if (v >= 1) ++v;
            }
        }
    }
    return mx;
}

// <diag, original>
std::pair<Matrix<int>, Matrix<int>> generate(int m, int n) {
    //std::vector<std::vector<int>> result;

    std::vector<Point> ps;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            ps.emplace_back(i, j);
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(ps.begin(), ps.end(), g);

    auto first = pair::build(m, n, ps);
    auto second = first;
    for (int& i : second) {
        if (i > 4) i = 1;
    }
    return {first, second};
}

}

bool check(const std::vector<Point>& ps, const Matrix<int>& expected) {
    return build(expected.width(), expected.height(), ps) == expected;
}


//==============================================================================
//==============================================================================
// SOLUTION COMES FROM HERE

int floodErrors = 0;

bool flood(std::vector<Point> st, Matrix<int>& m,
                std::vector<Point>& path) {
    std::vector<Point> nst;
    while (!st.empty()) {
        auto p = st.back();
        st.pop_back();
        if (m[p] != 1 ) { continue; }

        m[p] = 0;
        path.push_back(p);
        auto ns = getAllNeigbors(p);
        for (const auto& n : ns) {
            if (isInsideMatrix(m, n)) {
                --m[n];
                if (m[n] == 0) {
                    m[n] = 4;
                }
                if (m[n] < 0) m[n] = 0;
                if (m[n] == 1) nst.push_back(n);
            }
        }

        // Cut offs
        for (const auto& n : ns) {
            if (!isInsideMatrix(m, n)) {
                continue;
            }

            auto nns = getAllNeigbors(n);
            std::size_t nsize = std::count_if(nns.begin(), nns.end(),
                    [&](Point p) { return isInsideMatrix(m, p); });
            //auto sum_nns = 0;
            //for (const auto& nn : nns) {
                //sum_nns += m[nn];
            //}
            //if (sum_nns < m[n] - 1) {
                //std::cout << "FLOOD ERROR\n";
                //return false;
            //}

            // elszigetelt 2-es vagy annal nagyobb
            if (m[n] >= 2 && nsize == 0) {
                ++floodErrors;
                return false;
            }
            // - 3-as vagy 4-es aminek 1 szomszedja van.
            if (m[n] >= 3 && nsize == 1) {
                ++floodErrors;
                return false;
            }
            // - 4-es aminek 2 szomszedja van.
            if (m[n] >= 4 && nsize == 2) {
                ++floodErrors;
                return false;
            }
        }

        if (!flood(nst, m, path)) { return false; }
    }
    return true;
}

std::vector<Point> concat(const std::vector<Point>& v1,
                          const std::vector<Point>& v2) {
    std::vector<Point> result;
    result.reserve(v1.size() + v2.size());
    result.insert(result.end(), v1.begin(), v1.end());
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
}

// not used, flood first looks better
bool solve_exp_flood_last(std::vector<Point> S, Matrix<int> m,
                          std::vector<Point> path, std::vector<Point>& result) {
    // std::cout << S;
    // std::cout << m;
    if (path.size() == m.size()) {
        result = path;
        return true;
    }
    if (S.empty()) return false;
    auto p = S.back();
    S.pop_back();

    if (!solve_exp_flood_last(S, m, path, result)) {
        std::vector<Point> floodpath;
        if (flood({p}, m, floodpath)) {
            return solve_exp_flood_last(S, m, concat(path, floodpath), result);
        }
    }
    return false;
}


void printDepth(int depth) {
    for (int i = 0; i < depth; ++i) {
        std::cerr << " ";
    }
}

int solve_exp_called = 0;



bool get_1s_inside_loop(const Matrix<int>& m, const Point p,
                        std::vector<Point> H, std::vector<Point>& result/*,
                        int depth*/) {
    H.push_back(p);
    auto sum0s = 0;
    for (auto x : getAllNeigbors(p)) {
        if (matrixAt(m, x, 0) == 0 ||
            std::find(H.begin(), H.end(), x) != H.end()) {
            ++sum0s;
        }
    }
    if (sum0s > 5 - m[p]) {
        return false;
    }
    if (sum0s < 5 - m[p]) {
        return true;
    }
    //printDepth(depth);
    //std::cerr << "Considering: " << p << " [" << m[p] << "]. " << "sum=" << sum0s << "\n";
    for (auto n : getAllNeigbors(p)) {
        if (!isInsideMatrix(m, n) || m[n] == 0 ||
                std::find(H.begin(), H.end(), n) != H.end()) {
            continue;
        }

        if (m[n] == 1) {
            //printDepth(depth);
            //std::cerr << "Adding: " << n << "\n";
            result.push_back(n);
        } else {
            if (!get_1s_inside_loop(m, n, H, result/*, depth + 1*/)) {
                return false;
            }
        }
    }
    return true;
};

int get1sErrors = 0;

boost::optional<std::vector<Point>> get_1s_inside(const Matrix<int>& m) {
    std::vector<Point> result;
    for (int i = 0; i < m.height(); ++i) {
        for (int j = 0; j < m.width(); ++j) {
            std::vector<Point> H;
            Point p{j, i};
            if (!get_1s_inside_loop(m, p, H, result/*, 0*/)) {
                ++get1sErrors;
                return boost::none;
            }
        }
    }
    return result;
}

int numFalsePaths = 0;

bool solve_exp_flood_first(std::vector<Point> S, Matrix<int>& m,
                           size_t size,
                           std::vector<Point> path,
                           std::vector<Point>& result) {

    auto flooood = [size](Matrix<int>&m, std::vector<Point>& path){
        while (size != path.size()) {

            auto st = get_1s_inside(m);
            if (!st) {
                return false;
            }
            // check_if_really_1(diag, st);

            if (st->size() == 0) {
                break;
            }

            if (!flood(*st, m, path)) {
                return false;
            }
        }
        return true;
    };

    auto allFlood = [flooood](std::vector<Point> st, Matrix<int>&m, std::vector<Point>& path) {
        return flood(st, m, path) && flooood(m, path);
    };

    if (path.size() == size) {
        result = path;
        return true;
    }
    if (S.empty()) return false;

    auto p = S.back();
    S.pop_back();


    std::vector<Point> floodpath;

    if (!allFlood({p}, m, floodpath)) {
        doBuild(m, floodpath.rbegin(), floodpath.rend());
        ++numFalsePaths;
        return solve_exp_flood_first(S, m, size, path, result);
    }
    if (!solve_exp_flood_first(S, m, size, concat(path, floodpath), result)) {
        doBuild(m, floodpath.rbegin(), floodpath.rend());
        ++numFalsePaths;
        return solve_exp_flood_first(S, m, size, path, result);
    }
    return true;
}

bool solve_exp_flood_first(std::vector<Point> S, Matrix<int> m, size_t size,
                           std::vector<Point>& path) {
    return solve_exp_flood_first(S, m, size, path, path);
}

void check_if_really_1 (const Matrix<int>& diag, const std::vector<Point>& st){
    for (const auto& p : st) {
        if (diag.size() && diag[p] == 5) {
            std::cout << "ERROR1vs5 at " << p << std::endl;
            assert(false);
        }
    }
}

void get_group_impl(const Matrix<int>& m, Point p, int value,
               Matrix<bool>& marked,
               std::vector<Point>& result) {
    marked[p] = true;
    result.push_back(p);
    for (auto n : getAllNeigbors(p)) {
        if(isInsideMatrix(m, n) && !marked[n] && m[n] == value) {
            get_group_impl(m, n, value, marked, result);
        }
    }
}

// Returns a group, the groups first two elements are neigbors.
std::vector<Point> get_group(const Matrix<int>& m, Point p, int value,
                             Matrix<bool>& marked) {
    // Matrix<bool> marked{m.width(), m.height()};
    std::vector<Point> result;
    get_group_impl(m, p, value, marked, result);
    return result;
}

// Returns a group, the groups first two elements are neigbors.
std::vector<Point> get_group(const Matrix<int>& m, Point p, int value) {
    Matrix<bool> marked{m.width(), m.height()};
    std::vector<Point> result;
    get_group_impl(m, p, value, marked, result);
    return result;
}

void get_island_impl(const Matrix<int>& m, Point p,
               Matrix<bool>& marked,
               Point& min, Point& max,
               std::vector<Point>& result) {
    marked[p] = true;
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    result.push_back(p);
    for (auto n : getAllNeigbors(p)) {
        if(isInsideMatrix(m, n) && !marked[n] && m[n] != 0) {
            get_island_impl(m, n, marked, min, max, result);
        }
    }
}
// Returns a group, the groups first two elements are neigbors.
std::vector<Point> get_island(const Matrix<int>& m, Point p,
                             Matrix<bool>& marked,
                             Point& min, Point& max
                             ) {
    // Matrix<bool> marked{m.width(), m.height()};
    std::vector<Point> result;
    get_island_impl(m, p, marked, min, max, result);
    return result;
}

struct IslandInfo {
    Matrix<int> m;
    size_t size;
};

std::vector<IslandInfo> gather_islands(const Matrix<int>& m) {
    std::vector<IslandInfo> result;
    Matrix<bool> inIsland(m.width(), m.height(), false);
    for (int i = 0; i < m.height(); ++i) {
        for (int j = 0; j < m.width(); ++j) {
            Point p{j, i};
            if (m[p] !=0 && !inIsland[p]) {
                Point min, max;
                auto island = get_island(m, p, inIsland, min, max);
                Matrix<int> islandMatrix{m.width(), m.height(), 0};
                for (auto p : island) {
                    islandMatrix[p] = m[p];
                }
                result.push_back({std::move(islandMatrix), island.size()});
            }
        }
    }
    return result;
}

struct RankedPoints {
    Point p;
    Point neighbor;
    int rank;
    RankedPoints(Point p, int rank) : p(p), neighbor({-1, -1}), rank(rank) {}
    RankedPoints(Point p, Point neighbor, int rank)
        : p(p), neighbor(neighbor), rank(rank) {}
};

std::vector<Point> to_vector(const std::vector<RankedPoints>& ps) {
    std::vector<Point> result;
    for (const auto x : ps) {
        result.push_back(x.p);
        if (x.neighbor != Point{-1, -1}) {
            result.push_back(x.neighbor);
        }
    }
    return result;
}

std::vector<RankedPoints> gather_groups(const Matrix<int>& m) {
    std::vector<RankedPoints> result;
    Matrix<bool> inGroup(m.width(), m.height(), false);
    for (int i = 0; i < m.height(); ++i) {
        for (int j = 0; j < m.width(); ++j) {
            Point p{j, i};
            if (m[p] == 1 && !inGroup[p]) {
                auto gr = get_group(m, p, 1, inGroup);
                if (gr.size() >= 2) {
                    result.emplace_back(gr[0], gr[1], gr.size()); //Add the nbor
                } else {
                    result.emplace_back(gr[0], gr.size());
                }
            }
        }
    }
    return result;
}

std::vector<Point> solve(Matrix<int> m, const Matrix<int> diag = Matrix<int>{}) {
    if (diag.size()) std::cout << "DIAG:\n" << diag;

    std::vector<Point> path;
    std::vector<Point> result;

    auto flood_ones_from_edges = [&]() {
        std::vector<Point> st;
        // gather 1s at the edges
        for (int i = 0; i < m.width(); ++i) {
            {
                Point p0{i, 0};
                if (m[p0] == 1) st.push_back(p0);
            }
            {
                Point pN(i, m.height() - 1);
                if (m[pN] == 1) st.push_back(pN);
            }
        }
        for (int j = 1; j < m.height() - 1; ++j) {
            {
                Point p0{0, j};
                if (m[p0] == 1) st.push_back(p0);
            }
            {
                Point pN(m.width() - 1, j);
                if (m[pN] == 1) st.push_back(pN);
            }
        }
        flood(st, m, path);
    };
    flood_ones_from_edges();

    while (m.size() != path.size()) {

        auto st = get_1s_inside(m);
        // check_if_really_1(diag, st);
        assert(st);

        if (st->size() == 0) {
            //std::cerr << "FLOOD STOPPED:\n";
            //std::cerr << m;
            break;
        }

        flood(*st, m, path);
    }

    if (m.size() != path.size()) {

        auto islands = gather_islands(m);

        for (const auto& island : islands) {
            //std::cerr << "Island:\n" << island.m;
            auto groups = gather_groups(island.m);
            std::sort(groups.begin(), groups.end(),
                      [](const auto& l, const auto& r) { return l.rank < r.rank; });
            auto st = to_vector(groups);
            std::vector<Point> island_path;
            auto res = solve_exp_flood_first(st, island.m, island.size, island_path);
            path = concat(path, island_path);
            //std::cerr << "solution:\n" << island_path;
        }

        //std::cerr << "BEFORE SEARCH\n";
        //std::cerr << st;
        //std::cerr << m;
        //std::cerr << "solve returns: " << res << "\n";
    }

    std::cerr << "False paths: " << numFalsePaths << "\n";
    std::cerr << "Flood errors: " << floodErrors << "\n";
    std::cerr << "Flooood errors: " << get1sErrors << "\n";
    std::reverse(path.begin(), path.end());
    return path;
}

void CalculateBuildOrder(const std::vector<std::vector<int>>& buildings, std::vector<std::pair<size_t, size_t>>& solution) {
    Matrix<int> m(buildings.at(0).size(), buildings.size());
    for (int i = 0; i < buildings.size(); ++i){
        for (int j = 0; j < buildings[i].size(); ++j) {
            m[{j,i}] = buildings[i][j];
        }
    }
    auto result = solve(m);
    for (const auto& e : result) {
        solution.emplace_back(e.y, e.x);
    }
}
