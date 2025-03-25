#include <iostream>
#include <morph/graphing.h>

int main (int argc, char** argv)
{
    int rtn = 0;

    float a1 = 0.0f;
    float a2 = 9.0f;

    if (argc > 2) {
        a1 = std::atof (argv[1]);
        a2 = std::atof (argv[2]);
    }
    std::cout << "Data range: " << a1 << " to " << a2 << std::endl;

    morph::range<float> nticks = {3, 8};
    if (argc > 4) {
        nticks.min = std::atof (argv[3]);
        nticks.max = std::atof (argv[4]);
    }
    std::cout << "Number of ticks range: " << nticks << std::endl;

    std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks);

    std::cout << "\n" << a1 << " to " << a2 << ": ticks: ";
    for (auto t : ticks) { std::cout << t << ", "; }
    std::cout << std::endl;

    return rtn;
}
