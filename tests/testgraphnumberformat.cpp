#include <iostream>
// To test non-format code:
//#ifdef MORPH_HAVE_STD_FORMAT
//#undef MORPH_HAVE_STD_FORMAT
//#endif
#include <morph/graphing.h>

int main()
{
    float low = 1000.1f;
    float high = 2000.2f;
#if 0
    std::cout << "gnf (2.345f): " << morph::graphing::number_format (2.345f) << std::endl;
    std::cout << "gnf (2.345f, 2.335f): " << morph::graphing::number_format (2.345f, 2.335f) << std::endl;
    std::cout << "gnf (2.1f, 20.3f): " << morph::graphing::number_format (2.1f, 20.3f) << std::endl;
    for (int i = 0; i < 6; ++i) {
        std::cout << "gnf ("<<low<<", "<<high<<"): " << morph::graphing::number_format (low, high) << std::endl;
        low *= 10;
        high *= 10;
    }
    std::cout << "\n\n";
#endif

    low = 1.0f;
    high = 2.0f;
    std::cout << "gnf ("<<low<<", "<<high<<"): " << morph::graphing::number_format (low, high) << std::endl;
    low = -2.0f;
    high = -1.0f;
    std::cout << "gnf ("<<low<<", "<<high<<"): " << morph::graphing::number_format (low, high) << std::endl;

    low = 13.8889f;
    high = 6.94444f;
    std::cout << "gnf ("<<low<<", "<<high<<"): " << morph::graphing::number_format (low, high) << std::endl;
}
