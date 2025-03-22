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

#if 0 // known to fail at present:
    num = 8.75f;
    next = 7.5f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "8.75") { std::cout << "fail\n"; --rtn; }
#endif

    std::cout << (rtn ? "FAIL\n" : "PASS\n");
    return rtn;
}
