/*
 * An example program showing you how the indices are arranged in Grids with different orders. This
 * prints out the grid to stdout for inspection.
 */
#include "morph/Grid.h"
#include <iostream>

int main()
{
    morph::vec<float, 2> dx = { 1, 1 };
    morph::vec<float, 2> offset = { 0, 0 };

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
    morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
    morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
    morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

    std::cout << "Calling Grid::str() for a width 4, height 2 grid (different orders; offset (0,0)):\n\n";
    std::cout << g_bltr.str() << std::endl;
    std::cout << g_tlbr.str() << std::endl;
    std::cout << g_bltrc.str() << std::endl;
    std::cout << g_tlbrc.str() << std::endl;
    return 0;
}
