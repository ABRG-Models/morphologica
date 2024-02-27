/*!
 * \file
 *
 * This is a simple Cartesian Grid class. Simpler than morph::CartGrid. There is no option to
 * define an arbitrary boundary to your domain.
 *
 * I'm writing this as a fully compile-time class for which the programmer sets grid width and
 * height (in number of pixels) the gridspacing (as a vec<> hopefully) and the offset (another vec)
 * as template arguments. I think this will require C++-20 which permits floating point values to be
 * passed as template args.
 *
 * I might later make a runtime one, which will be compatible with <= C++-17
 *
 * \author Seb James
 * \date February 2024
 */

#pragma once

#include <string>
#include <sstream>
#include <morph/vec.h>
#include <morph/CartDomains.h>

namespace morph {

    /*!
     * A nice simple grid class. This provides coordinates for each element in a grid along with
     * neighbour relationships. The data associated with the grid is expected to be stored in an
     * array or vector. Use the index of the array/vector to obtain info about the grid
     * locations. The size of the grid is not expected to change and so is provided as template args
     *
     * \tparam n_x Number of elements that the grid is wide
     *
     * \tparam n_y Number of elements that the grid is high
     *
     * \tparam dx A two element vec providing the horizontal distance between adjacent grid element
     * centres (element 0) and the vertical distance between adjacent grid element centres (element 1).
     *
     * \tparam g_offset A vector giving the distance offset (in your chosen units) to Grid index 0.
     *
     * \tparam d_wrap An enum to set how the grid wraps. Affects neighbour relationships
     *
     * \tparam g_order The index order. Always counting left to right, but do you start on the top row or
     * the bottom row (the default)?
     */
    template < size_t n_x, size_t n_y,
               morph::vec<float, 2> dx = { 1.0f, 1.0f },
               morph::vec<float, 2> g_offset = { 0.0f, 0.0f },
               CartDomainWrap d_wrap = CartDomainWrap::None,
               GridOrder g_order = morph::GridOrder::bottomleft_to_topright >
    struct Grid
    {
        //! The number of elements in the grid
        static constexpr size_t n = n_x * n_y;

        // Indexing the grid will return a computed vec location.
        constexpr morph::vec<float, 2> operator[] (const size_t index) const
        {
            if (index >= n) { return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}); }
            morph::vec<float, 2> loc = g_offset;
            loc[0] += dx[0] * (index % n_x);
            if constexpr (g_order == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += dx[1] * (index / n_x);
            } else {
                loc[1] -= dx[1] * (index / n_x);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<size_t>::max()
        constexpr size_t index_ne (const size_t index)
        {
            size_t r = this->row (index);
            if (r == (n_x - 1) && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Vertical)) {
                return std::numeric_limits<size_t>::max();
            } else if (r == (n_x - 1) && (d_wrap == CartDomainWrap::Horizontal || d_wrap == CartDomainWrap::Both)) {
                return index - (n_x - 1);
            } else {
                return index + 1;
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_ne (const size_t index)
        {
            size_t idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the east
        constexpr bool has_ne (const size_t index) { return index_ne(index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<size_t>::max()
        constexpr size_t index_nw (const size_t index)
        {
            size_t r = this->row (index);
            if (r == 0 && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Vertical)) {
                return std::numeric_limits<size_t>::max();
            } else if (r == 0 && (d_wrap == CartDomainWrap::Horizontal || d_wrap == CartDomainWrap::Both)) {
                return index + (n_x - 1);
            } else {
                return index - 1;
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_nw (const size_t index)
        {
            size_t idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the west
        constexpr bool has_nw (const size_t index) { return index_nw(index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<size_t>::max()
        constexpr size_t index_nn (const size_t index)
        {
            size_t c = this->col (index);
            //std::cout << "Col c = " << c << " and n_y - 1 = " << (n_y - 1) << std::endl;
            if (c == (n_y - 1) && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Horizontal)) {
                return std::numeric_limits<size_t>::max();
            } else if (c == (n_y - 1) && (d_wrap == CartDomainWrap::Vertical || d_wrap == CartDomainWrap::Both)) {
                return index - (n_x * (n_y - 1));
            } else {
                return index + n_x;
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_nn (const size_t index)
        {
            size_t idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the north
        constexpr bool has_nn (const size_t index) { return index_nn(index) != std::numeric_limits<size_t>::max(); }

        /*
         * What is the width of a grid? Is it the distance from the centre of the left-most pixel to
         * the centre of the right-most pixel, OR is it the distance from the left edge of the
         * left-most pixel to the right edge of the right-most pixel? It could be either, so I
         * provide width() and width_of_pixels() as well as height() and height_of_pixels().
         */

        //! Return the distance from the centre of the left element column to the centre of the
        //! right element column
        float width() const { return dx[0] * n_x; }

        //! Return the width of the grid if drawn as pixels
        float width_of_pixels() const { return dx[0] * n_x + dx[0]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        float height() const { return dx[1] * n_y; }

        //! Return the height of the grid if drawn as pixels
        float height_of_pixels() const { return dx[1] * n_y + dx[1]; }

        //! Return the area of the grid, if drawn as pixels
        float area_of_pixels() const { return width_of_pixels() * height_of_pixels(); }

        //! Return the row for the index
        size_t row (const size_t index) const { return index < n ? index % n_x : std::numeric_limits<size_t>::max(); }

        //! Return the col for the index
        size_t col (const size_t index) const
        {
            // FIXME: Do I need to switch on d_wrap?
            return index < n ? index / n_x : std::numeric_limits<size_t>::max();
        }

        /*!
         * Output some text information about the hexgrid.
         */
        std::string output() const
        {
            std::stringstream ss;
            ss << n_x << " x " << n_y << " grid (" << n << " elements).";
            return ss.str();
        }

    };

} // namespace morph
