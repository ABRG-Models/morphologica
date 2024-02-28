/*
 * Profile CartGrid and my simpler grid. Just for interest.
 */

#include <morph/CartGrid.h>
#include <morph/Grid.h>
#include <chrono>

int main()
{
    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    constexpr size_t Nside = 1000;
    constexpr morph::vec<float, 2> grid_spacing = {1.0f, 1.0f};
    constexpr morph::vec<float, 2> grid_zero = {0.0f, 0.0f};
    constexpr morph::CartDomainWrap d_wrap = morph::CartDomainWrap::None;

    sc::time_point t0 = sc::now();

    // A Grid with no memory use (fast to instantiate, slower to access coordinates)
    morph::Grid<Nside, Nside, grid_spacing, grid_zero, false, d_wrap> grid;

    sc::time_point t1 = sc::now();

    // A Grid with memory use (slower to instantiate, fast to access coordinates)
    constexpr bool memory_vecs = true;
    morph::Grid<Nside, Nside, grid_spacing, grid_zero, memory_vecs, d_wrap> grid_mem;

    sc::time_point t2 = sc::now();

    // A CartGrid object
    morph::CartGrid cgrid(grid_spacing[0], grid_spacing[1],
                          grid_zero[0], grid_zero[1], (Nside-1)*grid_spacing[0], (Nside-1)*grid_spacing[1], 0.0f,
                          morph::CartDomainShape::Rectangle, d_wrap);
    cgrid.setBoundaryOnOuterEdge();
    sc::time_point t3 = sc::now();

    std::cout << "Grid sizes: " << grid.n << " and " << cgrid.num() << std::endl;

    std::cout << "Grid instantiation (without memory vecs):   " << duration_cast<milliseconds>(t1-t0).count() << " ms\n";
    std::cout << "Grid instantiation (WITH memory vecs):      " << duration_cast<milliseconds>(t2-t1).count() << " ms\n";
    std::cout << "CartGrid instantiation:                     " << duration_cast<milliseconds>(t3-t2).count() << " ms\n";

    std::cout << "\nGrid without memory\n------------------------------\n";

    morph::vec<float, 2> one_coordinate = {0,0};
    t0 = sc::now();
    for (size_t i = 0; i < grid.n; ++i) { one_coordinate += grid[i]; }
    t1 = sc::now();
    std::cout << "Grid (no mem) access as '+= grid[i]':               " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    std::cout << "\nGrid WITH memory\n------------------------------\n";
    t0 = sc::now();
    for (size_t i = 0; i < grid_mem.n; ++i) { one_coordinate += grid_mem[i]; }
    t1 = sc::now();
    std::cout << "Grid (WITH mem) access as  '+= grid[i]':            " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    t0 = sc::now();
    for (size_t i = 0; i < grid_mem.n; ++i) {
        one_coordinate[0] += grid_mem.d_x[i];
        one_coordinate[1] += grid_mem.d_y[i];

    }
    t1 = sc::now();
    std::cout << "Grid (WITH mem) access as '+= grid.d_x[i]/d_y[i]':  " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    std::cout << "\nCartGrid (also WITH memory)\n------------------------------\n";
    t0 = sc::now();
    for (size_t i = 0; i < cgrid.num(); ++i) {
        one_coordinate[0] += cgrid.d_x[i];
        one_coordinate[1] += cgrid.d_y[i];
    }
    t1 = sc::now();
    std::cout << "CartGrid access as '+= cgrid.d_x[i]/d_y[i]':        " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    std::cout << "\n\nLast one_coordinate: " << one_coordinate << std::endl;
    return 0;
}
