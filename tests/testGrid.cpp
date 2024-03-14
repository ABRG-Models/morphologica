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
    morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
    morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

    std::cout << "Grid g_bltr extents: " << g_bltr.extents() << std::endl;
    std::cout << "Grid g_bltrc extents: " << g_bltrc.extents() << std::endl;

    std::cout << "Grid g_tlbr extents: " << g_tlbr.extents() << std::endl;
    std::cout << "Grid g_tlbrc extents: " << g_tlbrc.extents() << std::endl;

    // BLTR extents should be same whether row or col major
    if (g_bltr.extents() != g_bltrc.extents()
        || g_tlbr.extents() != g_tlbrc.extents()) {
        --rtn;
    }

    std::cout << "Grid centre: " << g_bltr.centre() << std::endl;
    std::cout << "Grid centre: " << g_bltrc.centre() << std::endl;
    std::cout << "Grid centre: " << g_tlbr.centre() << std::endl;
    std::cout << "Grid centre: " << g_tlbrc.centre() << std::endl;

    if (g_bltr.centre() != g_bltrc.centre()
        || g_bltr.centre() != g_tlbr.centre()
        || g_bltr.centre() != g_tlbrc.centre()) {
        --rtn;
    }

    // Show coords for bltr columnar (inspect these)
    std::cout << "BLTR colmaj:\n";
    for (int i = 1; i < 8; i+=2) {
        std::cout << g_bltrc[i] << ", ";
    }
    std::cout << std::endl;
    for (int i = 0; i < 8; i+=2) {
        std::cout << g_bltrc[i] << ", ";
    }
    std::cout << std::endl;

    std::cout << "BLTR rowmaj:\n";
    for (int i = 4; i < 8; i+=1) {
        std::cout << g_bltr[i] << ", ";
    }
    std::cout << std::endl;
    for (int i = 0; i < 4; i+=1) {
        std::cout << g_bltr[i] << ", ";
    }
    std::cout << std::endl;

    std::cout << "TLBR rowmaj:\n";
    for (int i = 0; i < 4; i+=1) {
        std::cout << g_tlbr[i] << ", ";
    }
    std::cout << std::endl;
    for (int i = 4; i < 8; i+=1) {
        std::cout << g_tlbr[i] << ", ";
    }
    std::cout << std::endl;

    std::cout << "TLBR colmaj:\n";
    for (int i = 0; i < 8; i+=2) {
        std::cout << g_tlbrc[i] << ", ";
    }
    std::cout << std::endl;
    for (int i = 1; i < 8; i+=2) {
        std::cout << g_tlbrc[i] << ", ";
    }
    std::cout << std::endl;

    return rtn;
}
