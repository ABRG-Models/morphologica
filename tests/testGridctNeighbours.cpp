#include "morph/Gridct.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    constexpr morph::vec<float, 2> dx = { 1, 1 };
    constexpr morph::vec<float, 2> offset = { 0, 0 };
    constexpr bool with_memory = true;

    {
        //
        // morph::GridDomainWrap::None tests
        //
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::None, morph::GridOrder::bottomleft_to_topright> g_bltr;
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::None, morph::GridOrder::topleft_to_bottomright> g_tlbr;

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
    }

    {
        //
        // morph::GridDomainWrap::Horizontal tests
        //

        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Horizontal, morph::GridOrder::bottomleft_to_topright> g_bltr;
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Horizontal, morph::GridOrder::topleft_to_bottomright> g_tlbr;

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
    }

    {
        //
        // morph::GridDomainWrap::Vertical tests
        //
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Vertical, morph::GridOrder::bottomleft_to_topright> g_bltr;
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Vertical, morph::GridOrder::topleft_to_bottomright> g_tlbr;

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
    }

    {
        //
        // morph::GridDomainWrap::Both tests
        //
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Both, morph::GridOrder::bottomleft_to_topright> g_bltr;
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::Both, morph::GridOrder::topleft_to_bottomright> g_tlbr;

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
    }

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
