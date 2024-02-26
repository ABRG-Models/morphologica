#include "morph/Grid.h"
#include <iostream>

int main()
{
    morph::Grid<10, 4, 0.1f, 0.1f> grid;
    for (size_t i = 0; i < grid.n; ++i) {
        std::cout << "Location of grid["<<i<<"]: " << grid[i] << std::endl;
    }
    return 0;
}
