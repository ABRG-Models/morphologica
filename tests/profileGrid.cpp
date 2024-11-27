/*
 * Profile CartGrid and my simpler grid. Just for interest.
 */

#include <morph/CartGrid.h>
#include <morph/Gridct.h>
#include <morph/Grid.h>
#include <chrono>

int main()
{
    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    constexpr unsigned int Nside = 1000;
    constexpr morph::vec<float, 2> grid_spacing = {1.0f, 1.0f};
    constexpr morph::vec<float, 2> grid_zero = {0.0f, 0.0f};
    constexpr morph::GridDomainWrap d_wrap = morph::GridDomainWrap::None;

    sc::time_point t0 = sc::now();

    // A Grid with no memory use (fast to instantiate, slower to access coordinates)
    morph::Gridct<unsigned int, float, Nside, Nside, grid_spacing, grid_zero, false, d_wrap> grid_ct;

    sc::time_point t1 = sc::now();

    // A Grid with memory use (slower to instantiate, fast to access coordinates)
    constexpr bool memory_vecs = true;
    morph::Gridct<unsigned int, float, Nside, Nside, grid_spacing, grid_zero, memory_vecs, d_wrap> grid_ct_mem;

    sc::time_point t2 = sc::now();

    // A CartGrid object
    morph::CartGrid cgrid(grid_spacing[0], grid_spacing[1],
                          grid_zero[0], grid_zero[1], (Nside-1)*grid_spacing[0], (Nside-1)*grid_spacing[1], 0.0f,
                          morph::GridDomainShape::Rectangle, d_wrap);
    cgrid.setBoundaryOnOuterEdge();
    sc::time_point t3 = sc::now();

    morph::Grid grid_rt(Nside, Nside, grid_spacing, grid_zero, d_wrap);

    sc::time_point t4 = sc::now();

    std::cout << "Grid sizes: " << grid_ct.n << " and " << grid_rt.n() << " and " << cgrid.num() << std::endl;

    std::cout << "Gridct instantiation (without memory vecs): " << duration_cast<milliseconds>(t1-t0).count() << " ms\n";
    std::cout << "Gridct instantiation (WITH memory vecs):    " << duration_cast<milliseconds>(t2-t1).count() << " ms\n";
    std::cout << "CartGrid instantiation:                     " << duration_cast<milliseconds>(t3-t2).count() << " ms\n";
    std::cout << "Grid instantiation:                         " << duration_cast<milliseconds>(t4-t3).count() << " ms\n";

    std::cout << "\nGridct without memory\n------------------------------\n";

    morph::vec<float, 2> one_coordinate = {0,0};
    t0 = sc::now();
    for (unsigned int i = 0; i < grid_ct.n; ++i) { one_coordinate += grid_ct[i]; }
    t1 = sc::now();
    std::cout << "Gridct (no mem) access as '+= grid[i]':               " << duration_cast<microseconds>(t1-t0).count() << " us\n(one_coordinate: " << one_coordinate << "\n";

    one_coordinate = {0,0};
    std::cout << "\nGridct WITH memory\n------------------------------\n";
    t0 = sc::now();
    for (unsigned int i = 0; i < grid_ct_mem.n; ++i) { one_coordinate += grid_ct_mem[i]; }
    t1 = sc::now();
    std::cout << "Gridct (WITH mem) access as  '+= grid[i]':            " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    t0 = sc::now();
    for (unsigned int i = 0; i < grid_ct_mem.n; ++i) {
        one_coordinate[0] += grid_ct_mem.v_x[i];
        one_coordinate[1] += grid_ct_mem.v_y[i];
    }
    t1 = sc::now();
    std::cout << "Gridct (WITH mem) access as '+= grid.d_x[i]/d_y[i]':  " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    std::cout << "\nGrid without memory\n------------------------------\n";

    one_coordinate = {0,0};
    t0 = sc::now();
    for (unsigned int i = 0; i < grid_rt.n(); ++i) { one_coordinate += grid_rt.coord(i); }
    t1 = sc::now();
    std::cout << "Grid (no mem) access as '+= grid_rt.coord(i)':        " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    std::cout << "\nGrid WITH memory\n------------------------------\n";
    t0 = sc::now();
    for (unsigned int i = 0; i < grid_rt.n(); ++i) { one_coordinate += grid_rt[i]; }
    t1 = sc::now();
    std::cout << "Grid (WITH mem) access as  '+= grid_rt[i]':           " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    t0 = sc::now();
    for (unsigned int i = 0; i < grid_rt.n(); ++i) { one_coordinate += grid_rt.coord_ne(i); }
    t1 = sc::now();
    std::cout << "Grid neighbour access as  '+= grid_rt.coord_ne(i)':   " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    t0 = sc::now();
    for (unsigned int i = 0; i < grid_rt.n(); ++i) { one_coordinate += grid_rt.coord_nne(i); }
    t1 = sc::now();
    std::cout << "Grid neighbour access as  '+= grid_rt.coord_nne(i)':  " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    one_coordinate = {0,0};
    std::cout << "\nCartGrid (also WITH memory)\n------------------------------\n";
    t0 = sc::now();
    for (unsigned int i = 0; i < cgrid.num(); ++i) {
        one_coordinate[0] += cgrid.d_x[i];
        one_coordinate[1] += cgrid.d_y[i];
    }
    t1 = sc::now();
    std::cout << "CartGrid access as '+= cgrid.d_x[i]/d_y[i]':          " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    t0 = sc::now();
    for (unsigned int i = 0; i < cgrid.num(); ++i) {
        int ii = cgrid.d_ne[i];
        if (ii > -1) {
            one_coordinate[0] += cgrid.d_x[ii];
            one_coordinate[1] += cgrid.d_y[ii];
        }
    }

    t1 = sc::now();
    std::cout << "CartGrid neighbour access':                           " << duration_cast<microseconds>(t1-t0).count() << " us\n";

    std::cout << "\n\nLast one_coordinate: " << one_coordinate << std::endl;
    return 0;
}
