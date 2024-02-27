#include "morph/Grid.h"
#include <iostream>

int main()
{
    constexpr morph::vec<float, 2> grid_spacing = {1.0f, 1.0f};
    constexpr morph::vec<float, 2> grid_zero = {0.0f, 0.0f};
    morph::Grid<10, 4, grid_spacing, grid_zero> grid;

    for (size_t i = 0; i < grid.n; ++i) {
        std::cout << "Location of grid["<<i<<"]: " << grid[i]
                  << " has NW? " << grid.has_nw(i) << " has NE? " << grid.has_ne(i)
                  << " has NN? " << grid.has_nn(i) << std::endl;
    }
    std::cout << "Neighbour east of " << grid[0] << " is " << grid.coord_ne(0) << std::endl;
    std::cout << "Neighbour east of " << grid[9] << " is " << grid.coord_ne(9) << std::endl;
    std::cout << "Neighbour east of " << grid[39] << " is " << grid.coord_ne(39) << std::endl;
    std::cout << "Index NE of " << grid[39] << " is " << grid.index_ne(39) << std::endl;
    return 0;
}
