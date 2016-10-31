#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <assert.h>
#include <iostream>

using boost::multiprecision::mpz_int;

mpz_int nTimesSqrt2(const mpz_int& n) {
    mpz_int nSquare = n * n;
    mpz_int min = n;
    mpz_int max = n * 2;
    while (max - min > 1) {
        mpz_int mean = (min + max) / 2;
        if (mean * mean / nSquare < 2) {
            min = mean;
        } else {
            max = mean;
        }
    }
    return min;
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
    mpz_int minimumHeight = nTimesSqrt2(side);

    mpz_int minimumResult = (side + 1) * (minimumHeight + 1) / 2;
    mpz_int maximumResult = (side + 1) * (minimumHeight + 2) / 2;

    std::cout << minimumResult << "\n" << maximumResult << "\n" <<
            maximumResult - minimumResult << std::endl;
    return 0;
}
