#include "morph/Gridct.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    constexpr morph::vec<float, 2> dx = { 1, 1 };
    constexpr morph::vec<float, 2> offset = { 0, 0 };
    constexpr bool with_memory = true;

    std::cout << "WRAP NONE\n--------------------\n";
    {
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::None, morph::GridOrder::bottomleft_to_topright> g_bltr;
        morph::Gridct<int, float, 4, 2, dx, offset, with_memory, morph::GridDomainWrap::None, morph::GridOrder::topleft_to_bottomright> g_tlbr;

        std::cout << "ROW Bottom left top right...\n";
        if (g_bltr.row(0) != 0) { --rtn; std::cout << "bltr row(0) fails\n"; }
        if (g_bltr.row(1) != 0) { --rtn; std::cout << "bltr row(1) fails\n"; }
        if (g_bltr.row(2) != 0) { --rtn; std::cout << "bltr row(2) fails\n"; }
        if (g_bltr.row(3) != 0) { --rtn; std::cout << "bltr row(3) fails\n"; }
        if (g_bltr.row(4) != 1) { --rtn; std::cout << "bltr row(4) fails\n"; }
        if (g_bltr.row(5) != 1) { --rtn; std::cout << "bltr row(5) fails\n"; }
        if (g_bltr.row(6) != 1) { --rtn; std::cout << "bltr row(6) fails\n"; }
        if (g_bltr.row(7) != 1) { --rtn; std::cout << "bltr row(7) fails\n"; }
        std::cout << "ROW Top left bottom right...\n";
        if (g_tlbr.row(0) != 0) { --rtn; std::cout << "tlbr row(0) fails\n"; }
        if (g_tlbr.row(1) != 0) { --rtn; std::cout << "tlbr row(1) fails\n"; }
        if (g_tlbr.row(2) != 0) { --rtn; std::cout << "tlbr row(2) fails\n"; }
        if (g_tlbr.row(3) != 0) { --rtn; std::cout << "tlbr row(3) fails\n"; }
        if (g_tlbr.row(4) != 1) { --rtn; std::cout << "tlbr row(4) fails\n"; }
        if (g_tlbr.row(5) != 1) { --rtn; std::cout << "tlbr row(5) fails\n"; }
        if (g_tlbr.row(6) != 1) { --rtn; std::cout << "tlbr row(6) fails\n"; }
        if (g_tlbr.row(7) != 1) { --rtn; std::cout << "tlbr row(7) fails\n"; }

        std::cout << "COL Bottom left top right...\n";
        if (g_bltr.col(0) != 0) { --rtn; std::cout << "bltr col(0) fails\n"; }
        if (g_bltr.col(1) != 1) { --rtn; std::cout << "bltr col(1) fails\n"; }
        if (g_bltr.col(2) != 2) { --rtn; std::cout << "bltr col(2) fails\n"; }
        if (g_bltr.col(3) != 3) { --rtn; std::cout << "bltr col(3) fails\n"; }
        if (g_bltr.col(4) != 0) { --rtn; std::cout << "bltr col(4) fails\n"; }
        if (g_bltr.col(5) != 1) { --rtn; std::cout << "bltr col(5) fails\n"; }
        if (g_bltr.col(6) != 2) { --rtn; std::cout << "bltr col(6) fails\n"; }
        if (g_bltr.col(7) != 3) { --rtn; std::cout << "bltr col(7) fails\n"; }
        std::cout << "COL Top left bottom right...\n";
        if (g_tlbr.col(0) != 0) { --rtn; std::cout << "tlbr col(0) fails\n"; }
        if (g_tlbr.col(1) != 1) { --rtn; std::cout << "tlbr col(1) fails\n"; }
        if (g_tlbr.col(2) != 2) { --rtn; std::cout << "tlbr col(2) fails\n"; }
        if (g_tlbr.col(3) != 3) { --rtn; std::cout << "tlbr col(3) fails\n"; }
        if (g_tlbr.col(4) != 0) { --rtn; std::cout << "tlbr col(4) fails\n"; }
        if (g_tlbr.col(5) != 1) { --rtn; std::cout << "tlbr col(5) fails\n"; }
        if (g_tlbr.col(6) != 2) { --rtn; std::cout << "tlbr col(6) fails\n"; }
        if (g_tlbr.col(7) != 3) { --rtn; std::cout << "tlbr col(7) fails\n"; }
    }


    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
