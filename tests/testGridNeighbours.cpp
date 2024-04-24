#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    morph::vec<float, 2> dx = { 1, 1 };
    morph::vec<float, 2> offset = { 0, 0 };

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
    morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
    //morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
    //morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);
/*
    // Test neighbour north/south in g_bltr
    if (g_bltr.index_nn(0) != 4) { --rtn; }
    if (g_bltr.index_nn(1) != 5) { --rtn; }
    if (g_bltr.index_nn(2) != 6) { --rtn; }
    if (g_bltr.index_nn(3) != 7) { --rtn; }
    if (g_bltr.index_nn(4) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_nn(5) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_nn(6) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_nn(7) != std::numeric_limits<int>::max()) { --rtn; }
    //
    if (g_bltr.index_ns(0) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_ns(1) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_ns(2) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_ns(3) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_bltr.index_ns(4) != 0) { --rtn; }
    if (g_bltr.index_ns(5) != 1) { --rtn; }
    if (g_bltr.index_ns(6) != 2) { --rtn; }
    if (g_bltr.index_ns(7) != 3) { --rtn; }


    if (g_tlbr.index_nn(0) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_tlbr.index_nn(1) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_tlbr.index_nn(2) != std::numeric_limits<int>::max()) { --rtn; }
    if (g_tlbr.index_nn(3) != std::numeric_limits<int>::max()) { --rtn; }
*/
    if (g_tlbr.index_nn(4) != 0) { --rtn; } // debug just this one
    //if (g_tlbr.index_nn(5) != 1) { --rtn; }
    //if (g_tlbr.index_nn(6) != 2) { --rtn; }
    //if (g_tlbr.index_nn(7) != 3) { --rtn; }

//    std::cout << g_bltr.str() << std::endl;
//    std::cout << std::endl;
//   std::cout << g_tlbr.str() << std::endl;


    std::cout << "Return: " << rtn << std::endl;
    return rtn;
}
