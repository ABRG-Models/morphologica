#include <iostream>
#include <morph/math.h>

template <typename F>
bool different (F f1, F f2) { return std::abs(f1 - f2) > std::numeric_limits<F>::epsilon(); }

int main()
{
    int rtn = 0;
    int tnum = 1;

        std::cout.precision(20);

    float f = 1.2345f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    float fr = morph::math::round_to_col (f, -2);
    std::cout << "fr: " << fr << std::endl;
    if (different(fr, 1.23f)) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 1.2345f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    fr = morph::math::round_to_col (f, -6);
    std::cout << "fr: " << fr << std::endl;
    if (different(fr, 1.2345f)) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 1.2345f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    fr = morph::math::round_to_col (f, 0);
    std::cout << "fr: " << fr << std::endl;
    if (different(fr, 1.0f)) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 1000.01f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    fr = morph::math::round_to_col (f, -2);
    std::cout << "fr: " << fr << std::endl;
    if (different(fr, 1000.01f)) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    std::cout << (rtn ? "FAIL" : "PASS") << std::endl;

    return rtn;
}
