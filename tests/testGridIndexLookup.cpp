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

    //std::cout << "part 1\n";
    int idx_expected = 0;

    for (int yii = 0; yii < g_bltr.get_h(); yii++) {
        float yi = offset[1] + coord_shift[1] + dx[1] * yii;
        for (int xii = 0; xii < g_bltr.get_w(); xii++) {
            float xi = offset[0] + coord_shift[0] + dx[0] * xii;

            int i_l = g_bltr.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "bltr coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_bltr.coord_lookup(i_l);
            if ((c - (morph::vec<float, 2>{xi, yi} - coord_shift)).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_bltr.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} - coord_shift) << "\n";
                --rtn;
            }
        }
    }

    //std::cout << "part 2\n";
    idx_expected = 0;
    for (int xii = 0; xii < g_bltr.get_w(); xii++) {
        float xi = offset[0] + coord_shift[0] + dx[0] * xii;
        for (int yii = 0; yii < g_bltr.get_h(); yii++) {
            float yi = offset[1] + coord_shift[1] + dx[1] * yii;

            int i_l =g_bltrc.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "bltrc coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_bltrc.coord_lookup(i_l);
            if ((c - (morph::vec<float, 2>{xi, yi} - coord_shift)).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_bltrc.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} - coord_shift) << "\n";
                --rtn;
            }
        }
    }

    //std::cout << "part 3\n";
    idx_expected = 0;
    for (int yii = 0; yii < g_bltr.get_h(); yii++) {
        float yi = offset[1] + coord_shift[1] - dx[1] * yii;
        for (int xii = 0; xii < g_bltr.get_w(); xii++) {
            float xi = offset[0] + coord_shift[0] + dx[0] * xii;

            int i_l = g_tlbr.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "tlbr coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_tlbr.coord_lookup(i_l);
            if ((c - (morph::vec<float, 2>{xi, yi} - coord_shift)).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_tlbr.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} - coord_shift) << "\n";
                --rtn;
            }
        }
    }
    return rtn;

    //std::cout << "part 4\n";
    idx_expected = 0;
    for (int xii = 0; xii < g_bltr.get_w(); xii++) {
        float xi = offset[0] + coord_shift[0] + dx[0] * xii;
        for (int yii = 0; yii < g_bltr.get_h(); yii++) {
            float yi = offset[1] + coord_shift[1] - dx[1] * yii;

            int i_l = g_tlbrc.index_lookup(morph::vec<float, 2>{xi, yi});
            if (i_l != idx_expected++) {
                --rtn;
                std::cout << "tlbrc coord lookup fails for ("<<xi<<","<<yi<<")\n";
            }
            morph::vec<float, 2> c = g_tlbrc.coord_lookup(i_l);
            if ((c - (morph::vec<float, 2>{xi, yi} - coord_shift)).abs().sum() > std::numeric_limits<float>::epsilon()) {
                std::cout << "g_tlbrc.coord_lookup("<<i_l<<") returned " << c << " not " << (morph::vec<float, 2>{xi, yi} - coord_shift) << "\n";
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
    try {
        for (float shift = 0.0f; shift < 0.25f; shift += 0.01f) {
            std::cout << "Test batch for shift = " << shift << std::endl;
            rtn += do_test (wrap, morph::vec<float, 2>{shift, 0.0f});
            rtn += do_test (wrap, morph::vec<float, 2>{-shift, 0.0f});
            rtn += do_test (wrap, morph::vec<float, 2>{0.0f, shift});
            rtn += do_test (wrap, morph::vec<float, 2>{0.0f, -shift});
        }
    } catch (const std::exception& e) {
        std::cout << "Exception running test batch: " << e.what() << "\n";
        --rtn;
    }

    float shift = 0.25f;  // expect fail
    try {
        do_test (wrap, morph::vec<float, 2>{shift, 0.0f});
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << "\n";
    }

    //
    // morph::GridDomainWrap::Horizontal tests should work exactly the same
    //
    wrap = morph::GridDomainWrap::Horizontal;
    try {
        for (float shift = 0.0f; shift < 0.25f; shift += 0.01f) {
            std::cout << "Test batch for shift = " << shift << std::endl;
            rtn += do_test (wrap, morph::vec<float, 2>{shift, 0.0f});
            rtn += do_test (wrap, morph::vec<float, 2>{-shift, 0.0f});
            rtn += do_test (wrap, morph::vec<float, 2>{0.0f, shift});
            rtn += do_test (wrap, morph::vec<float, 2>{0.0f, -shift});
        }
    } catch (const std::exception& e) {
        std::cout << "Exception running test batch: " << e.what() << "\n";
        --rtn;
    }

    shift = 0.25f;  // expect fail
    try {
        do_test (wrap, morph::vec<float, 2>{shift, 0.0f});
        --rtn;
        std::cout << "Unexpected pass 1\n";
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << "\n";
    }
    try {
        do_test (wrap, morph::vec<float, 2>{-0.51f, 0.0f});
        --rtn;
        std::cout << "Unexpected pass 2\n";
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << "\n";
    }
    try {
        do_test (wrap, morph::vec<float, 2>{0.0f, shift});
        --rtn;
        std::cout << "Unexpected pass 3\n";
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << "\n";
    }
    try {
        do_test (wrap, morph::vec<float, 2>{0.0f, -shift});
        --rtn;
        std::cout << "Unexpected pass 4\n";
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << "\n";
    }

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed (rtn = " << rtn << ")\n";
    }
    return rtn;
}
