#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <cassert>
#include <iostream>


int main (int argc, char* argv[]) {
    using namespace boost::multiprecision;

    assert(argc == 2);
    mpz_int input = boost::lexical_cast<mpz_int>(argv[1]);
    const mpz_int base = input; // since there is one 'tavtarto' at the wall
    // the number to find the square root for
    const mpz_int z = base * base * 2;

    mpz_int remainder = 0;
    const mpz_int n = sqrt(z, remainder);

    std::cout << "the square number is: " << z << std::endl;
    std::cout << "other side is " << n << " remainder: " << remainder
              << std::endl;

    mpz_int checkNumber1 = n * n + remainder;
    assert(checkNumber1 == z);
    mpz_int checkNumber2 = (n + 1) * (n + 1);
    assert(checkNumber2 > z);

    const mpz_int a_1 = base + 1; // first element of the series
    const mpz_int a_n = 1; // last element of the series

    const mpz_int s_n = ((a_1 + a_n) * n) / 2;

    std::cout << "The number of 'tavtarto' required: " << s_n
              << std::endl;

    std::cout << "Test if solution is good" << std::endl;
    mpf_float::default_precision(1000000);
    mpf_float_1000 one = 1.0, two = 2.0;
    mpf_float_1000 zf = base * base * two;
    mpf_float_1000 nf = sqrt(zf);
    mpf_float_1000 solutionf = ((static_cast<mpf_float_1000>(base + 1) + one) * nf) / 2;

    std::cout.precision(std::numeric_limits<mpf_float_1000>::max_digits10);  // Ensure all potentially significant bits are output.
    //std::cout.flags(std::ios_base::fmtflags(std::ios_base::scientific)); // Use scientific format.
    std::cout << solutionf << std::endl;

    mpf_float::default_precision(100000);
    mpf_float_1000 df = one / boost::multiprecision::sqrt(two);
    mpf_float_1000 a = base + 1;

    //assert(1 == (a - (static_cast<mpz_int>(nf) - 1) * df));
    //std::cout << (a - (static_cast<mpz_int>(nf) - 1) * df) << std::endl;

    mpz_int sum = 0;
    std::size_t iterations = 0;
    while (a > 0) {
        mpz_int ai = static_cast<mpz_int>(a);
        sum += ai;
        a -= df;
        ++iterations;
    }
    std::cout << "Second test solution: " << sum << std::endl;

    std::cout << "iterations: " << iterations << " n: " << n
              << " nf: " << nf << std::endl;

    a = base + 1;
    mpz_int n_i = static_cast<mpz_int>(a / df) + 1;
    mpz_int s_ni = static_cast<mpz_int>(((a) * n_i) / 2);

    std::cout << "Thid test solution: " << s_ni << std::endl;
    std::cout << "number of elements in the progression: " << n_i << std::endl;

    mpz_int a_ni = static_cast<mpz_int>(a_1 - df*n_i);
    std::cout << a_ni << std::endl;

    mpz_int snii = static_cast<mpz_int>(n_i/2*((n_i - 1) * df));
    std::cout << snii << std::endl;

    mpq_rational zr = base * base * 2;
    //mpq_rational nr = sqrt(zr);
}
