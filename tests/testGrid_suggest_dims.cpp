#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    std::cout << "Allowing only 'perfect' grids:\n";
    for (int n = 20; n < 40; ++n) {
        morph::vec<int, 2> dims = morph::Grid<int, float>::suggest_dims (n, false);
        if (dims[0] != std::numeric_limits<int>::max()) {
            std::cout << n << " elements can be made into a grid of dims " << dims << std::endl;
            if (n != dims.product()) { --rtn; }
        } else {
            std::cout << n << " elements can't be made into a perfect grid" << std::endl;
        }
    }

    std::cout << "\nAllowing imperfect grids:\n";
    for (int n = 20; n < 40; ++n) {
        morph::vec<int, 2> dims = morph::Grid<int, float>::suggest_dims (n, true);
        if (dims[0] != std::numeric_limits<int>::max()) {
            if (n == dims.product()) {
                std::cout << n << " elements can be made into a perfect grid of dims " << dims << std::endl;
            } else {
                std::cout << n << " elements can be made into a near-enough grid of dims " << dims << std::endl;
                if (dims.product() <= n) { --rtn; }
            }
        } else {
            std::cout << n << " elements can't be made into a perfect grid" << std::endl;
        }
    }

    std::cout << "\nTest " << (rtn ? "failed" : "passed") << std::endl;
    return rtn;
}
