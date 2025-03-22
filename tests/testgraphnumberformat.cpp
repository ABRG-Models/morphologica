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

#if 0 // next:
    num = 1000.01f;
    next = 1000.04f;
    str = morph::graphing::number_format (num, next);
    std::cout << "gnf ("<<num<<", "<<next<<"): " << str << std::endl;
    if (str != "1000.01") { std::cout << "fail\n"; --rtn; }
#endif

    std::cout << (rtn ? "FAIL\n" : "PASS\n");

    return rtn;
}
