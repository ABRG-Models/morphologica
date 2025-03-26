#include <deque>
#include <iostream>
#include <morph/graphing.h>

template<typename T>
void print_ticks (const T& tcks)
{
    std::cout << "ticks: ";
    for (size_t i = 0; i < tcks.size(); ++i) {
        std::cout << morph::graphing::number_format (tcks[i], tcks[i==0?1:i-1]) << ", ";
    }
    std::cout << std::endl;
}

int main()
{
    int rtn = 0;

    float a1 = 0.0f;
    float a2 = 9.0f;

    for (unsigned int i = 2; i < 20; ++i) {
        morph::range<float> nticks = {static_cast<float>(i), static_cast<float>(i)};
        std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks);
        std::cout << "i ";
        print_ticks (ticks);
        if (ticks.size() != i) { --rtn; }
    }

    std::cout << "\n\n";

    for (unsigned int i = 3; i < 30; ++i) {
        morph::range<float> nticks = {static_cast<float>(i-1), static_cast<float>(i+1)};
        std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks);
        std::cout << "i+-1 ";
        print_ticks (ticks);
        if (nticks.includes (ticks.size()) == false) { --rtn; }
    }

    std::cout << "\n\n";

    for (unsigned int i = 4; i < 40; ++i) {
        morph::range<float> nticks = {static_cast<float>(i-1), static_cast<float>(i+1)};
        std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks);
        std::cout << "i+-2 ";
        print_ticks (ticks);
        if (nticks.includes (ticks.size()) == false) { --rtn; }
    }

    std::cout << "\n\n";

    for (unsigned int i = 2; i < 20; ++i) {
        morph::range<float> nticks = { 2, 12 };
        float f = 22.0f * static_cast<float>(i);
        std::deque<float> ticks = morph::graphing::maketicks (a1, a2+f, a1, a2+f, nticks);
        std::cout << "i ";
        print_ticks (ticks);
        if (nticks.includes (ticks.size()) == false) { --rtn; }
    }

    std::cout << "\n\n";
    for (float l = 1.0f; l < 20.0f; l += 1.0f) {
        for (unsigned int i = 3; i < 20; ++i) {
            morph::range<float> nticks = {static_cast<float>(i-1), static_cast<float>(i+1)};
            std::deque<float> ticks = morph::graphing::maketicks (a1, l, a1, l, nticks);
            std::cout << nticks << " ticks, data " << a1 << "-" << l << ": ";
            print_ticks (ticks);
            if (nticks.includes (ticks.size()) == false) { --rtn; }
        }
    }

    if (rtn == 0) { std::cout << "Test SUCCESS\n"; } else { std::cout << "Test FAIL\n"; }

    return rtn;
}
