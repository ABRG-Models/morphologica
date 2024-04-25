#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    morph::vec<float, 2> dx = { 1, 1 };
    morph::vec<float, 2> offset = { 0, 0 };

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    {
        //
        // morph::GridDomainWrap::None tests
        //

        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

        // Neighbour NORTH tests
        // Test in g_bltr
        if (g_bltr.index_nn(0) != 4) { --rtn; std::cout << "nn bltr 0 fails\n"; }
        if (g_bltr.index_nn(1) != 5) { --rtn; std::cout << "nn bltr 1 fails\n"; }
        if (g_bltr.index_nn(2) != 6) { --rtn; std::cout << "nn bltr 2 fails\n"; }
        if (g_bltr.index_nn(3) != 7) { --rtn; std::cout << "nn bltr 3 fails\n"; }
        if (g_bltr.index_nn(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 4 fails\n"; }
        if (g_bltr.index_nn(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 5 fails\n"; }
        if (g_bltr.index_nn(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 6 fails\n"; }
        if (g_bltr.index_nn(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nn(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 0 fails\n"; }
        if (g_tlbr.index_nn(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 1 fails\n"; }
        if (g_tlbr.index_nn(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 2 fails\n"; }
        if (g_tlbr.index_nn(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 3 fails\n"; }
        if (g_tlbr.index_nn(4) != 0) { --rtn; std::cout << "nn tlbr 4 fails\n"; }
        if (g_tlbr.index_nn(5) != 1) { --rtn; std::cout << "nn tlbr 5 fails\n"; }
        if (g_tlbr.index_nn(6) != 2) { --rtn; std::cout << "nn tlbr 6 fails\n"; }
        if (g_tlbr.index_nn(7) != 3) { --rtn; std::cout << "nn tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nn(0) != 1) { --rtn; std::cout << "nn bltrc 0 fails\n"; }
        if (g_bltrc.index_nn(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 1 fails\n"; }
        if (g_bltrc.index_nn(2) != 3) { --rtn; std::cout << "nn bltrc 2 fails\n"; }
        if (g_bltrc.index_nn(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 3 fails\n"; }
        if (g_bltrc.index_nn(4) != 5) { --rtn; std::cout << "nn bltrc 4 fails\n"; }
        if (g_bltrc.index_nn(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 5 fails\n"; }
        if (g_bltrc.index_nn(6) != 7) { --rtn; std::cout << "nn bltrc 6 fails\n"; }
        if (g_bltrc.index_nn(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nn(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nn(1) != 0) { --rtn; std::cout << "nn tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nn(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nn(3) != 2) { --rtn; std::cout << "nn tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nn(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nn(5) != 4) { --rtn; std::cout << "nn tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nn(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nn(7) != 6) { --rtn; std::cout << "nn tlbrc 7 fails\n"; }

        // Neighbour SOUTH tests
        // Test in g_bltr
        if (g_bltr.index_ns(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 0 fails\n"; }
        if (g_bltr.index_ns(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 1 fails\n"; }
        if (g_bltr.index_ns(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 2 fails\n"; }
        if (g_bltr.index_ns(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 3 fails\n"; }
        if (g_bltr.index_ns(4) != 0) { --rtn; std::cout << "ns bltr 4 fails\n"; }
        if (g_bltr.index_ns(5) != 1) { --rtn; std::cout << "ns bltr 5 fails\n"; }
        if (g_bltr.index_ns(6) != 2) { --rtn; std::cout << "ns bltr 6 fails\n"; }
        if (g_bltr.index_ns(7) != 3) { --rtn; std::cout << "ns bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ns(0) != 4) { --rtn; std::cout << "ns tlbr 0 fails\n"; }
        if (g_tlbr.index_ns(1) != 5) { --rtn; std::cout << "ns tlbr 1 fails\n"; }
        if (g_tlbr.index_ns(2) != 6) { --rtn; std::cout << "ns tlbr 2 fails\n"; }
        if (g_tlbr.index_ns(3) != 7) { --rtn; std::cout << "ns tlbr 3 fails\n"; }
        if (g_tlbr.index_ns(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 4 fails\n"; }
        if (g_tlbr.index_ns(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 5 fails\n"; }
        if (g_tlbr.index_ns(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 6 fails\n"; }
        if (g_tlbr.index_ns(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ns(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 0 fails\n"; }
        if (g_bltrc.index_ns(1) != 0) { --rtn; std::cout << "ns bltrc 1 fails\n"; }
        if (g_bltrc.index_ns(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 2 fails\n"; }
        if (g_bltrc.index_ns(3) != 2) { --rtn; std::cout << "ns bltrc 3 fails\n"; }
        if (g_bltrc.index_ns(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 4 fails\n"; }
        if (g_bltrc.index_ns(5) != 4) { --rtn; std::cout << "ns bltrc 5 fails\n"; }
        if (g_bltrc.index_ns(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 6 fails\n"; }
        if (g_bltrc.index_ns(7) != 6) { --rtn; std::cout << "ns bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ns(0) != 1) { --rtn; std::cout << "ns tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ns(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ns(2) != 3) { --rtn; std::cout << "ns tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ns(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ns(4) != 5) { --rtn; std::cout << "ns tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ns(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ns(6) != 7) { --rtn; std::cout << "ns tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ns(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 7 fails\n"; }

        // Neighbour EAST tests
        // Test in g_bltr
        if (g_bltr.index_ne(0) != 1) { --rtn; std::cout << "ne bltr 0 fails\n"; }
        if (g_bltr.index_ne(1) != 2) { --rtn; std::cout << "ne bltr 1 fails\n"; }
        if (g_bltr.index_ne(2) != 3) { --rtn; std::cout << "ne bltr 2 fails\n"; }
        if (g_bltr.index_ne(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltr 3 fails\n"; }
        if (g_bltr.index_ne(4) != 5) { --rtn; std::cout << "ne bltr 4 fails\n"; }
        if (g_bltr.index_ne(5) != 6) { --rtn; std::cout << "ne bltr 5 fails\n"; }
        if (g_bltr.index_ne(6) != 7) { --rtn; std::cout << "ne bltr 6 fails\n"; }
        if (g_bltr.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ne(0) != 1) { --rtn; std::cout << "ne tlbr 0 fails\n"; }
        if (g_tlbr.index_ne(1) != 2) { --rtn; std::cout << "ne tlbr 1 fails\n"; }
        if (g_tlbr.index_ne(2) != 3) { --rtn; std::cout << "ne tlbr 2 fails\n"; }
        if (g_tlbr.index_ne(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbr 3 fails\n"; }
        if (g_tlbr.index_ne(4) != 5) { --rtn; std::cout << "ne tlbr 4 fails\n"; }
        if (g_tlbr.index_ne(5) != 6) { --rtn; std::cout << "ne tlbr 5 fails\n"; }
        if (g_tlbr.index_ne(6) != 7) { --rtn; std::cout << "ne tlbr 6 fails\n"; }
        if (g_tlbr.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ne(0) != 2) { --rtn; std::cout << "ne bltrc 0 fails\n"; }
        if (g_bltrc.index_ne(1) != 3) { --rtn; std::cout << "ne bltrc 1 fails\n"; }
        if (g_bltrc.index_ne(2) != 4) { --rtn; std::cout << "ne bltrc 2 fails\n"; }
        if (g_bltrc.index_ne(3) != 5) { --rtn; std::cout << "ne bltrc 3 fails\n"; }
        if (g_bltrc.index_ne(4) != 6) { --rtn; std::cout << "ne bltrc 4 fails\n"; }
        if (g_bltrc.index_ne(5) != 7) { --rtn; std::cout << "ne bltrc 5 fails\n"; }
        if (g_bltrc.index_ne(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltrc 6 fails\n"; }
        if (g_bltrc.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ne(0) != 2) { --rtn; std::cout << "ne tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ne(1) != 3) { --rtn; std::cout << "ne tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ne(2) != 4) { --rtn; std::cout << "ne tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ne(3) != 5) { --rtn; std::cout << "ne tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ne(4) != 6) { --rtn; std::cout << "ne tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ne(5) != 7) { --rtn; std::cout << "ne tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ne(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbrc 7 fails\n"; }

        // Neighbour WEST tests
        // Test in g_bltr
        if (g_bltr.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltr 0 fails\n"; }
        if (g_bltr.index_nw(1) != 0) { --rtn; std::cout << "nw bltr 1 fails\n"; }
        if (g_bltr.index_nw(2) != 1) { --rtn; std::cout << "nw bltr 2 fails\n"; }
        if (g_bltr.index_nw(3) != 2) { --rtn; std::cout << "nw bltr 3 fails\n"; }
        if (g_bltr.index_nw(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltr 4 fails\n"; }
        if (g_bltr.index_nw(5) != 4) { --rtn; std::cout << "nw bltr 5 fails\n"; }
        if (g_bltr.index_nw(6) != 5) { --rtn; std::cout << "nw bltr 6 fails\n"; }
        if (g_bltr.index_nw(7) != 6) { --rtn; std::cout << "nw bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbr 0 fails\n"; }
        if (g_tlbr.index_nw(1) != 0) { --rtn; std::cout << "nw tlbr 1 fails\n"; }
        if (g_tlbr.index_nw(2) != 1) { --rtn; std::cout << "nw tlbr 2 fails\n"; }
        if (g_tlbr.index_nw(3) != 2) { --rtn; std::cout << "nw tlbr 3 fails\n"; }
        if (g_tlbr.index_nw(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbr 4 fails\n"; }
        if (g_tlbr.index_nw(5) != 4) { --rtn; std::cout << "nw tlbr 5 fails\n"; }
        if (g_tlbr.index_nw(6) != 5) { --rtn; std::cout << "nw tlbr 6 fails\n"; }
        if (g_tlbr.index_nw(7) != 6) { --rtn; std::cout << "nw tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltrc 0 fails\n"; }
        if (g_bltrc.index_nw(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltrc 1 fails\n"; }
        if (g_bltrc.index_nw(2) != 0) { --rtn; std::cout << "nw bltrc 2 fails\n"; }
        if (g_bltrc.index_nw(3) != 1) { --rtn; std::cout << "nw bltrc 3 fails\n"; }
        if (g_bltrc.index_nw(4) != 2) { --rtn; std::cout << "nw bltrc 4 fails\n"; }
        if (g_bltrc.index_nw(5) != 3) { --rtn; std::cout << "nw bltrc 5 fails\n"; }
        if (g_bltrc.index_nw(6) != 4) { --rtn; std::cout << "nw bltrc 6 fails\n"; }
        if (g_bltrc.index_nw(7) != 5) { --rtn; std::cout << "nw bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nw(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nw(2) != 0) { --rtn; std::cout << "nw tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nw(3) != 1) { --rtn; std::cout << "nw tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nw(4) != 2) { --rtn; std::cout << "nw tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nw(5) != 3) { --rtn; std::cout << "nw tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nw(6) != 4) { --rtn; std::cout << "nw tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nw(7) != 5) { --rtn; std::cout << "nw tlbrc 7 fails\n"; }
    }

    {
        //
        // morph::GridDomainWrap::Horizontal tests
        //
        wrap = morph::GridDomainWrap::Horizontal;

        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

        // Neighbour NORTH tests
        // Test in g_bltr
        if (g_bltr.index_nn(0) != 4) { --rtn; std::cout << "nn bltr 0 fails\n"; }
        if (g_bltr.index_nn(1) != 5) { --rtn; std::cout << "nn bltr 1 fails\n"; }
        if (g_bltr.index_nn(2) != 6) { --rtn; std::cout << "nn bltr 2 fails\n"; }
        if (g_bltr.index_nn(3) != 7) { --rtn; std::cout << "nn bltr 3 fails\n"; }
        if (g_bltr.index_nn(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 4 fails\n"; }
        if (g_bltr.index_nn(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 5 fails\n"; }
        if (g_bltr.index_nn(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 6 fails\n"; }
        if (g_bltr.index_nn(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nn(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 0 fails\n"; }
        if (g_tlbr.index_nn(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 1 fails\n"; }
        if (g_tlbr.index_nn(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 2 fails\n"; }
        if (g_tlbr.index_nn(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbr 3 fails\n"; }
        if (g_tlbr.index_nn(4) != 0) { --rtn; std::cout << "nn tlbr 4 fails\n"; }
        if (g_tlbr.index_nn(5) != 1) { --rtn; std::cout << "nn tlbr 5 fails\n"; }
        if (g_tlbr.index_nn(6) != 2) { --rtn; std::cout << "nn tlbr 6 fails\n"; }
        if (g_tlbr.index_nn(7) != 3) { --rtn; std::cout << "nn tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nn(0) != 1) { --rtn; std::cout << "nn bltrc 0 fails\n"; }
        if (g_bltrc.index_nn(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 1 fails\n"; }
        if (g_bltrc.index_nn(2) != 3) { --rtn; std::cout << "nn bltrc 2 fails\n"; }
        if (g_bltrc.index_nn(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 3 fails\n"; }
        if (g_bltrc.index_nn(4) != 5) { --rtn; std::cout << "nn bltrc 4 fails\n"; }
        if (g_bltrc.index_nn(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 5 fails\n"; }
        if (g_bltrc.index_nn(6) != 7) { --rtn; std::cout << "nn bltrc 6 fails\n"; }
        if (g_bltrc.index_nn(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nn(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nn(1) != 0) { --rtn; std::cout << "nn tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nn(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nn(3) != 2) { --rtn; std::cout << "nn tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nn(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nn(5) != 4) { --rtn; std::cout << "nn tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nn(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nn tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nn(7) != 6) { --rtn; std::cout << "nn tlbrc 7 fails\n"; }

        // Neighbour SOUTH tests
        // Test in g_bltr
        if (g_bltr.index_ns(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 0 fails\n"; }
        if (g_bltr.index_ns(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 1 fails\n"; }
        if (g_bltr.index_ns(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 2 fails\n"; }
        if (g_bltr.index_ns(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltr 3 fails\n"; }
        if (g_bltr.index_ns(4) != 0) { --rtn; std::cout << "ns bltr 4 fails\n"; }
        if (g_bltr.index_ns(5) != 1) { --rtn; std::cout << "ns bltr 5 fails\n"; }
        if (g_bltr.index_ns(6) != 2) { --rtn; std::cout << "ns bltr 6 fails\n"; }
        if (g_bltr.index_ns(7) != 3) { --rtn; std::cout << "ns bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ns(0) != 4) { --rtn; std::cout << "ns tlbr 0 fails\n"; }
        if (g_tlbr.index_ns(1) != 5) { --rtn; std::cout << "ns tlbr 1 fails\n"; }
        if (g_tlbr.index_ns(2) != 6) { --rtn; std::cout << "ns tlbr 2 fails\n"; }
        if (g_tlbr.index_ns(3) != 7) { --rtn; std::cout << "ns tlbr 3 fails\n"; }
        if (g_tlbr.index_ns(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 4 fails\n"; }
        if (g_tlbr.index_ns(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 5 fails\n"; }
        if (g_tlbr.index_ns(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 6 fails\n"; }
        if (g_tlbr.index_ns(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ns(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 0 fails\n"; }
        if (g_bltrc.index_ns(1) != 0) { --rtn; std::cout << "ns bltrc 1 fails\n"; }
        if (g_bltrc.index_ns(2) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 2 fails\n"; }
        if (g_bltrc.index_ns(3) != 2) { --rtn; std::cout << "ns bltrc 3 fails\n"; }
        if (g_bltrc.index_ns(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 4 fails\n"; }
        if (g_bltrc.index_ns(5) != 4) { --rtn; std::cout << "ns bltrc 5 fails\n"; }
        if (g_bltrc.index_ns(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns bltrc 6 fails\n"; }
        if (g_bltrc.index_ns(7) != 6) { --rtn; std::cout << "ns bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ns(0) != 1) { --rtn; std::cout << "ns tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ns(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ns(2) != 3) { --rtn; std::cout << "ns tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ns(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ns(4) != 5) { --rtn; std::cout << "ns tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ns(5) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ns(6) != 7) { --rtn; std::cout << "ns tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ns(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ns tlbrc 7 fails\n"; }

        // Neighbour EAST tests
        // Test in g_bltr
        if (g_bltr.index_ne(0) != 1) { --rtn; std::cout << "ne bltr 0 fails\n"; }
        if (g_bltr.index_ne(1) != 2) { --rtn; std::cout << "ne bltr 1 fails\n"; }
        if (g_bltr.index_ne(2) != 3) { --rtn; std::cout << "ne bltr 2 fails\n"; }
        if (g_bltr.index_ne(3) != 0) { --rtn; std::cout << "ne bltr 3 fails\n"; }
        if (g_bltr.index_ne(4) != 5) { --rtn; std::cout << "ne bltr 4 fails\n"; }
        if (g_bltr.index_ne(5) != 6) { --rtn; std::cout << "ne bltr 5 fails\n"; }
        if (g_bltr.index_ne(6) != 7) { --rtn; std::cout << "ne bltr 6 fails\n"; }
        if (g_bltr.index_ne(7) != 4) { --rtn; std::cout << "ne bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ne(0) != 1) { --rtn; std::cout << "ne tlbr 0 fails\n"; }
        if (g_tlbr.index_ne(1) != 2) { --rtn; std::cout << "ne tlbr 1 fails\n"; }
        if (g_tlbr.index_ne(2) != 3) { --rtn; std::cout << "ne tlbr 2 fails\n"; }
        if (g_tlbr.index_ne(3) != 0) { --rtn; std::cout << "ne tlbr 3 fails\n"; }
        if (g_tlbr.index_ne(4) != 5) { --rtn; std::cout << "ne tlbr 4 fails\n"; }
        if (g_tlbr.index_ne(5) != 6) { --rtn; std::cout << "ne tlbr 5 fails\n"; }
        if (g_tlbr.index_ne(6) != 7) { --rtn; std::cout << "ne tlbr 6 fails\n"; }
        if (g_tlbr.index_ne(7) != 4) { --rtn; std::cout << "ne tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ne(0) != 2) { --rtn; std::cout << "ne bltrc 0 fails\n"; }
        if (g_bltrc.index_ne(1) != 3) { --rtn; std::cout << "ne bltrc 1 fails\n"; }
        if (g_bltrc.index_ne(2) != 4) { --rtn; std::cout << "ne bltrc 2 fails\n"; }
        if (g_bltrc.index_ne(3) != 5) { --rtn; std::cout << "ne bltrc 3 fails\n"; }
        if (g_bltrc.index_ne(4) != 6) { --rtn; std::cout << "ne bltrc 4 fails\n"; }
        if (g_bltrc.index_ne(5) != 7) { --rtn; std::cout << "ne bltrc 5 fails\n"; }
        if (g_bltrc.index_ne(6) != 0) { --rtn; std::cout << "ne bltrc 6 fails\n"; }
        if (g_bltrc.index_ne(7) != 1) { --rtn; std::cout << "ne bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ne(0) != 2) { --rtn; std::cout << "ne tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ne(1) != 3) { --rtn; std::cout << "ne tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ne(2) != 4) { --rtn; std::cout << "ne tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ne(3) != 5) { --rtn; std::cout << "ne tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ne(4) != 6) { --rtn; std::cout << "ne tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ne(5) != 7) { --rtn; std::cout << "ne tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ne(6) != 0) { --rtn; std::cout << "ne tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ne(7) != 1) { --rtn; std::cout << "ne tlbrc 7 fails\n"; }

        // Neighbour WEST tests
        // Test in g_bltr
        if (g_bltr.index_nw(0) != 3) { --rtn; std::cout << "nw bltr 0 fails\n"; }
        if (g_bltr.index_nw(1) != 0) { --rtn; std::cout << "nw bltr 1 fails\n"; }
        if (g_bltr.index_nw(2) != 1) { --rtn; std::cout << "nw bltr 2 fails\n"; }
        if (g_bltr.index_nw(3) != 2) { --rtn; std::cout << "nw bltr 3 fails\n"; }
        if (g_bltr.index_nw(4) != 7) { --rtn; std::cout << "nw bltr 4 fails\n"; }
        if (g_bltr.index_nw(5) != 4) { --rtn; std::cout << "nw bltr 5 fails\n"; }
        if (g_bltr.index_nw(6) != 5) { --rtn; std::cout << "nw bltr 6 fails\n"; }
        if (g_bltr.index_nw(7) != 6) { --rtn; std::cout << "nw bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nw(0) != 3) { --rtn; std::cout << "nw tlbr 0 fails\n"; }
        if (g_tlbr.index_nw(1) != 0) { --rtn; std::cout << "nw tlbr 1 fails\n"; }
        if (g_tlbr.index_nw(2) != 1) { --rtn; std::cout << "nw tlbr 2 fails\n"; }
        if (g_tlbr.index_nw(3) != 2) { --rtn; std::cout << "nw tlbr 3 fails\n"; }
        if (g_tlbr.index_nw(4) != 7) { --rtn; std::cout << "nw tlbr 4 fails\n"; }
        if (g_tlbr.index_nw(5) != 4) { --rtn; std::cout << "nw tlbr 5 fails\n"; }
        if (g_tlbr.index_nw(6) != 5) { --rtn; std::cout << "nw tlbr 6 fails\n"; }
        if (g_tlbr.index_nw(7) != 6) { --rtn; std::cout << "nw tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nw(0) != 6) { --rtn; std::cout << "nw bltrc 0 fails\n"; }
        if (g_bltrc.index_nw(1) != 7) { --rtn; std::cout << "nw bltrc 1 fails\n"; }
        if (g_bltrc.index_nw(2) != 0) { --rtn; std::cout << "nw bltrc 2 fails\n"; }
        if (g_bltrc.index_nw(3) != 1) { --rtn; std::cout << "nw bltrc 3 fails\n"; }
        if (g_bltrc.index_nw(4) != 2) { --rtn; std::cout << "nw bltrc 4 fails\n"; }
        if (g_bltrc.index_nw(5) != 3) { --rtn; std::cout << "nw bltrc 5 fails\n"; }
        if (g_bltrc.index_nw(6) != 4) { --rtn; std::cout << "nw bltrc 6 fails\n"; }
        if (g_bltrc.index_nw(7) != 5) { --rtn; std::cout << "nw bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nw(0) != 6) { --rtn; std::cout << "nw tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nw(1) != 7) { --rtn; std::cout << "nw tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nw(2) != 0) { --rtn; std::cout << "nw tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nw(3) != 1) { --rtn; std::cout << "nw tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nw(4) != 2) { --rtn; std::cout << "nw tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nw(5) != 3) { --rtn; std::cout << "nw tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nw(6) != 4) { --rtn; std::cout << "nw tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nw(7) != 5) { --rtn; std::cout << "nw tlbrc 7 fails\n"; }
    }

    {
        //
        // morph::GridDomainWrap::Vertical tests
        //
        wrap = morph::GridDomainWrap::Vertical;

        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

        // Neighbour NORTH tests
        // Test in g_bltr
        if (g_bltr.index_nn(0) != 4) { --rtn; std::cout << "nn bltr 0 fails\n"; }
        if (g_bltr.index_nn(1) != 5) { --rtn; std::cout << "nn bltr 1 fails\n"; }
        if (g_bltr.index_nn(2) != 6) { --rtn; std::cout << "nn bltr 2 fails\n"; }
        if (g_bltr.index_nn(3) != 7) { --rtn; std::cout << "nn bltr 3 fails\n"; }
        if (g_bltr.index_nn(4) != 0) { --rtn; std::cout << "nn bltr 4 fails\n"; }
        if (g_bltr.index_nn(5) != 1) { --rtn; std::cout << "nn bltr 5 fails\n"; }
        if (g_bltr.index_nn(6) != 2) { --rtn; std::cout << "nn bltr 6 fails\n"; }
        if (g_bltr.index_nn(7) != 3) { --rtn; std::cout << "nn bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nn(0) != 4) { --rtn; std::cout << "nn tlbr 0 fails\n"; }
        if (g_tlbr.index_nn(1) != 5) { --rtn; std::cout << "nn tlbr 1 fails\n"; }
        if (g_tlbr.index_nn(2) != 6) { --rtn; std::cout << "nn tlbr 2 fails\n"; }
        if (g_tlbr.index_nn(3) != 7) { --rtn; std::cout << "nn tlbr 3 fails\n"; }
        if (g_tlbr.index_nn(4) != 0) { --rtn; std::cout << "nn tlbr 4 fails\n"; }
        if (g_tlbr.index_nn(5) != 1) { --rtn; std::cout << "nn tlbr 5 fails\n"; }
        if (g_tlbr.index_nn(6) != 2) { --rtn; std::cout << "nn tlbr 6 fails\n"; }
        if (g_tlbr.index_nn(7) != 3) { --rtn; std::cout << "nn tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nn(0) != 1) { --rtn; std::cout << "nn bltrc 0 fails\n"; }
        if (g_bltrc.index_nn(1) != 0) { --rtn; std::cout << "nn bltrc 1 fails\n"; }
        if (g_bltrc.index_nn(2) != 3) { --rtn; std::cout << "nn bltrc 2 fails\n"; }
        if (g_bltrc.index_nn(3) != 2) { --rtn; std::cout << "nn bltrc 3 fails\n"; }
        if (g_bltrc.index_nn(4) != 5) { --rtn; std::cout << "nn bltrc 4 fails\n"; }
        if (g_bltrc.index_nn(5) != 4) { --rtn; std::cout << "nn bltrc 5 fails\n"; }
        if (g_bltrc.index_nn(6) != 7) { --rtn; std::cout << "nn bltrc 6 fails\n"; }
        if (g_bltrc.index_nn(7) != 6) { --rtn; std::cout << "nn bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nn(0) != 1) { --rtn; std::cout << "nn tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nn(1) != 0) { --rtn; std::cout << "nn tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nn(2) != 3) { --rtn; std::cout << "nn tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nn(3) != 2) { --rtn; std::cout << "nn tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nn(4) != 5) { --rtn; std::cout << "nn tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nn(5) != 4) { --rtn; std::cout << "nn tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nn(6) != 7) { --rtn; std::cout << "nn tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nn(7) != 6) { --rtn; std::cout << "nn tlbrc 7 fails\n"; }

        // Neighbour SOUTH tests
        // Test in g_bltr
        if (g_bltr.index_ns(0) != 4) { --rtn; std::cout << "ns bltr 0 fails\n"; }
        if (g_bltr.index_ns(1) != 5) { --rtn; std::cout << "ns bltr 1 fails\n"; }
        if (g_bltr.index_ns(2) != 6) { --rtn; std::cout << "ns bltr 2 fails\n"; }
        if (g_bltr.index_ns(3) != 7) { --rtn; std::cout << "ns bltr 3 fails\n"; }
        if (g_bltr.index_ns(4) != 0) { --rtn; std::cout << "ns bltr 4 fails\n"; }
        if (g_bltr.index_ns(5) != 1) { --rtn; std::cout << "ns bltr 5 fails\n"; }
        if (g_bltr.index_ns(6) != 2) { --rtn; std::cout << "ns bltr 6 fails\n"; }
        if (g_bltr.index_ns(7) != 3) { --rtn; std::cout << "ns bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ns(0) != 4) { --rtn; std::cout << "ns tlbr 0 fails\n"; }
        if (g_tlbr.index_ns(1) != 5) { --rtn; std::cout << "ns tlbr 1 fails\n"; }
        if (g_tlbr.index_ns(2) != 6) { --rtn; std::cout << "ns tlbr 2 fails\n"; }
        if (g_tlbr.index_ns(3) != 7) { --rtn; std::cout << "ns tlbr 3 fails\n"; }
        if (g_tlbr.index_ns(4) != 0) { --rtn; std::cout << "ns tlbr 4 fails\n"; }
        if (g_tlbr.index_ns(5) != 1) { --rtn; std::cout << "ns tlbr 5 fails\n"; }
        if (g_tlbr.index_ns(6) != 2) { --rtn; std::cout << "ns tlbr 6 fails\n"; }
        if (g_tlbr.index_ns(7) != 3) { --rtn; std::cout << "ns tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ns(0) != 1) { --rtn; std::cout << "ns bltrc 0 fails\n"; }
        if (g_bltrc.index_ns(1) != 0) { --rtn; std::cout << "ns bltrc 1 fails\n"; }
        if (g_bltrc.index_ns(2) != 3) { --rtn; std::cout << "ns bltrc 2 fails\n"; }
        if (g_bltrc.index_ns(3) != 2) { --rtn; std::cout << "ns bltrc 3 fails\n"; }
        if (g_bltrc.index_ns(4) != 5) { --rtn; std::cout << "ns bltrc 4 fails\n"; }
        if (g_bltrc.index_ns(5) != 4) { --rtn; std::cout << "ns bltrc 5 fails\n"; }
        if (g_bltrc.index_ns(6) != 7) { --rtn; std::cout << "ns bltrc 6 fails\n"; }
        if (g_bltrc.index_ns(7) != 6) { --rtn; std::cout << "ns bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ns(0) != 1) { --rtn; std::cout << "ns tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ns(1) != 0) { --rtn; std::cout << "ns tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ns(2) != 3) { --rtn; std::cout << "ns tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ns(3) != 2) { --rtn; std::cout << "ns tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ns(4) != 5) { --rtn; std::cout << "ns tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ns(5) != 4) { --rtn; std::cout << "ns tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ns(6) != 7) { --rtn; std::cout << "ns tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ns(7) != 6) { --rtn; std::cout << "ns tlbrc 7 fails\n"; }

        // Neighbour EAST tests
        // Test in g_bltr
        if (g_bltr.index_ne(0) != 1) { --rtn; std::cout << "ne bltr 0 fails\n"; }
        if (g_bltr.index_ne(1) != 2) { --rtn; std::cout << "ne bltr 1 fails\n"; }
        if (g_bltr.index_ne(2) != 3) { --rtn; std::cout << "ne bltr 2 fails\n"; }
        if (g_bltr.index_ne(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltr 3 fails\n"; }
        if (g_bltr.index_ne(4) != 5) { --rtn; std::cout << "ne bltr 4 fails\n"; }
        if (g_bltr.index_ne(5) != 6) { --rtn; std::cout << "ne bltr 5 fails\n"; }
        if (g_bltr.index_ne(6) != 7) { --rtn; std::cout << "ne bltr 6 fails\n"; }
        if (g_bltr.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ne(0) != 1) { --rtn; std::cout << "ne tlbr 0 fails\n"; }
        if (g_tlbr.index_ne(1) != 2) { --rtn; std::cout << "ne tlbr 1 fails\n"; }
        if (g_tlbr.index_ne(2) != 3) { --rtn; std::cout << "ne tlbr 2 fails\n"; }
        if (g_tlbr.index_ne(3) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbr 3 fails\n"; }
        if (g_tlbr.index_ne(4) != 5) { --rtn; std::cout << "ne tlbr 4 fails\n"; }
        if (g_tlbr.index_ne(5) != 6) { --rtn; std::cout << "ne tlbr 5 fails\n"; }
        if (g_tlbr.index_ne(6) != 7) { --rtn; std::cout << "ne tlbr 6 fails\n"; }
        if (g_tlbr.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ne(0) != 2) { --rtn; std::cout << "ne bltrc 0 fails\n"; }
        if (g_bltrc.index_ne(1) != 3) { --rtn; std::cout << "ne bltrc 1 fails\n"; }
        if (g_bltrc.index_ne(2) != 4) { --rtn; std::cout << "ne bltrc 2 fails\n"; }
        if (g_bltrc.index_ne(3) != 5) { --rtn; std::cout << "ne bltrc 3 fails\n"; }
        if (g_bltrc.index_ne(4) != 6) { --rtn; std::cout << "ne bltrc 4 fails\n"; }
        if (g_bltrc.index_ne(5) != 7) { --rtn; std::cout << "ne bltrc 5 fails\n"; }
        if (g_bltrc.index_ne(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltrc 6 fails\n"; }
        if (g_bltrc.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ne(0) != 2) { --rtn; std::cout << "ne tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ne(1) != 3) { --rtn; std::cout << "ne tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ne(2) != 4) { --rtn; std::cout << "ne tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ne(3) != 5) { --rtn; std::cout << "ne tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ne(4) != 6) { --rtn; std::cout << "ne tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ne(5) != 7) { --rtn; std::cout << "ne tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ne(6) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ne(7) != std::numeric_limits<int>::max()) { --rtn; std::cout << "ne tlbrc 7 fails\n"; }

        // Neighbour WEST tests
        // Test in g_bltr
        if (g_bltr.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltr 0 fails\n"; }
        if (g_bltr.index_nw(1) != 0) { --rtn; std::cout << "nw bltr 1 fails\n"; }
        if (g_bltr.index_nw(2) != 1) { --rtn; std::cout << "nw bltr 2 fails\n"; }
        if (g_bltr.index_nw(3) != 2) { --rtn; std::cout << "nw bltr 3 fails\n"; }
        if (g_bltr.index_nw(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltr 4 fails\n"; }
        if (g_bltr.index_nw(5) != 4) { --rtn; std::cout << "nw bltr 5 fails\n"; }
        if (g_bltr.index_nw(6) != 5) { --rtn; std::cout << "nw bltr 6 fails\n"; }
        if (g_bltr.index_nw(7) != 6) { --rtn; std::cout << "nw bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbr 0 fails\n"; }
        if (g_tlbr.index_nw(1) != 0) { --rtn; std::cout << "nw tlbr 1 fails\n"; }
        if (g_tlbr.index_nw(2) != 1) { --rtn; std::cout << "nw tlbr 2 fails\n"; }
        if (g_tlbr.index_nw(3) != 2) { --rtn; std::cout << "nw tlbr 3 fails\n"; }
        if (g_tlbr.index_nw(4) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbr 4 fails\n"; }
        if (g_tlbr.index_nw(5) != 4) { --rtn; std::cout << "nw tlbr 5 fails\n"; }
        if (g_tlbr.index_nw(6) != 5) { --rtn; std::cout << "nw tlbr 6 fails\n"; }
        if (g_tlbr.index_nw(7) != 6) { --rtn; std::cout << "nw tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltrc 0 fails\n"; }
        if (g_bltrc.index_nw(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw bltrc 1 fails\n"; }
        if (g_bltrc.index_nw(2) != 0) { --rtn; std::cout << "nw bltrc 2 fails\n"; }
        if (g_bltrc.index_nw(3) != 1) { --rtn; std::cout << "nw bltrc 3 fails\n"; }
        if (g_bltrc.index_nw(4) != 2) { --rtn; std::cout << "nw bltrc 4 fails\n"; }
        if (g_bltrc.index_nw(5) != 3) { --rtn; std::cout << "nw bltrc 5 fails\n"; }
        if (g_bltrc.index_nw(6) != 4) { --rtn; std::cout << "nw bltrc 6 fails\n"; }
        if (g_bltrc.index_nw(7) != 5) { --rtn; std::cout << "nw bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nw(0) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nw(1) != std::numeric_limits<int>::max()) { --rtn; std::cout << "nw tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nw(2) != 0) { --rtn; std::cout << "nw tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nw(3) != 1) { --rtn; std::cout << "nw tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nw(4) != 2) { --rtn; std::cout << "nw tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nw(5) != 3) { --rtn; std::cout << "nw tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nw(6) != 4) { --rtn; std::cout << "nw tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nw(7) != 5) { --rtn; std::cout << "nw tlbrc 7 fails\n"; }
    }

    {
        //
        // morph::GridDomainWrap::Both tests
        //
        wrap = morph::GridDomainWrap::Both;

        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

        // Neighbour NORTH tests
        // Test in g_bltr
        if (g_bltr.index_nn(0) != 4) { --rtn; std::cout << "nn bltr 0 fails\n"; }
        if (g_bltr.index_nn(1) != 5) { --rtn; std::cout << "nn bltr 1 fails\n"; }
        if (g_bltr.index_nn(2) != 6) { --rtn; std::cout << "nn bltr 2 fails\n"; }
        if (g_bltr.index_nn(3) != 7) { --rtn; std::cout << "nn bltr 3 fails\n"; }
        if (g_bltr.index_nn(4) != 0) { --rtn; std::cout << "nn bltr 4 fails\n"; }
        if (g_bltr.index_nn(5) != 1) { --rtn; std::cout << "nn bltr 5 fails\n"; }
        if (g_bltr.index_nn(6) != 2) { --rtn; std::cout << "nn bltr 6 fails\n"; }
        if (g_bltr.index_nn(7) != 3) { --rtn; std::cout << "nn bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nn(0) != 4) { --rtn; std::cout << "nn tlbr 0 fails\n"; }
        if (g_tlbr.index_nn(1) != 5) { --rtn; std::cout << "nn tlbr 1 fails\n"; }
        if (g_tlbr.index_nn(2) != 6) { --rtn; std::cout << "nn tlbr 2 fails\n"; }
        if (g_tlbr.index_nn(3) != 7) { --rtn; std::cout << "nn tlbr 3 fails\n"; }
        if (g_tlbr.index_nn(4) != 0) { --rtn; std::cout << "nn tlbr 4 fails\n"; }
        if (g_tlbr.index_nn(5) != 1) { --rtn; std::cout << "nn tlbr 5 fails\n"; }
        if (g_tlbr.index_nn(6) != 2) { --rtn; std::cout << "nn tlbr 6 fails\n"; }
        if (g_tlbr.index_nn(7) != 3) { --rtn; std::cout << "nn tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nn(0) != 1) { --rtn; std::cout << "nn bltrc 0 fails\n"; }
        if (g_bltrc.index_nn(1) != 0) { --rtn; std::cout << "nn bltrc 1 fails\n"; }
        if (g_bltrc.index_nn(2) != 3) { --rtn; std::cout << "nn bltrc 2 fails\n"; }
        if (g_bltrc.index_nn(3) != 2) { --rtn; std::cout << "nn bltrc 3 fails\n"; }
        if (g_bltrc.index_nn(4) != 5) { --rtn; std::cout << "nn bltrc 4 fails\n"; }
        if (g_bltrc.index_nn(5) != 4) { --rtn; std::cout << "nn bltrc 5 fails\n"; }
        if (g_bltrc.index_nn(6) != 7) { --rtn; std::cout << "nn bltrc 6 fails\n"; }
        if (g_bltrc.index_nn(7) != 6) { --rtn; std::cout << "nn bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nn(0) != 1) { --rtn; std::cout << "nn tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nn(1) != 0) { --rtn; std::cout << "nn tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nn(2) != 3) { --rtn; std::cout << "nn tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nn(3) != 2) { --rtn; std::cout << "nn tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nn(4) != 5) { --rtn; std::cout << "nn tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nn(5) != 4) { --rtn; std::cout << "nn tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nn(6) != 7) { --rtn; std::cout << "nn tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nn(7) != 6) { --rtn; std::cout << "nn tlbrc 7 fails\n"; }

        // Neighbour SOUTH tests
        // Test in g_bltr
        if (g_bltr.index_ns(0) != 4) { --rtn; std::cout << "ns bltr 0 fails\n"; }
        if (g_bltr.index_ns(1) != 5) { --rtn; std::cout << "ns bltr 1 fails\n"; }
        if (g_bltr.index_ns(2) != 6) { --rtn; std::cout << "ns bltr 2 fails\n"; }
        if (g_bltr.index_ns(3) != 7) { --rtn; std::cout << "ns bltr 3 fails\n"; }
        if (g_bltr.index_ns(4) != 0) { --rtn; std::cout << "ns bltr 4 fails\n"; }
        if (g_bltr.index_ns(5) != 1) { --rtn; std::cout << "ns bltr 5 fails\n"; }
        if (g_bltr.index_ns(6) != 2) { --rtn; std::cout << "ns bltr 6 fails\n"; }
        if (g_bltr.index_ns(7) != 3) { --rtn; std::cout << "ns bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ns(0) != 4) { --rtn; std::cout << "ns tlbr 0 fails\n"; }
        if (g_tlbr.index_ns(1) != 5) { --rtn; std::cout << "ns tlbr 1 fails\n"; }
        if (g_tlbr.index_ns(2) != 6) { --rtn; std::cout << "ns tlbr 2 fails\n"; }
        if (g_tlbr.index_ns(3) != 7) { --rtn; std::cout << "ns tlbr 3 fails\n"; }
        if (g_tlbr.index_ns(4) != 0) { --rtn; std::cout << "ns tlbr 4 fails\n"; }
        if (g_tlbr.index_ns(5) != 1) { --rtn; std::cout << "ns tlbr 5 fails\n"; }
        if (g_tlbr.index_ns(6) != 2) { --rtn; std::cout << "ns tlbr 6 fails\n"; }
        if (g_tlbr.index_ns(7) != 3) { --rtn; std::cout << "ns tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ns(0) != 1) { --rtn; std::cout << "ns bltrc 0 fails\n"; }
        if (g_bltrc.index_ns(1) != 0) { --rtn; std::cout << "ns bltrc 1 fails\n"; }
        if (g_bltrc.index_ns(2) != 3) { --rtn; std::cout << "ns bltrc 2 fails\n"; }
        if (g_bltrc.index_ns(3) != 2) { --rtn; std::cout << "ns bltrc 3 fails\n"; }
        if (g_bltrc.index_ns(4) != 5) { --rtn; std::cout << "ns bltrc 4 fails\n"; }
        if (g_bltrc.index_ns(5) != 4) { --rtn; std::cout << "ns bltrc 5 fails\n"; }
        if (g_bltrc.index_ns(6) != 7) { --rtn; std::cout << "ns bltrc 6 fails\n"; }
        if (g_bltrc.index_ns(7) != 6) { --rtn; std::cout << "ns bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ns(0) != 1) { --rtn; std::cout << "ns tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ns(1) != 0) { --rtn; std::cout << "ns tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ns(2) != 3) { --rtn; std::cout << "ns tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ns(3) != 2) { --rtn; std::cout << "ns tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ns(4) != 5) { --rtn; std::cout << "ns tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ns(5) != 4) { --rtn; std::cout << "ns tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ns(6) != 7) { --rtn; std::cout << "ns tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ns(7) != 6) { --rtn; std::cout << "ns tlbrc 7 fails\n"; }

        // Neighbour EAST tests
        // Test in g_bltr
        if (g_bltr.index_ne(0) != 1) { --rtn; std::cout << "ne bltr 0 fails\n"; }
        if (g_bltr.index_ne(1) != 2) { --rtn; std::cout << "ne bltr 1 fails\n"; }
        if (g_bltr.index_ne(2) != 3) { --rtn; std::cout << "ne bltr 2 fails\n"; }
        if (g_bltr.index_ne(3) != 0) { --rtn; std::cout << "ne bltr 3 fails\n"; }
        if (g_bltr.index_ne(4) != 5) { --rtn; std::cout << "ne bltr 4 fails\n"; }
        if (g_bltr.index_ne(5) != 6) { --rtn; std::cout << "ne bltr 5 fails\n"; }
        if (g_bltr.index_ne(6) != 7) { --rtn; std::cout << "ne bltr 6 fails\n"; }
        if (g_bltr.index_ne(7) != 4) { --rtn; std::cout << "ne bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_ne(0) != 1) { --rtn; std::cout << "ne tlbr 0 fails\n"; }
        if (g_tlbr.index_ne(1) != 2) { --rtn; std::cout << "ne tlbr 1 fails\n"; }
        if (g_tlbr.index_ne(2) != 3) { --rtn; std::cout << "ne tlbr 2 fails\n"; }
        if (g_tlbr.index_ne(3) != 0) { --rtn; std::cout << "ne tlbr 3 fails\n"; }
        if (g_tlbr.index_ne(4) != 5) { --rtn; std::cout << "ne tlbr 4 fails\n"; }
        if (g_tlbr.index_ne(5) != 6) { --rtn; std::cout << "ne tlbr 5 fails\n"; }
        if (g_tlbr.index_ne(6) != 7) { --rtn; std::cout << "ne tlbr 6 fails\n"; }
        if (g_tlbr.index_ne(7) != 4) { --rtn; std::cout << "ne tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_ne(0) != 2) { --rtn; std::cout << "ne bltrc 0 fails\n"; }
        if (g_bltrc.index_ne(1) != 3) { --rtn; std::cout << "ne bltrc 1 fails\n"; }
        if (g_bltrc.index_ne(2) != 4) { --rtn; std::cout << "ne bltrc 2 fails\n"; }
        if (g_bltrc.index_ne(3) != 5) { --rtn; std::cout << "ne bltrc 3 fails\n"; }
        if (g_bltrc.index_ne(4) != 6) { --rtn; std::cout << "ne bltrc 4 fails\n"; }
        if (g_bltrc.index_ne(5) != 7) { --rtn; std::cout << "ne bltrc 5 fails\n"; }
        if (g_bltrc.index_ne(6) != 0) { --rtn; std::cout << "ne bltrc 6 fails\n"; }
        if (g_bltrc.index_ne(7) != 1) { --rtn; std::cout << "ne bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_ne(0) != 2) { --rtn; std::cout << "ne tlbrc 0 fails\n"; }
        if (g_tlbrc.index_ne(1) != 3) { --rtn; std::cout << "ne tlbrc 1 fails\n"; }
        if (g_tlbrc.index_ne(2) != 4) { --rtn; std::cout << "ne tlbrc 2 fails\n"; }
        if (g_tlbrc.index_ne(3) != 5) { --rtn; std::cout << "ne tlbrc 3 fails\n"; }
        if (g_tlbrc.index_ne(4) != 6) { --rtn; std::cout << "ne tlbrc 4 fails\n"; }
        if (g_tlbrc.index_ne(5) != 7) { --rtn; std::cout << "ne tlbrc 5 fails\n"; }
        if (g_tlbrc.index_ne(6) != 0) { --rtn; std::cout << "ne tlbrc 6 fails\n"; }
        if (g_tlbrc.index_ne(7) != 1) { --rtn; std::cout << "ne tlbrc 7 fails\n"; }

        // Neighbour WEST tests
        // Test in g_bltr
        if (g_bltr.index_nw(0) != 3) { --rtn; std::cout << "nw bltr 0 fails\n"; }
        if (g_bltr.index_nw(1) != 0) { --rtn; std::cout << "nw bltr 1 fails\n"; }
        if (g_bltr.index_nw(2) != 1) { --rtn; std::cout << "nw bltr 2 fails\n"; }
        if (g_bltr.index_nw(3) != 2) { --rtn; std::cout << "nw bltr 3 fails\n"; }
        if (g_bltr.index_nw(4) != 7) { --rtn; std::cout << "nw bltr 4 fails\n"; }
        if (g_bltr.index_nw(5) != 4) { --rtn; std::cout << "nw bltr 5 fails\n"; }
        if (g_bltr.index_nw(6) != 5) { --rtn; std::cout << "nw bltr 6 fails\n"; }
        if (g_bltr.index_nw(7) != 6) { --rtn; std::cout << "nw bltr 7 fails\n"; }
        // Test in g_tlbr
        if (g_tlbr.index_nw(0) != 3) { --rtn; std::cout << "nw tlbr 0 fails\n"; }
        if (g_tlbr.index_nw(1) != 0) { --rtn; std::cout << "nw tlbr 1 fails\n"; }
        if (g_tlbr.index_nw(2) != 1) { --rtn; std::cout << "nw tlbr 2 fails\n"; }
        if (g_tlbr.index_nw(3) != 2) { --rtn; std::cout << "nw tlbr 3 fails\n"; }
        if (g_tlbr.index_nw(4) != 7) { --rtn; std::cout << "nw tlbr 4 fails\n"; }
        if (g_tlbr.index_nw(5) != 4) { --rtn; std::cout << "nw tlbr 5 fails\n"; }
        if (g_tlbr.index_nw(6) != 5) { --rtn; std::cout << "nw tlbr 6 fails\n"; }
        if (g_tlbr.index_nw(7) != 6) { --rtn; std::cout << "nw tlbr 7 fails\n"; }
        // Test in g_bltrc
        if (g_bltrc.index_nw(0) != 6) { --rtn; std::cout << "nw bltrc 0 fails\n"; }
        if (g_bltrc.index_nw(1) != 7) { --rtn; std::cout << "nw bltrc 1 fails\n"; }
        if (g_bltrc.index_nw(2) != 0) { --rtn; std::cout << "nw bltrc 2 fails\n"; }
        if (g_bltrc.index_nw(3) != 1) { --rtn; std::cout << "nw bltrc 3 fails\n"; }
        if (g_bltrc.index_nw(4) != 2) { --rtn; std::cout << "nw bltrc 4 fails\n"; }
        if (g_bltrc.index_nw(5) != 3) { --rtn; std::cout << "nw bltrc 5 fails\n"; }
        if (g_bltrc.index_nw(6) != 4) { --rtn; std::cout << "nw bltrc 6 fails\n"; }
        if (g_bltrc.index_nw(7) != 5) { --rtn; std::cout << "nw bltrc 7 fails\n"; }
        // Test in g_tlbrc
        if (g_tlbrc.index_nw(0) != 6) { --rtn; std::cout << "nw tlbrc 0 fails\n"; }
        if (g_tlbrc.index_nw(1) != 7) { --rtn; std::cout << "nw tlbrc 1 fails\n"; }
        if (g_tlbrc.index_nw(2) != 0) { --rtn; std::cout << "nw tlbrc 2 fails\n"; }
        if (g_tlbrc.index_nw(3) != 1) { --rtn; std::cout << "nw tlbrc 3 fails\n"; }
        if (g_tlbrc.index_nw(4) != 2) { --rtn; std::cout << "nw tlbrc 4 fails\n"; }
        if (g_tlbrc.index_nw(5) != 3) { --rtn; std::cout << "nw tlbrc 5 fails\n"; }
        if (g_tlbrc.index_nw(6) != 4) { --rtn; std::cout << "nw tlbrc 6 fails\n"; }
        if (g_tlbrc.index_nw(7) != 5) { --rtn; std::cout << "nw tlbrc 7 fails\n"; }
    }

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
