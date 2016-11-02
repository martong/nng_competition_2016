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

mpz_int nTimesSqrt2(const mpz_int& n) {
    mpz_int nSquare = n * n;
    mpz_int min = n;
    mpz_int max = n * 2;
    while (abs(max - min) > 1) {
        mpz_int mean = (min + max) / 2;
        if (mean * mean / nSquare < 2) {
            min = mean;
        } else {
            max = mean;
        }
    }
    return std::min(min, max);
}

mpq_rational nTimesSqrt2(const mpq_rational& n) {
    mpz_int d = denominator(n);
    return nTimesSqrt2(mpz_int{numerator(n) * d}) / d / d;
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
    for (mpz_int i = 0; i <= sideApprox; ++i) {
        Number height = multiplyWithSqrt2(side - i);
        result += approximate(height);
    }

    return result;
}

void printSpaces(int depth) {
    for (int i = 0; i < depth; ++i) {
        std::cerr << " ";
    }
}

mpz_int calculate(const Number& side) {
    mpz_int x = approximate(side) / 2;
    if (x < 10) {
        mpz_int result = calculateExact(side);
        return result;
    }
    mpz_int y = nTimesSqrt2(x);
    Number aa = side - Number{0, y} / mpq_rational{2};
    return x * y + calculate(side - x) + calculate(aa) - calculate(aa - x);
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
