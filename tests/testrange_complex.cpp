#include <iostream>
#include <complex>
#include <morph/range.h>
#include <morph/mathconst.h>

int main()
{
    int rtn = 0;

    morph::range<std::complex<float>> r({2.0f, 1.0f}, {4.0f, -1.0f});

    if (r.update ({10.0f, 30.0f}) == false) { --rtn; } // Update with this value should change the range and return true
    if (r.update ({10.0f, 30.0f}) == true) { --rtn; }  // Can't update again and get true returned

    if (r.includes ({0.1f, 0.04f}) == true) { --rtn; }
    if (r.includes ({6.5f, 3.0f}) == false) { --rtn; }

    std::cout << "r.span: " << r.span() << std::endl;

    // Fully real
    morph::range<std::complex<float>> r1({2.0f, 0.0f}, {4.0f, 0.0f});
    if (r1.update ({10.0f, 0.0f}) == false) { --rtn; }
    if (r1.update ({10.0f, 0.0f}) == true) { --rtn; }
    if (r1.update ({-20.0f, 0.0f}) == false) { --rtn; }
    if (r1.includes ({0.1f, 0.0f}) == false) { --rtn; std::cout << "0.1f + 0i erroneously excluded\n"; }
    if (r1.includes ({1.5f, 0.0f}) == false) { --rtn; std::cout << "1.5f + 0i erroneously excluded\n"; }
    if (r1.includes ({1.5f, 1.0f}) == true) { --rtn; std::cout << "1.5f + 1i erroneously included\n"; }
    std::cout << "r1.span: " << r1.span() << std::endl;

    // Fully imaginary
    morph::range<std::complex<float>> r2({0.0f, 2.0f}, {0.0f, 2.0f});
    if (r2.update ({0.0f, 10.0f}) == false) { --rtn; }
    if (r2.update ({0.0f, 10.0f}) == true) { --rtn; }
    if (r2.update ({0.0f, -20.0f}) == false) { --rtn; }
    if (r2.includes ({0.1f, 0.0f}) == true) { --rtn; std::cout << "0.1 + 0i erroneously included\n"; }
    if (r2.includes ({0.0f, 1.0f}) == false) { --rtn; std::cout << "0 + 1i erroneously excluded\n"; }
    if (r2.includes ({0.0f, 100.0f}) == true) { --rtn; std::cout << "0 + 100i erroneously included\n"; }
    std::cout << "r2.span: " << r2.span() << std::endl;

    std::cout << "Test " << (rtn == 0 ? "Passed" : "Failed") << std::endl;
    return rtn;
}
