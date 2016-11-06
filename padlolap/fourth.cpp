#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <cassert>
#include <iostream>


int main (int argc, char* argv[]) {
    using namespace boost::multiprecision;

    assert(argc == 2);
    mpz_int input = boost::lexical_cast<mpz_int>(argv[1]);
    const mpz_int base = input; // since there is one 'tavtarto' at the wall

    mpz_int sum = 0;
    mpf_float::default_precision(1000);
    mpf_float_1000 one = 1.0, two = 2.0;
    mpf_float_1000 df = one / boost::multiprecision::sqrt(two);
    mpf_float_1000 a = base + 1;
    std::size_t iterations = 0;
    while (a > 0) {
        mpz_int ai = static_cast<mpz_int>(a);
        sum += ai;
        a -= df;
        ++iterations;
    }

    std::cout << "Second test solution: " << sum << std::endl;
    std::cout << "iterations: " << iterations << std::endl;

    const mpz_int z = base * base * 2;
    const mpz_int n = sqrt(z);
    //const mpz_int squareCorners = (4 + (base - 1) * 2) + ((n - 1) * (2 + (base - 1)));
    const mpz_int squareCorners = (base + n*base + n + 1);
    const mpz_int solution = static_cast<mpz_int>(squareCorners / 2);

    std::cout << "Solution using corner computation: " << solution
            << " square corners: " << squareCorners
              << std::endl;

    const mpf_float_1000 b = sqrt(static_cast<mpf_float_1000>(z));
    const mpf_float_1000 area = (base  + 1) * (b + 1);
    const mpz_int areaGuess = static_cast<mpz_int>(area / 2) + 1;

    std::cout << "Area: " << area
              << " guess: " << areaGuess << std::endl;
}
