#include "morph/Grid.h"
#include <iostream>
#include <limits>

int main()
{
    int rtn = 0;

    constexpr morph::vec<float, 2> grid_spacing = { 1.0f, 1.0f };
    constexpr morph::vec<float, 2> grid_zero = { 0.0f, 0.0f };
    constexpr bool non_memory = false;
    constexpr bool with_memory = true;
    constexpr size_t no_element = std::numeric_limits<size_t>::max();

    // Expected coordinate
    morph::vec<float, 2> expected = { 0.0f, 0.0f };

    // Test 1. 2x2 grid, no offset, memory coords, no wrapping, bottomleft_to_topright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, with_memory, morph::CartDomainWrap::None, morph::GridOrder::bottomleft_to_topright> grid1;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid1[0] != expected) { --rtn; }
    if (grid1.index_ne(0) != 1) { --rtn; }
    if (grid1.index_nw(0) != no_element) { --rtn; }
    if (grid1.index_nn(0) != 2) { --rtn; }
    if (grid1.index_ns(0) != no_element) { --rtn; }
    if (grid1.index_nne(0) != 3) { --rtn; }
    if (grid1.index_nnw(0) != no_element) { --rtn; }
    if (grid1.index_nse(0) != no_element) { --rtn; }
    if (grid1.index_nsw(0) != no_element) { --rtn; }
    if (rtn != 0) { std::cout << "Test 1 el 0 FAILED\n"; return rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid1[1] != expected) { --rtn; }
    if (grid1.index_ne(1) != no_element) { --rtn; }
    if (grid1.index_nw(1) != 0) { --rtn; }
    if (grid1.index_nn(1) != 3) { --rtn; }
    if (grid1.index_ns(1) != no_element) { --rtn; }
    if (grid1.index_nne(1) != no_element) { --rtn; }
    if (grid1.index_nnw(1) != 2) { --rtn; }
    if (grid1.index_nse(1) != no_element) { --rtn; }
    if (grid1.index_nsw(1) != no_element) { --rtn; }
    if (rtn != 0) { std::cout << "Test 1 el 1 FAILED\n"; return rtn; }

    // Element 2
    expected = { 0.0f, 1.0f };
    if (grid1[2] != expected) { --rtn; }
    if (grid1.index_ne(2) != 3) { --rtn; }
    if (grid1.index_nw(2) != no_element) { --rtn; }
    if (grid1.index_nn(2) != no_element) { --rtn; }
    if (grid1.index_ns(2) != 0) { --rtn; }
    if (grid1.index_nne(2) != no_element) { --rtn; }
    if (grid1.index_nnw(2) != no_element) { --rtn; }
    if (grid1.index_nse(2) != 1) { --rtn; }
    if (grid1.index_nsw(2) != no_element) { --rtn; }
    if (rtn != 0) { std::cout << "Test 1 el 2 FAILED\n"; return rtn; }

    // Element 3
    expected = { 1.0f, 1.0f };
    if (grid1[3] != expected) { --rtn; }
    if (grid1.index_ne(3) != no_element) { --rtn; }
    if (grid1.index_nw(3) != 2) { --rtn; }
    if (grid1.index_nn(3) != no_element) { --rtn; }
    if (grid1.index_ns(3) != 1) { --rtn; }
    if (grid1.index_nne(3) != no_element) { --rtn; }
    if (grid1.index_nnw(3) != no_element) { --rtn; }
    if (grid1.index_nse(3) != no_element) { --rtn; }
    if (grid1.index_nsw(3) != 0) { --rtn; }
    if (rtn != 0) { std::cout << "Test 1 el 3 FAILED\n"; return rtn; }

    // Test 1.1. 2x2 grid, no offset, NO memory coords, no wrapping, bottomleft_to_topright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, non_memory, morph::CartDomainWrap::None, morph::GridOrder::bottomleft_to_topright> grid1p1;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid1p1[0] != expected) { --rtn; }
    if (grid1p1.index_ne(0) != 1) { --rtn; }
    if (grid1p1.index_nw(0) != no_element) { --rtn; }
    if (grid1p1.index_nn(0) != 2) { --rtn; }
    if (grid1p1.index_ns(0) != no_element) { --rtn; }
    if (grid1p1.index_nne(0) != 3) { --rtn; }
    if (grid1p1.index_nnw(0) != no_element) { --rtn; }
    if (grid1p1.index_nse(0) != no_element) { --rtn; }
    if (grid1p1.index_nsw(0) != no_element) { --rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid1p1[1] != expected) { --rtn; }
    if (grid1p1.index_ne(1) != no_element) { --rtn; }
    if (grid1p1.index_nw(1) != 0) { --rtn; }
    if (grid1p1.index_nn(1) != 3) { --rtn; }
    if (grid1p1.index_ns(1) != no_element) { --rtn; }
    if (grid1p1.index_nne(1) != no_element) { --rtn; }
    if (grid1p1.index_nnw(1) != 2) { --rtn; }
    if (grid1p1.index_nse(1) != no_element) { --rtn; }
    if (grid1p1.index_nsw(1) != no_element) { --rtn; }

    // Element 2
    expected = { 0.0f, 1.0f };
    if (grid1p1[2] != expected) { --rtn; }
    if (grid1p1.index_ne(2) != 3) { --rtn; }
    if (grid1p1.index_nw(2) != no_element) { --rtn; }
    if (grid1p1.index_nn(2) != no_element) { --rtn; }
    if (grid1p1.index_ns(2) != 0) { --rtn; }
    if (grid1p1.index_nne(2) != no_element) { --rtn; }
    if (grid1p1.index_nnw(2) != no_element) { --rtn; }
    if (grid1p1.index_nse(2) != 1) { --rtn; }
    if (grid1p1.index_nsw(2) != no_element) { --rtn; }

    // Element 3
    expected = { 1.0f, 1.0f };
    if (grid1p1[3] != expected) { --rtn; }
    if (grid1p1.index_ne(3) != no_element) { --rtn; }
    if (grid1p1.index_nw(3) != 2) { --rtn; }
    if (grid1p1.index_nn(3) != no_element) { --rtn; }
    if (grid1p1.index_ns(3) != 1) { --rtn; }
    if (grid1p1.index_nne(3) != no_element) { --rtn; }
    if (grid1p1.index_nnw(3) != no_element) { --rtn; }
    if (grid1p1.index_nse(3) != no_element) { --rtn; }
    if (grid1p1.index_nsw(3) != 0) { --rtn; }

    if (rtn != 0) { std::cout << "Test 1.1 FAILED\n"; return rtn; }

    // Test 2. 2x2 grid, no offset, memory coords, horz wrapping, bottomleft_to_topright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, with_memory, morph::CartDomainWrap::Horizontal, morph::GridOrder::bottomleft_to_topright> grid2;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid2[0] != expected) { --rtn; }
    if (grid2.index_ne(0) != 1) { --rtn; }
    if (grid2.index_nw(0) != 1) { --rtn; }
    if (grid2.index_nn(0) != 2) { --rtn; }
    if (grid2.index_ns(0) != no_element) { --rtn; }
    if (grid2.index_nne(0) != 3) { --rtn; }
    if (grid2.index_nnw(0) != 3) { --rtn; }
    if (grid2.index_nse(0) != no_element) { --rtn; }
    if (grid2.index_nsw(0) != no_element) { --rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid2[1] != expected) { --rtn; }
    if (grid2.index_ne(1) != 0) { --rtn; }
    if (grid2.index_nw(1) != 0) { --rtn; }
    if (grid2.index_nn(1) != 3) { --rtn; }
    if (grid2.index_ns(1) != no_element) { --rtn; }
    if (grid2.index_nne(1) != 2) { --rtn; }
    if (grid2.index_nnw(1) != 2) { --rtn; }
    if (grid2.index_nse(1) != no_element) { --rtn; }
    if (grid2.index_nsw(1) != no_element) { --rtn; }

    // Element 2
    expected = { 0.0f, 1.0f };
    if (grid2[2] != expected) { --rtn; }
    if (grid2.index_ne(2) != 3) { --rtn; }
    if (grid2.index_nw(2) != 3) { --rtn; }
    if (grid2.index_nn(2) != no_element) { --rtn; }
    if (grid2.index_ns(2) != 0) { --rtn; }
    if (grid2.index_nne(2) != no_element) { --rtn; }
    if (grid2.index_nnw(2) != no_element) { --rtn; }
    if (grid2.index_nse(2) != 1) { --rtn; }
    if (grid2.index_nsw(2) != 1) { --rtn; }

    // Element 3
    expected = { 1.0f, 1.0f };
    if (grid2[3] != expected) { --rtn; }
    if (grid2.index_ne(3) != 2) { --rtn; }
    if (grid2.index_nw(3) != 2) { --rtn; }
    if (grid2.index_nn(3) != no_element) { --rtn; }
    if (grid2.index_ns(3) != 1) { --rtn; }
    if (grid2.index_nne(3) != no_element) { --rtn; }
    if (grid2.index_nnw(3) != no_element) { --rtn; }
    if (grid2.index_nse(3) != 0) { --rtn; }
    if (grid2.index_nsw(3) != 0) { --rtn; }

    if (rtn != 0) { std::cout << "Test 2 FAILED\n"; return rtn; }

    // Test 3. 2x2 grid, no offset, memory coords, both wrapping, bottomleft_to_topright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, with_memory, morph::CartDomainWrap::Both, morph::GridOrder::bottomleft_to_topright> grid3;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid3[0] != expected) { --rtn; }
    if (grid3.index_ne(0) != 1) { --rtn; }
    if (grid3.index_nw(0) != 1) { --rtn; }
    if (grid3.index_nn(0) != 2) { --rtn; }
    if (grid3.index_ns(0) != 2) { --rtn; }
    if (grid3.index_nne(0) != 3) { --rtn; }
    if (grid3.index_nnw(0) != 3) { --rtn; }
    if (grid3.index_nse(0) != 3) { --rtn; }
    if (grid3.index_nsw(0) != 3) { --rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid3[1] != expected) { --rtn; }
    if (grid3.index_ne(1) != 0) { --rtn; }
    if (grid3.index_nw(1) != 0) { --rtn; }
    if (grid3.index_nn(1) != 3) { --rtn; }
    if (grid3.index_ns(1) != 3) { --rtn; }
    if (grid3.index_nne(1) != 2) { --rtn; }
    if (grid3.index_nnw(1) != 2) { --rtn; }
    if (grid3.index_nse(1) != 2) { --rtn; }
    if (grid3.index_nsw(1) != 2) { --rtn; }

    // Element 2
    expected = { 0.0f, 1.0f };
    if (grid3[2] != expected) { --rtn; }
    if (grid3.index_ne(2) != 3) { --rtn; }
    if (grid3.index_nw(2) != 3) { --rtn; }
    if (grid3.index_nn(2) != 0) { --rtn; }
    if (grid3.index_ns(2) != 0) { --rtn; }
    if (grid3.index_nne(2) != 1) { --rtn; }
    if (grid3.index_nnw(2) != 1) { --rtn; }
    if (grid3.index_nse(2) != 1) { --rtn; }
    if (grid3.index_nsw(2) != 1) { --rtn; }

    // Element 3
    expected = { 1.0f, 1.0f };
    if (grid3[3] != expected) { --rtn; }
    if (grid3.index_ne(3) != 2) { --rtn; }
    if (grid3.index_nw(3) != 2) { --rtn; }
    if (grid3.index_nn(3) != 1) { --rtn; }
    if (grid3.index_ns(3) != 1) { --rtn; }
    if (grid3.index_nne(3) != 0) { --rtn; }
    if (grid3.index_nnw(3) != 0) { --rtn; }
    if (grid3.index_nse(3) != 0) { --rtn; }
    if (grid3.index_nsw(3) != 0) { --rtn; }

    if (rtn != 0) { std::cout << "Test 3 FAILED\n"; return rtn; }

    // Test 4. 2x2 grid, no offset, memory coords, no wrapping, topleft_to_bottomright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, with_memory, morph::CartDomainWrap::None, morph::GridOrder::topleft_to_bottomright> grid4;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid4[0] != expected) { --rtn; }
    if (grid4.index_ne(0) != 1) { --rtn; }
    if (grid4.index_nw(0) != no_element) { --rtn; }
    if (grid4.index_nn(0) != no_element) { --rtn; }
    if (grid4.index_ns(0) != 2) { --rtn; }
    if (grid4.index_nne(0) != no_element) { --rtn; }
    if (grid4.index_nnw(0) != no_element) { --rtn; }
    if (grid4.index_nse(0) != 3) { --rtn; }
    if (grid4.index_nsw(0) != no_element) { --rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid4[1] != expected) { --rtn; }
    if (grid4.index_ne(1) != no_element) { --rtn; }
    if (grid4.index_nw(1) != 0) { --rtn; }
    if (grid4.index_nn(1) != no_element) { --rtn; }
    if (grid4.index_ns(1) != 3) { --rtn; }
    if (grid4.index_nne(1) != no_element) { --rtn; }
    if (grid4.index_nnw(1) != no_element) { --rtn; }
    if (grid4.index_nse(1) != no_element) { --rtn; }
    if (grid4.index_nsw(1) != 2) { --rtn; }

    // Element 2
    expected = { 0.0f, -1.0f };
    if (grid4[2] != expected) { --rtn; }
    if (grid4.index_ne(2) != 3) { --rtn; }
    if (grid4.index_nw(2) != no_element) { --rtn; }
    if (grid4.index_nn(2) != 0) { --rtn; }
    if (grid4.index_ns(2) != no_element) { --rtn; }
    if (grid4.index_nne(2) != 1) { --rtn; }
    if (grid4.index_nnw(2) != no_element) { --rtn; }
    if (grid4.index_nse(2) != no_element) { --rtn; }
    if (grid4.index_nsw(2) != no_element) { --rtn; }

    // Element 3
    expected = { 1.0f, -1.0f };
    if (grid4[3] != expected) { --rtn; }
    if (grid4.index_ne(3) != no_element) { --rtn; }
    if (grid4.index_nw(3) != 2) { --rtn; }
    if (grid4.index_nn(3) != 1) { --rtn; }
    if (grid4.index_ns(3) != no_element) { --rtn; }
    if (grid4.index_nne(3) != no_element) { --rtn; }
    if (grid4.index_nnw(3) != 0) { --rtn; }
    if (grid4.index_nse(3) != no_element) { --rtn; }
    if (grid4.index_nsw(3) != no_element) { --rtn; }

    if (rtn != 0) { std::cout << "Test 4 FAILED\n"; return rtn; }

    // Test 5. 2x2 grid, no offset, memory coords, vertical wrapping, topleft_to_bottomright ordering,
    morph::Grid<2, 2, grid_spacing, grid_zero, with_memory, morph::CartDomainWrap::Vertical, morph::GridOrder::topleft_to_bottomright> grid5;

    // Test element 0
    expected = { 0.0f, 0.0f };
    if (grid5[0] != expected) { --rtn; }
    if (grid5.index_ne(0) != 1) { --rtn; }
    if (grid5.index_nw(0) != no_element) { --rtn; }
    if (grid5.index_nn(0) != 2) { --rtn; }
    if (grid5.index_ns(0) != 2) { --rtn; }
    if (grid5.index_nne(0) != 3) { --rtn; }
    if (grid5.index_nnw(0) != no_element) { --rtn; }
    if (grid5.index_nse(0) != 3) { --rtn; }
    if (grid5.index_nsw(0) != no_element) { --rtn; }

    // Element 1
    expected = { 1.0f, 0.0f };
    if (grid5[1] != expected) { --rtn; }
    if (grid5.index_ne(1) != no_element) { --rtn; }
    if (grid5.index_nw(1) != 0) { --rtn; }
    if (grid5.index_nn(1) != 3) { --rtn; }
    if (grid5.index_ns(1) != 3) { --rtn; }
    if (grid5.index_nne(1) != no_element) { --rtn; }
    if (grid5.index_nnw(1) != 2) { --rtn; }
    if (grid5.index_nse(1) != no_element) { --rtn; }
    if (grid5.index_nsw(1) != 2) { --rtn; }

    // Element 2
    expected = { 0.0f, -1.0f };
    if (grid5[2] != expected) { --rtn; }
    if (grid5.index_ne(2) != 3) { --rtn; }
    if (grid5.index_nw(2) != no_element) { --rtn; }
    if (grid5.index_nn(2) != 0) { --rtn; }
    if (grid5.index_ns(2) != 0) { --rtn; }
    if (grid5.index_nne(2) != 1) { --rtn; }
    if (grid5.index_nnw(2) != no_element) { --rtn; }
    if (grid5.index_nse(2) != 1) { --rtn; }
    if (grid5.index_nsw(2) != no_element) { --rtn; }

    // Element 3
    expected = { 1.0f, -1.0f };
    if (grid5[3] != expected) { --rtn; }
    if (grid5.index_ne(3) != no_element) { --rtn; }
    if (grid5.index_nw(3) != 2) { --rtn; }
    if (grid5.index_nn(3) != 1) { --rtn; }
    if (grid5.index_ns(3) != 1) { --rtn; }
    if (grid5.index_nne(3) != no_element) { --rtn; }
    if (grid5.index_nnw(3) != 0) { --rtn; }
    if (grid5.index_nse(3) != no_element) { --rtn; }
    if (grid5.index_nsw(3) != 0) { --rtn; }

    if (rtn != 0) { std::cout << "Test 5 FAILED\n"; return rtn; }

    if (rtn == 0) {
        std::cout << "Test PASSED\n";
    } else {
        std::cout << "Test FAILED\n";
    }
    return rtn;
}
