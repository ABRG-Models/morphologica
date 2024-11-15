#include <morph/Grid.h>
#include <iostream>

int main()
{
    int rtn = 0;
    morph::vec<float, 2> dx = {1.0f, 1.0f};
    morph::vec<float, 2> offset = {0.0f, 0.0f};
    morph::GridDomainWrap wrap = morph::GridDomainWrap::None;
    morph::GridOrder order = morph::GridOrder::bottomleft_to_topright;
    int start_ind = 7;

    // ================================= ROW MAJOR GRID =========================================
    // ================== x_shift ===========================
    {
        // On-grid horizontal movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = 2;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 4) { --rtn; }
    }
    {
        // On-grid horizontal movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = -2;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 0) { --rtn; }
    }
    {
        // Off-grid to the right horizontal movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = 3;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the left horizontal movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = -3;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the right horizontal movement. Row major. Horizontal wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Horizontal);
        int hor_shift = 3;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 0) { --rtn; }
    }
    {
        // Off-grid to the left horizontal movement. Row major. Horizontal wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Horizontal);
        int hor_shift = -4;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 3) { --rtn; }
    }
    // ================== y_shift ================================
    {
        // On-grid vertical movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int ver_shift = 2;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 3) { --rtn; }
    }
    {
        // Off-grid to the top, vertical movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int ver_shift = 3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the bottom, vertical movement. Row major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int ver_shift = -3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the top, vertical movement. Row major. Vertical wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Vertical);
        int ver_shift = 3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 0) { --rtn; }
    }
    {
        // Off-grid to the bottom, vertical movement. Row major. Vertical wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Vertical);
        int ver_shift = -3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 2) { --rtn; }
    }

    // =================================  COLUMN MAJOR GRID ======================================
    // ================== x_shift =========================

    order = morph::GridOrder::bottomleft_to_topright_colmaj;

    {
        // On-grid horizontal movement. Column major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = 2;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 3) { --rtn; }
    }
    {
        // Off-grid to the right horizontal movement. Column major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = 4;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the left horizontal movement. Column major
        morph::Grid<int, float> g(5, 4, dx, offset, wrap, order);
        int hor_shift = -2;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the right horizontal movement. Column major. Horizontal wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Horizontal, order);
        int hor_shift = 4;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 0) { --rtn; }
    }
    {
        // Off-grid to the left horizontal movement. Column major. Horizontal wrapping
        morph::Grid<int, float> g(5, 4, dx, offset, morph::GridDomainWrap::Horizontal, order);
        int hor_shift = -3;
        int ind_after_move = g.col_after_x_shift (start_ind, hor_shift);
        if (ind_after_move != 3) { --rtn; }
    }
    // ================== y_shift ===================================
    {
        // On-grid vertical movement. Column major
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        int ver_shift = 2;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 4) { --rtn; }
    }
    {
        // Off-grid to the top, vertical movement. Column major
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        int ver_shift = 3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the bottom, vertical movement. Column major
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        int ver_shift = -3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid to the top, vertical movement. Column major. Vertical wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Vertical, order);
        int ver_shift = 3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 0) { --rtn; }
    }
    {
        // Off-grid to the bottom, vertical movement. Column major. Vertical wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Vertical, order);
        int ver_shift = -3;
        int ind_after_move = g.row_after_y_shift (start_ind, ver_shift);
        if (ind_after_move != 4) { --rtn; }
    }

    // ============================== SHIFT_INDEX TEST =======================================
    // ====== Row major ========================
    order = morph::GridOrder::bottomleft_to_topright;

    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {2, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 19) { --rtn; }
    }
    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, 3};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 20) { --rtn; }
    }
    {
        // Off-grid on the horizontal
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-3, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the vertical
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, -2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the horizontal with horizontal wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Horizontal, order);
        morph::vec<int, 2> delta = {3, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        std::cout << "ind_after_move:  " << ind_after_move << std::endl;
        if (ind_after_move != 15) { --rtn; }
    }

    // ====== Column major ========================
    order = morph::GridOrder::bottomleft_to_topright_colmaj;

    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {3, -2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 20) { --rtn; }
    }
    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-1, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 4) { --rtn; }
    }
    {
        // Off-grid on the horizontal
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the vertical
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-1, -3};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the horizontal with horizontal wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Horizontal, order);
        morph::vec<int, 2> delta = {-3, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 18) { --rtn; }
    }
    // ============================== SHIFT_INDEX TEST =======================================
    // ====== Row major ========================
    order = morph::GridOrder::bottomleft_to_topright;

    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {2, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 19) { --rtn; }
    }
    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, 3};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 20) { --rtn; }
    }
    {
        // Off-grid on the horizontal
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-3, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the vertical
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, -2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the horizontal with horizontal wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Horizontal, order);
        morph::vec<int, 2> delta = {3, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        std::cout << "ind_after_move:  " << ind_after_move << std::endl;
        if (ind_after_move != 15) { --rtn; }
    }

    // ====== Column major ========================
    order = morph::GridOrder::bottomleft_to_topright_colmaj;

    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {3, -2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 20) { --rtn; }
    }
    {
        // On-grid
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-1, 2};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 4) { --rtn; }
    }
    {
        // Off-grid on the horizontal
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-2, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the vertical
        morph::Grid<int, float> g(5, 5, dx, offset, wrap, order);
        morph::vec<int, 2> delta = {-1, -3};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != std::numeric_limits<int>::max()) { --rtn; }
    }
    {
        // Off-grid on the horizontal with horizontal wrapping
        morph::Grid<int, float> g(5, 5, dx, offset, morph::GridDomainWrap::Horizontal, order);
        morph::vec<int, 2> delta = {-3, 1};
        int ind_after_move = g.shift_index (start_ind, delta);
        if (ind_after_move != 18) { --rtn; }
    }

    return rtn;
}
