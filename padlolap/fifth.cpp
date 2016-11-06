#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <cassert>
#include <iostream>


int main (int argc, char* argv[]) {
    using namespace boost::multiprecision;

    assert(argc == 2);
    mpz_int input = boost::lexical_cast<mpz_int>(argv[1]);


    const mpz_int ten = 10;
    mpz_int power = 1;
    for (mpz_int i = 0; i < input; ++i) {
        power *= ten;
    }

    const mpz_int a = power; // since there is one 'tavtarto' at the wall


    const mpz_int z = a * a * 2;

    const mpf_float_1000 b = sqrt(static_cast<mpf_float_1000>(z));
    const mpf_float_1000 area = (a  + 1) * (b + 1);
    const mpz_int areaGuess = static_cast<mpz_int>(area / 2) + 1;

    //std::cout << "Area: " << area
    //          << " guess: " << areaGuess << std::endl;

    std::cout << areaGuess << std::endl;
}
