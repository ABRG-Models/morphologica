#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    morph::vec<float, 2> dx = { 1, 1 };
    morph::vec<float, 2> offset = { 0, 0 };

    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;

    //
    // morph::GridDomainWrap::None tests
    //
    std::cout << "WRAP NONE\n--------------------\n";
    {
        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

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
        std::cout << "ROW Bottom left top right column major...\n";
        if (g_bltrc.row(0) != 0) { --rtn; std::cout << "bltrc row(0) fails\n"; }
        if (g_bltrc.row(1) != 1) { --rtn; std::cout << "bltrc row(1) fails\n"; }
        if (g_bltrc.row(2) != 0) { --rtn; std::cout << "bltrc row(2) fails\n"; }
        if (g_bltrc.row(3) != 1) { --rtn; std::cout << "bltrc row(3) fails\n"; }
        if (g_bltrc.row(4) != 0) { --rtn; std::cout << "bltrc row(4) fails\n"; }
        if (g_bltrc.row(5) != 1) { --rtn; std::cout << "bltrc row(5) fails\n"; }
        if (g_bltrc.row(6) != 0) { --rtn; std::cout << "bltrc row(6) fails\n"; }
        if (g_bltrc.row(7) != 1) { --rtn; std::cout << "bltrc row(7) fails\n"; }
        std::cout << "ROW Top left bottom right column major...\n";
        if (g_tlbrc.row(0) != 0) { --rtn; std::cout << "tlbrc row(0) fails\n"; }
        if (g_tlbrc.row(1) != 1) { --rtn; std::cout << "tlbrc row(1) fails\n"; }
        if (g_tlbrc.row(2) != 0) { --rtn; std::cout << "tlbrc row(2) fails\n"; }
        if (g_tlbrc.row(3) != 1) { --rtn; std::cout << "tlbrc row(3) fails\n"; }
        if (g_tlbrc.row(4) != 0) { --rtn; std::cout << "tlbrc row(4) fails\n"; }
        if (g_tlbrc.row(5) != 1) { --rtn; std::cout << "tlbrc row(5) fails\n"; }
        if (g_tlbrc.row(6) != 0) { --rtn; std::cout << "tlbrc row(6) fails\n"; }
        if (g_tlbrc.row(7) != 1) { --rtn; std::cout << "tlbrc row(7) fails\n"; }

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
        std::cout << "COL Bottom left top right column major...\n";
        if (g_bltrc.col(0) != 0) { --rtn; std::cout << "bltrc col(0) fails\n"; }
        if (g_bltrc.col(1) != 0) { --rtn; std::cout << "bltrc col(1) fails\n"; }
        if (g_bltrc.col(2) != 1) { --rtn; std::cout << "bltrc col(2) fails\n"; }
        if (g_bltrc.col(3) != 1) { --rtn; std::cout << "bltrc col(3) fails\n"; }
        if (g_bltrc.col(4) != 2) { --rtn; std::cout << "bltrc col(4) fails\n"; }
        if (g_bltrc.col(5) != 2) { --rtn; std::cout << "bltrc col(5) fails\n"; }
        if (g_bltrc.col(6) != 3) { --rtn; std::cout << "bltrc col(6) fails\n"; }
        if (g_bltrc.col(7) != 3) { --rtn; std::cout << "bltrc col(7) fails\n"; }
        std::cout << "COL Top left bottom right column major...\n";
        if (g_tlbrc.col(0) != 0) { --rtn; std::cout << "tlbrc col(0) fails\n"; }
        if (g_tlbrc.col(1) != 0) { --rtn; std::cout << "tlbrc col(1) fails\n"; }
        if (g_tlbrc.col(2) != 1) { --rtn; std::cout << "tlbrc col(2) fails\n"; }
        if (g_tlbrc.col(3) != 1) { --rtn; std::cout << "tlbrc col(3) fails\n"; }
        if (g_tlbrc.col(4) != 2) { --rtn; std::cout << "tlbrc col(4) fails\n"; }
        if (g_tlbrc.col(5) != 2) { --rtn; std::cout << "tlbrc col(5) fails\n"; }
        if (g_tlbrc.col(6) != 3) { --rtn; std::cout << "tlbrc col(6) fails\n"; }
        if (g_tlbrc.col(7) != 3) { --rtn; std::cout << "tlbrc col(7) fails\n"; }
    }

    //
    // morph::GridDomainWrap::Horzontal tests
    //
    wrap = morph::GridDomainWrap::Horizontal;
    std::cout << "WRAP HORIZONTAL\n--------------------\n";
    {
        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

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
        std::cout << "ROW Bottom left top right column major...\n";
        if (g_bltrc.row(0) != 0) { --rtn; std::cout << "bltrc row(0) fails\n"; }
        if (g_bltrc.row(1) != 1) { --rtn; std::cout << "bltrc row(1) fails\n"; }
        if (g_bltrc.row(2) != 0) { --rtn; std::cout << "bltrc row(2) fails\n"; }
        if (g_bltrc.row(3) != 1) { --rtn; std::cout << "bltrc row(3) fails\n"; }
        if (g_bltrc.row(4) != 0) { --rtn; std::cout << "bltrc row(4) fails\n"; }
        if (g_bltrc.row(5) != 1) { --rtn; std::cout << "bltrc row(5) fails\n"; }
        if (g_bltrc.row(6) != 0) { --rtn; std::cout << "bltrc row(6) fails\n"; }
        if (g_bltrc.row(7) != 1) { --rtn; std::cout << "bltrc row(7) fails\n"; }
        std::cout << "ROW Top left bottom right column major...\n";
        if (g_tlbrc.row(0) != 0) { --rtn; std::cout << "tlbrc row(0) fails\n"; }
        if (g_tlbrc.row(1) != 1) { --rtn; std::cout << "tlbrc row(1) fails\n"; }
        if (g_tlbrc.row(2) != 0) { --rtn; std::cout << "tlbrc row(2) fails\n"; }
        if (g_tlbrc.row(3) != 1) { --rtn; std::cout << "tlbrc row(3) fails\n"; }
        if (g_tlbrc.row(4) != 0) { --rtn; std::cout << "tlbrc row(4) fails\n"; }
        if (g_tlbrc.row(5) != 1) { --rtn; std::cout << "tlbrc row(5) fails\n"; }
        if (g_tlbrc.row(6) != 0) { --rtn; std::cout << "tlbrc row(6) fails\n"; }
        if (g_tlbrc.row(7) != 1) { --rtn; std::cout << "tlbrc row(7) fails\n"; }

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
        std::cout << "COL Bottom left top right column major...\n";
        if (g_bltrc.col(0) != 0) { --rtn; std::cout << "bltrc col(0) fails\n"; }
        if (g_bltrc.col(1) != 0) { --rtn; std::cout << "bltrc col(1) fails\n"; }
        if (g_bltrc.col(2) != 1) { --rtn; std::cout << "bltrc col(2) fails\n"; }
        if (g_bltrc.col(3) != 1) { --rtn; std::cout << "bltrc col(3) fails\n"; }
        if (g_bltrc.col(4) != 2) { --rtn; std::cout << "bltrc col(4) fails\n"; }
        if (g_bltrc.col(5) != 2) { --rtn; std::cout << "bltrc col(5) fails\n"; }
        if (g_bltrc.col(6) != 3) { --rtn; std::cout << "bltrc col(6) fails\n"; }
        if (g_bltrc.col(7) != 3) { --rtn; std::cout << "bltrc col(7) fails\n"; }
        std::cout << "COL Top left bottom right column major...\n";
        if (g_tlbrc.col(0) != 0) { --rtn; std::cout << "tlbrc col(0) fails\n"; }
        if (g_tlbrc.col(1) != 0) { --rtn; std::cout << "tlbrc col(1) fails\n"; }
        if (g_tlbrc.col(2) != 1) { --rtn; std::cout << "tlbrc col(2) fails\n"; }
        if (g_tlbrc.col(3) != 1) { --rtn; std::cout << "tlbrc col(3) fails\n"; }
        if (g_tlbrc.col(4) != 2) { --rtn; std::cout << "tlbrc col(4) fails\n"; }
        if (g_tlbrc.col(5) != 2) { --rtn; std::cout << "tlbrc col(5) fails\n"; }
        if (g_tlbrc.col(6) != 3) { --rtn; std::cout << "tlbrc col(6) fails\n"; }
        if (g_tlbrc.col(7) != 3) { --rtn; std::cout << "tlbrc col(7) fails\n"; }
    }

    //
    // morph::GridDomainWrap::Vertical tests
    //
    wrap = morph::GridDomainWrap::Vertical;
    std::cout << "WRAP VERTICAL\n--------------------\n";
    {
        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

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
        std::cout << "ROW Bottom left top right column major...\n";
        if (g_bltrc.row(0) != 0) { --rtn; std::cout << "bltrc row(0) fails\n"; }
        if (g_bltrc.row(1) != 1) { --rtn; std::cout << "bltrc row(1) fails\n"; }
        if (g_bltrc.row(2) != 0) { --rtn; std::cout << "bltrc row(2) fails\n"; }
        if (g_bltrc.row(3) != 1) { --rtn; std::cout << "bltrc row(3) fails\n"; }
        if (g_bltrc.row(4) != 0) { --rtn; std::cout << "bltrc row(4) fails\n"; }
        if (g_bltrc.row(5) != 1) { --rtn; std::cout << "bltrc row(5) fails\n"; }
        if (g_bltrc.row(6) != 0) { --rtn; std::cout << "bltrc row(6) fails\n"; }
        if (g_bltrc.row(7) != 1) { --rtn; std::cout << "bltrc row(7) fails\n"; }
        std::cout << "ROW Top left bottom right column major...\n";
        if (g_tlbrc.row(0) != 0) { --rtn; std::cout << "tlbrc row(0) fails\n"; }
        if (g_tlbrc.row(1) != 1) { --rtn; std::cout << "tlbrc row(1) fails\n"; }
        if (g_tlbrc.row(2) != 0) { --rtn; std::cout << "tlbrc row(2) fails\n"; }
        if (g_tlbrc.row(3) != 1) { --rtn; std::cout << "tlbrc row(3) fails\n"; }
        if (g_tlbrc.row(4) != 0) { --rtn; std::cout << "tlbrc row(4) fails\n"; }
        if (g_tlbrc.row(5) != 1) { --rtn; std::cout << "tlbrc row(5) fails\n"; }
        if (g_tlbrc.row(6) != 0) { --rtn; std::cout << "tlbrc row(6) fails\n"; }
        if (g_tlbrc.row(7) != 1) { --rtn; std::cout << "tlbrc row(7) fails\n"; }

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
        std::cout << "COL Bottom left top right column major...\n";
        if (g_bltrc.col(0) != 0) { --rtn; std::cout << "bltrc col(0) fails\n"; }
        if (g_bltrc.col(1) != 0) { --rtn; std::cout << "bltrc col(1) fails\n"; }
        if (g_bltrc.col(2) != 1) { --rtn; std::cout << "bltrc col(2) fails\n"; }
        if (g_bltrc.col(3) != 1) { --rtn; std::cout << "bltrc col(3) fails\n"; }
        if (g_bltrc.col(4) != 2) { --rtn; std::cout << "bltrc col(4) fails\n"; }
        if (g_bltrc.col(5) != 2) { --rtn; std::cout << "bltrc col(5) fails\n"; }
        if (g_bltrc.col(6) != 3) { --rtn; std::cout << "bltrc col(6) fails\n"; }
        if (g_bltrc.col(7) != 3) { --rtn; std::cout << "bltrc col(7) fails\n"; }
        std::cout << "COL Top left bottom right column major...\n";
        if (g_tlbrc.col(0) != 0) { --rtn; std::cout << "tlbrc col(0) fails\n"; }
        if (g_tlbrc.col(1) != 0) { --rtn; std::cout << "tlbrc col(1) fails\n"; }
        if (g_tlbrc.col(2) != 1) { --rtn; std::cout << "tlbrc col(2) fails\n"; }
        if (g_tlbrc.col(3) != 1) { --rtn; std::cout << "tlbrc col(3) fails\n"; }
        if (g_tlbrc.col(4) != 2) { --rtn; std::cout << "tlbrc col(4) fails\n"; }
        if (g_tlbrc.col(5) != 2) { --rtn; std::cout << "tlbrc col(5) fails\n"; }
        if (g_tlbrc.col(6) != 3) { --rtn; std::cout << "tlbrc col(6) fails\n"; }
        if (g_tlbrc.col(7) != 3) { --rtn; std::cout << "tlbrc col(7) fails\n"; }
    }

    //
    // morph::GridDomainWrap::Both tests
    //
    wrap = morph::GridDomainWrap::Both;
    std::cout << "WRAP BOTH\n--------------------\n";
    {
        morph::Grid<int, float> g_bltr(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright);
        morph::Grid<int, float> g_tlbr(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright);
        morph::Grid<int, float> g_bltrc(4, 2, dx, offset, wrap, morph::GridOrder::bottomleft_to_topright_colmaj);
        morph::Grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, morph::GridOrder::topleft_to_bottomright_colmaj);

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
        std::cout << "ROW Bottom left top right column major...\n";
        if (g_bltrc.row(0) != 0) { --rtn; std::cout << "bltrc row(0) fails\n"; }
        if (g_bltrc.row(1) != 1) { --rtn; std::cout << "bltrc row(1) fails\n"; }
        if (g_bltrc.row(2) != 0) { --rtn; std::cout << "bltrc row(2) fails\n"; }
        if (g_bltrc.row(3) != 1) { --rtn; std::cout << "bltrc row(3) fails\n"; }
        if (g_bltrc.row(4) != 0) { --rtn; std::cout << "bltrc row(4) fails\n"; }
        if (g_bltrc.row(5) != 1) { --rtn; std::cout << "bltrc row(5) fails\n"; }
        if (g_bltrc.row(6) != 0) { --rtn; std::cout << "bltrc row(6) fails\n"; }
        if (g_bltrc.row(7) != 1) { --rtn; std::cout << "bltrc row(7) fails\n"; }
        std::cout << "ROW Top left bottom right column major...\n";
        if (g_tlbrc.row(0) != 0) { --rtn; std::cout << "tlbrc row(0) fails\n"; }
        if (g_tlbrc.row(1) != 1) { --rtn; std::cout << "tlbrc row(1) fails\n"; }
        if (g_tlbrc.row(2) != 0) { --rtn; std::cout << "tlbrc row(2) fails\n"; }
        if (g_tlbrc.row(3) != 1) { --rtn; std::cout << "tlbrc row(3) fails\n"; }
        if (g_tlbrc.row(4) != 0) { --rtn; std::cout << "tlbrc row(4) fails\n"; }
        if (g_tlbrc.row(5) != 1) { --rtn; std::cout << "tlbrc row(5) fails\n"; }
        if (g_tlbrc.row(6) != 0) { --rtn; std::cout << "tlbrc row(6) fails\n"; }
        if (g_tlbrc.row(7) != 1) { --rtn; std::cout << "tlbrc row(7) fails\n"; }

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
        std::cout << "COL Bottom left top right column major...\n";
        if (g_bltrc.col(0) != 0) { --rtn; std::cout << "bltrc col(0) fails\n"; }
        if (g_bltrc.col(1) != 0) { --rtn; std::cout << "bltrc col(1) fails\n"; }
        if (g_bltrc.col(2) != 1) { --rtn; std::cout << "bltrc col(2) fails\n"; }
        if (g_bltrc.col(3) != 1) { --rtn; std::cout << "bltrc col(3) fails\n"; }
        if (g_bltrc.col(4) != 2) { --rtn; std::cout << "bltrc col(4) fails\n"; }
        if (g_bltrc.col(5) != 2) { --rtn; std::cout << "bltrc col(5) fails\n"; }
        if (g_bltrc.col(6) != 3) { --rtn; std::cout << "bltrc col(6) fails\n"; }
        if (g_bltrc.col(7) != 3) { --rtn; std::cout << "bltrc col(7) fails\n"; }
        std::cout << "COL Top left bottom right column major...\n";
        if (g_tlbrc.col(0) != 0) { --rtn; std::cout << "tlbrc col(0) fails\n"; }
        if (g_tlbrc.col(1) != 0) { --rtn; std::cout << "tlbrc col(1) fails\n"; }
        if (g_tlbrc.col(2) != 1) { --rtn; std::cout << "tlbrc col(2) fails\n"; }
        if (g_tlbrc.col(3) != 1) { --rtn; std::cout << "tlbrc col(3) fails\n"; }
        if (g_tlbrc.col(4) != 2) { --rtn; std::cout << "tlbrc col(4) fails\n"; }
        if (g_tlbrc.col(5) != 2) { --rtn; std::cout << "tlbrc col(5) fails\n"; }
        if (g_tlbrc.col(6) != 3) { --rtn; std::cout << "tlbrc col(6) fails\n"; }
        if (g_tlbrc.col(7) != 3) { --rtn; std::cout << "tlbrc col(7) fails\n"; }
    }

    if (rtn == 0) {
        std::cout << "All tests PASSED\n";
    } else {
        std::cout << "Some tests failed\n";
    }
    return rtn;
}
