#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    morph::vec<float, 2> dx = { 0.13, 0.13 };
    morph::vec<float, 2> offset = { -1, 2 };

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    //
    // morph::GridDomainWrap::None tests
    //
    std::cout << "WRAP NONE\n--------------------\n";
    {
        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
#if 0
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);
#endif
        std::cout << "ROW Bottom left top right...\n";
        if (g_bltr.index_lookup(morph::vec<float, 2>{-1, 2}) != 0) { --rtn; std::cout << "bltr coord lookup fails\n"; }
        if (g_bltr.index_lookup(morph::vec<float, 2>{-1+0.13, 2}) != 1) { --rtn; std::cout << "bltr coord lookup fails\n"; }

        if (g_bltr.index_lookup(morph::vec<float, 2>{-1+2*0.13+0.06, 2}) != 2) { --rtn; std::cout << "bltr coord lookup fails\n"; }
        if (g_bltr.index_lookup(morph::vec<float, 2>{-1+2*0.13+0.07, 2}) != 3) { --rtn; std::cout << "bltr coord lookup fails\n"; }
    }

    //
    // morph::GridDomainWrap::Horzontal tests
    //
    //wrap = morph::GridDomainWrap::Horizontal;
    //std::cout << "WRAP HORIZONTAL\n--------------------\n";
    //{
    //}

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
