#include "morph/Grid.h"
#include <iostream>

int main()
{
    constexpr morph::vec<float, 2> grid_spacing = { 1.0f, 1.0f };
    constexpr morph::vec<float, 2> grid_zero = { 0.0f, 0.0f };

    morph::Grid<10, 4, grid_spacing, grid_zero> grid;

    for (size_t i = 0; i < grid.n; ++i) {
        std::cout << "grid["<<i<<"]: "
                  << " has NW? " << grid.has_nw(i) << " has NE? " << grid.has_ne(i)
                  << " has NN? " << grid.has_nn(i) << " has NS? " << grid.has_ns(i) << std::endl;
    }

    for (size_t i = 0; i < grid.n; ++i) {
        std::cout << "grid["<<i<<"]: "
                  << " NW: " << (grid.has_nw(i) ? grid.index_nw(i) : 99) << " NE: " << (grid.has_ne(i) ? grid.index_ne(i) : 99)
                  << " NN: " << (grid.has_nn(i) ?  grid.index_nn(i) : 99) << " NS: " << (grid.has_ns(i) ?  grid.index_ns(i) : 99) << std::endl;
    }

    for (size_t i = 0; i < grid.n; ++i) {
        std::cout << "grid["<<i<<"]: "
                  << " NNW: " << (grid.has_nnw(i) ? grid.index_nnw(i) : 99) << " NNE: " << (grid.has_nne(i) ? grid.index_nne(i) : 99)
                  << " NSW: " << (grid.has_nsw(i) ?  grid.index_nsw(i) : 99) << " NSE: " << (grid.has_nse(i) ?  grid.index_nse(i) : 99) << std::endl;
    }

    morph::Grid<10, 4, grid_spacing, grid_zero, true,
                morph::CartDomainWrap::None, morph::GridOrder::topleft_to_bottomright> grid2;

    for (size_t i = 0; i < grid2.n; ++i) {
        std::cout << "grid2["<<i<<"]: "
                  << " has NW? " << grid2.has_nw(i) << " has NE? " << grid2.has_ne(i)
                  << " has NN? " << grid2.has_nn(i) << " has NS? " << grid2.has_ns(i) << std::endl;
    }

    for (size_t i = 0; i < grid2.n; ++i) {
        std::cout << "grid2["<<i<<"]: "
                  << " NW: " << (grid2.has_nw(i) ? grid2.index_nw(i) : 99) << " NE: " << (grid2.has_ne(i) ? grid2.index_ne(i) : 99)
                  << " NN: " << (grid2.has_nn(i) ?  grid2.index_nn(i) : 99) << " NS: " << (grid2.has_ns(i) ?  grid2.index_ns(i) : 99) << std::endl;
    }

    std::cout << "Extents for grid: " << grid.extents() << std::endl;

    // FIXME: Make the testing count for something!
    return 0;
}
