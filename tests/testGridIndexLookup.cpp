#include "morph/Grid.h"
#include <iostream>
#include <limits>

// The test checks that locations exactly on the grid work
int do_test (morph::GridDomainWrap wrap, const morph::vec<float, 2>& coord_shift)
{
    int rtn = 0;

    morph::vec<float, 2> dx = { 0.5f, 0.5f };
    morph::vec<float, 2> offset = { -0.5f, 1.0f };
    morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
    morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
    morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
    morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

    int idx_expected = 0;
    for (float yi = 1.0f + coord_shift[1]; yi < 2.0f + coord_shift[1]; yi += dx[1]) {
        for (float xi = -0.5f + coord_shift[0]; xi < 1.5f + coord_shift[0]; xi += dx[0]) {
            int i_l = g_bltr.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "bltr coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_bltr.coord_lookup(i_l);
            std::cout << "g_bltr.coord_lookup("<<i_l<<") returned " << c << " cf " << morph::vec<float, 2>{xi, yi} << "\n";
            if ((c - morph::vec<float, 2>{xi, yi} + coord_shift).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_bltr.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} + coord_shift) << "\n";
                --rtn;
            }
        }
    }

    idx_expected = 0;
    for (float xi = -0.5f + coord_shift[0]; xi < 1.5f + coord_shift[0]; xi += dx[0]) {
        for (float yi = 1.0f + coord_shift[1]; yi < 2.0f + coord_shift[1]; yi += dx[1]) {
            int i_l =g_bltrc.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "bltrc coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_bltrc.coord_lookup(i_l);
            if ((c - morph::vec<float, 2>{xi, yi} + coord_shift).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_bltrc.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} + coord_shift) << "\n";
                --rtn;
            }
        }
    }

    idx_expected = 0;
    for (float yi = 1.0f + coord_shift[1]; yi > 0.0f + coord_shift[1]; yi -= dx[1]) {
        for (float xi = -0.5f + coord_shift[0]; xi < 1.5f + coord_shift[0]; xi += dx[0]) {
            int i_l = g_tlbr.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "tlbr coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_tlbr.coord_lookup(i_l);
            if ((c - morph::vec<float, 2>{xi, yi}).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_tlbr.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} + coord_shift) << "\n";
                --rtn;
            }
        }
    }

    idx_expected = 0;
    for (float xi = -0.5f + coord_shift[0]; xi < 1.5f + coord_shift[0]; xi += dx[0]) {
        for (float yi = 1.0f + coord_shift[1]; yi > 0.0f + coord_shift[1]; yi -= dx[1]) {
            int i_l = g_tlbrc.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "tlbrc coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_tlbrc.coord_lookup(i_l);
            if ((c - morph::vec<float, 2>{xi, yi} + coord_shift).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_tlbrc.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} + coord_shift) << "\n";
                --rtn;
            }
        }
    }

    return rtn;
}

int main()
{
    int rtn = 0;

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    //
    // morph::GridDomainWrap::None tests
    //
    std::cout << "Test 1.\n";
    rtn += do_test (wrap, morph::vec<float, 2>{0, 0});

    //std::cout << "Test 2.\n"; // FIXME: Work on this
    //rtn += do_test (wrap, morph::vec<float, 2>{0.24, 0});

    //rtn += do_test (wrap, morph::vec<float, 2>{-0.24, 0});
    //rtn += do_test (wrap, morph::vec<float, 2>{0, 0.24});
    //rtn += do_test (wrap, morph::vec<float, 2>{0, -0.24});

    //
    // morph::GridDomainWrap::Horzontal tests should work exactly the same
    //
    //wrap = morph::GridDomainWrap::Horizontal;
    //rtn += do_test (wrap, morph::vec<float, 2>{0, 0});

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
