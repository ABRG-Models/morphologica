#include <morph/Grid.h>
#include <iostream>

int main()
{
    int rtn = 0;

    // x_shift ========================================================

    morph::Grid<int, float> g(5, 4, morph::vec<float, 2>({1.0f, 1.0f}));

    // On-grid horizontal movement. Row major

    int start_ind = 7;
    int hor_shift = 2;

    int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);

    if (ind_after_move != 9) { --rtn; }

    //


    return rtn;
}
