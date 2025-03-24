#include <iostream>
#include <morph/math.h>

int main()
{
    int rtn = 0;
    int tnum = 1;

    std::cout.precision(20);

    float f = 3.999999999998f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    morph::range<int> sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{0, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 4.123f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-3, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = -4.123f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-3, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    int prec = morph::math::significant_figs (f);
    std::cout << "prec: " << prec << std::endl;
    if (prec != 3) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = -4.12345678f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-6, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 23948318.0f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{1, 7}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 2394000.0f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{3, 6}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    // This example gives us the 'rounding problem'
    double d = 23948318.123;
    std::cout << "\nTest " << tnum << ": " <<  d << std::endl;
    sf = morph::math::significant_cols (d);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-3, 7}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    d = 23.123832;
    std::cout << "\nTest " << tnum << ": " <<  d << std::endl;
    sf = morph::math::significant_cols (d);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-6, 1}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    // Test nan and constexpr
    std::cout << "\nTest " << tnum << ": " <<  std::numeric_limits<double>::quiet_NaN() << std::endl;
    constexpr morph::range<int> sfce = morph::math::significant_cols (std::numeric_limits<double>::quiet_NaN());
    std::cout << "sf: " << sfce << std::endl;
    if (sfce != morph::range<int>{0, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    d = 2.999275;
    std::cout << "\nTest " << tnum << ": " <<  d << std::endl;
    sf = morph::math::significant_cols (d);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-6, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    d = 2.9999275;
    std::cout << "\nTest " << tnum << ": " <<  d << std::endl;
    sf = morph::math::significant_cols (d);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-7, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    d = 2.9999975;
    std::cout << "\nTest " << tnum << ": " <<  d << std::endl;
    sf = morph::math::significant_cols (d);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-7, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    // Back to float
    f = 2.9999975f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-6, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 2.9999995f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{0, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;

    f = 9.36f;
    std::cout << "\nTest " << tnum << ": " <<  f << std::endl;
    sf = morph::math::significant_cols (f);
    std::cout << "sf: " << sf << std::endl;
    if (sf != morph::range<int>{-2, 0}) { std::cout << "Fail " << tnum << std::endl; --rtn; }
    ++tnum;


    std::cout << (rtn ? "FAIL" : "PASS") << std::endl;

    return rtn;
}
