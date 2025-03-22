#include <iostream>

// To test non-format code:
#ifdef FORCE_NON_FORMAT
# ifdef MORPH_HAVE_STD_FORMAT
#  undef MORPH_HAVE_STD_FORMAT
# endif
#endif

#include <morph/graphing.h>

int main()
{
    int rtn = 0;

    float num = 0.0f;
    float next = 0.0f;

    num = 1.0f;
    next = 2.0f;
    std::string str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "1") { std::cout << "fail\n"; --rtn; }

    num = -2.0f;
    next = -1.0f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "-2") { std::cout << "fail\n"; --rtn; }

    num = 13.8889f;
    next = 6.94444f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "13.9") { std::cout << "fail\n"; --rtn; }

    num = 1000.01f;
    next = 1000.04f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "1000.01") { std::cout << "fail\n"; --rtn; }

    num = 0.01f;
    next = 0.04f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != ".01") { std::cout << "fail\n"; --rtn; }

    num = -0.0104f;
    next = -0.0105999999999f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "-.0104") { std::cout << "fail\n"; --rtn; }

    num = -0.30000001f;
    next = -0.350000f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "-.30") { std::cout << "fail\n"; --rtn; }

    num = -0.01060f;
    next = -0.010401f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "-.0106") { std::cout << "fail\n"; --rtn; }

    num = 0.01040f;
    next = 0.0103f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != ".0104") { std::cout << "fail\n"; --rtn; }

    num = 20000.0f;
    next = 10000.0f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "2e+04") { std::cout << "fail\n"; --rtn; }

    num = 20000.0f;
    next = 20001.0f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "2.0000e+04") { std::cout << "fail\n"; --rtn; }

    num = 20000.0f;
    next = 20010.0f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "2.000e+04") { std::cout << "fail\n"; --rtn; }

#if 0
    std::cout << "std::floor (-4.0f) = " << std::floor (-4.0f) << std::endl;

    float up = 9.99998e-05f * 1e5f;
    float up2 = 1e-04f * 1e4f;
    std::cout << "up=" << up << std::endl;
    std::cout << "up2=" << up2 << std::endl;
    std::cout << "10 - up=" << (10.0f - up) << std::endl;
    std::cout << "10 - up2=" << (10.0f - up2) << std::endl;

    int expnt_ = static_cast<int>(std::floor (std::log10 (10.0f - up)));
    std::cout << "expnt_ = " << expnt_ << std::endl;
    std::cout << std::log10(9.99998e-05f) + 4.0f  << std::endl;
    std::cout << std::log10(1e-4f) + 4.0f  << std::endl;
    std::cout << std::floor (std::log10(9.99998e-05f))  << std::endl;
    std::cout << std::floor (std::log10(1e-4f))  << std::endl;
#endif
    std::cout << (rtn ? "FAIL\n" : "PASS\n");
    return rtn;
}
