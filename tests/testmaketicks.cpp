#include <deque>
#include <iostream>
#include <morph/graphing.h>

template<typename T>
void print_ticks (const T& tcks)
{
    std::cout << "ticks: ";
    for (auto t : tcks) { std::cout << t << ", "; }
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

    for (float l = 4.0f; l < 12.0f; ++l) {
        // Asking for 0 or 1 ticks should give min 2
        morph::range<float> nticks = { 0.0f, 1.0f };
        std::deque<float> ticks = morph::graphing::maketicks (-l, l, -l, l, nticks);
        std::cout << "l " << l << ", [0,1]: ";
        print_ticks (ticks);
        if (ticks.size() < 2) { --rtn; }
    }

    std::cout << "\n\n";

    for (float l = 4.0f; l < 12.0f; ++l) {
        // Asking for 0 or 1 ticks should give min 2
        morph::range<float> nticks = { 0.0f, 1.0f };
        std::deque<float> ticks = morph::graphing::maketicks (0.0f, l, 0.0f, l, nticks);
        std::cout << "l " << l << ", [0,1]: ";
        print_ticks (ticks);
        if (ticks.size() < 2) { --rtn; }
    }

    std::cout << "\n\n";

    if (rtn == 0) { std::cout << "Test SUCCESS\n"; } else { std::cout << "Test FAIL\n"; }

    return rtn;
}
