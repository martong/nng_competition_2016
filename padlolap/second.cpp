#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <assert.h>
#include <iostream>

using namespace boost::multiprecision;

// a + b*sqrt(2)
struct Number {
    Number(const mpq_rational& a, const mpq_rational& b) :
            a(a), b(b) {
    }

    Number(const Number&) = default;
    Number(Number&&) = default;
    Number& operator=(const Number&) = default;
    Number& operator=(Number&&) = default;

    Number& operator+=(const Number& x) {
        a += x.a;
        b += x.b;
        return *this;
    }

    Number& operator+=(const mpq_rational& x) {
        a += x;
        return *this;
    }

    Number& operator-=(const Number& x) {
        a -= x.a;
        b -= x.b;
        return *this;
    }

    Number& operator-=(const mpq_rational& x) {
        a -= x;
        return *this;
    }

    Number& operator*=(const mpq_rational& x) {
        a *= x;
        b *= x;
        return *this;
    }

    Number& operator/=(const mpq_rational& x) {
        a /= x;
        b /= x;
        return *this;
    }

    mpq_rational a;
    mpq_rational b;
};

template<typename T>
Number operator+(Number lhs, const T& rhs) {
    return lhs += rhs;
}

template<typename T>
Number operator-(Number lhs, const T& rhs) {
    return lhs -= rhs;
}

template<typename T>
Number operator*(Number lhs, const T& rhs) {
    return lhs *= rhs;
}

template<typename T>
Number operator/(Number lhs, const T& rhs) {
    return lhs /= rhs;
}

mpq_rational getLimit(const mpq_rational r) {
    return mpq_rational{1} / denominator(r);
}

template<typename T>
T getLimit(const T&) { return 1; }

template<typename T>
T nTimesSqrt2(const T& n) {
    T nSquare = n * n;
    T min = n;
    T max = n * 2;
    T limit = getLimit(n);
    while (max - min > limit) {
        T mean = (min + max) / 2;
        if (mean * mean / nSquare < 2) {
            min = mean;
        } else {
            max = mean;
        }
    }
    std::cerr << "[Limit: " << limit << " Error: " << max - min << "]";
    return min;
}

mpz_int approximate(const Number& number) {
    mpq_rational r = number.a + nTimesSqrt2(number.b);
    return numerator(r) / denominator(r);
}

std::ostream& operator<<(std::ostream& os, const Number& n) {
    os << n.a;
    if (n.b != 0) {
        os << (n.b>0?"+":"") << n.b << "*√2{" << approximate(n) << "}";
    }
    return os;
}

Number multiplyWithSqrt2(const Number& number) {
    return Number{2 * number.b, number.a};
}

mpz_int calculateExact(const Number& side) {
    mpz_int sideApprox = approximate(side);
    mpz_int result = sideApprox + 1;
    if (side.b == 0) {
        for (mpz_int i = 1; i <= sideApprox; ++i) {
            result += nTimesSqrt2(i);
        }
    } else {
        for (mpz_int i = 0; i <= sideApprox; ++i) {
            Number height = multiplyWithSqrt2(side - i);
            std::cerr << i << " -> " << height << "\n";
            result += approximate(height);
        }
    }

    return result;
}

mpz_int calculate(const Number& side, int depth = 0, int branch = 0) {
    for (int i = 0; i < depth; ++i) {
        std::cerr << " ";
    }
    if (side.a + side.b < 10) {
        std::cerr << branch << ": side=" << side << " --> calculating exact: ";
        mpz_int result = calculateExact(side);
        std::cerr << result << "\n";
        return result;
    }
    mpz_int x = approximate(side) / 2;
    mpz_int y = nTimesSqrt2(x);
    Number b = multiplyWithSqrt2(side) - y;
    Number a = multiplyWithSqrt2(b) / mpq_rational{2};
    std::cerr << branch << ": side=" << side << " x=" << x << " y=" << y << " b=" << b << " a=" << a << "\n";
    return x * y + calculate(side - x, depth + 1, 1) + calculate(a, depth + 1, 2);
}

mpz_int tenPower(mpz_int n) {
    mpz_int stopAt = n / 2;
    mpz_int result = 10;
    mpz_int p = 1;
    while (p < stopAt) {
        result *= result;
        p *= 2;
    }
    for (; p < n; ++p) {
        result *= 10;
    }
    return result;
}

int main(int argc, const char* argv[]) {
    assert(argc == 2);

    mpz_int sideLog = boost::lexical_cast<mpz_int>(argv[1]);
    mpz_int side = tenPower(sideLog);
    std::cout << calculate(Number{side, 0}) << std::endl;
    return 0;
}
