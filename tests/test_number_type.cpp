#include <iostream>
#include <complex>
#include <morph/vec.h>
#include <morph/trait_tests.h>


// What does morph::number_type return for std::complex? This is a test just of morph::number_type
// from trait_tests.h (it used to be in its own header)
int main()
{
    int rtn = 0;

    if constexpr (morph::number_type<float>::value == 1) {
        std::cout << "float is scalar\n";
    } else { --rtn; }

    if constexpr (morph::number_type<morph::vec<float, 3>>::value == 0) {
        std::cout << "vec<float> is not scalar\n";
    } else { --rtn; }

    if constexpr (morph::number_type<std::complex<float>>::value == 2) {
        std::cout << "std::complex<float> is a complex scalar\n";
    } else { --rtn; }

    std::cout << "morph::number_type test " << (rtn == 0 ? "passed" : "failed") << std::endl;
    return rtn;
}
