#include <iostream>
#include <morph/graphing.h>

void usage (char** argv)
{
    std::cout << "Usage\n";
    std::cout << argv[0] << " <ticksmin> <ticksmax> <num ticks at least> <num ticks at most>\n";
}

int main (int argc, char** argv)
{
    float a1 = 0.0f;
    float a2 = 9.0f;

    if (argc > 2) {
        a1 = std::atof (argv[1]);
        a2 = std::atof (argv[2]);
    } else {
        usage(argv);
        return -1;
    }
    std::cout << "Data range: " << a1 << " to " << a2 << std::endl;

    morph::range<float> nticks = {3, 8};
    if (argc > 4) {
        nticks.min = std::atof (argv[3]);
        nticks.max = std::atof (argv[4]);
    } else {
        usage(argv);
        return -1;
    }
    std::cout << "Number of ticks range: " << nticks << std::endl;

    std::deque<float> ticks = morph::graphing::maketicks (a1, a2, a1, a2, nticks);

    std::cout << "\n" << a1 << " to " << a2 << ": ticks: ";
    for (auto t : ticks) { std::cout << t << ", "; }
    std::cout << std::endl;
}
