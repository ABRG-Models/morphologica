#include <iostream>
#include <morph/graphing.h>

int main()
{
    int rtn = 0;

    float a1 = 0.0f;
    float a2 = 10.0f;

    morph::range<float> nticks = {10, 10};

    std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks, true);

    std::cout << "ticks: ";
    for (auto t : ticks) {
        std::cout << t << ", ";
    }
    std::cout << std::endl;

    return rtn;
}
