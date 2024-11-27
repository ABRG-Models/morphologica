#include <morph/vvec.h>
#include <morph/Random.h>
#include <morph/Grid.h>
#include <iostream>

// Grid extended with a coord_lookup function that calls get_n_pixels for each lookup.
template<typename I = unsigned int, typename C = float>
struct GridPlus : public morph::Grid<I, C>
{
    GridPlus (const I _w, const I _h,
              const morph::vec<C, 2> _dx = { C{1}, C{1} },
              const morph::vec<C, 2> _offset = { C{0}, C{0} },
              const morph::GridDomainWrap _wrap = morph::GridDomainWrap::None,
              const morph::GridOrder _order = morph::GridOrder::bottomleft_to_topright)
        : morph::Grid<I, C>::Grid (_w, _h, _dx, _offset, _wrap, _order) { this->n_mem = this->n(); };

    I n_mem = I{0};

    morph::vec<C, 2> coord_lookup_with_mem_n (const I index) const
    {
        return index >= this->n_mem ? morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()} : this->v_c[index];
    }
};

// Profiling with std::chrono:
#include <chrono>
using namespace std::chrono;
using sc = std::chrono::steady_clock;

int main()
{
    int rtn = 0;
    morph::vec<float, 2> dx = {1.0f, 1.0f};
    morph::vec<float, 2> offset = {0.0f, 0.0f};
    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;
    morph::GridOrder order = morph::GridOrder::bottomleft_to_topright;

    int _w = 500;
    int _h = 400;

    morph::vvec<morph::vec<float, 2>> coords (_w * _h, {0.0f});
    // Random indices with a seed
    morph::RandUniform<int> rng (0, _w * _h, 1020u);
    std::vector<std::vector<int>> ridx(1000);
    for (int j = 0; j < 1000; ++j) {
        ridx[j] = rng.get (_w * _h);
    }

    {
        GridPlus<int, float> g(_w, _h, dx, offset, wrap, order);

        sc::time_point t0 = sc::now();
        for (int j = 0; j < 1000; ++j) {
            for (int i = 0; i < g.n(); ++i) {
                coords[i] = g.coord_lookup (ridx[j][i]);
            }
        }
        sc::time_point t1 = sc::now();

        sc::duration t_d = t1 - t0;
        std::cout << "coords vvec filled in " << duration_cast<microseconds>(t_d).count() << " us with coord_lookup (w*h performed each time)\n";
    }

    {
        GridPlus<int, float> g(_w, _h, dx, offset, wrap, order);

        sc::time_point t0 = sc::now();
        for (int j = 0; j < 1000; ++j) {
            for (int i = 0; i < g.n_mem; ++i) {
                coords[i] = g.coord_lookup_with_mem_n (ridx[j][i]);
            }
        }
        sc::time_point t1 = sc::now();

        sc::duration t_d = t1 - t0;
        std::cout << "coords vvec filled in " << duration_cast<microseconds>(t_d).count() << " us with coord_lookup (w*h in memory)\n";
    }

    return rtn;
}
