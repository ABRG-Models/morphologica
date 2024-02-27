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

    // What's the ordering of the grid?
    //
    // An example grid of width 4 and height 2 should illustrate:
    //
    // bottomleft_to_topright:
    //
    // 4 5 6 7
    // 0 1 2 3
    //
    // topleft_to_bottomright:
    //
    // 0 1 2 3
    // 4 5 6 7
    //
    // Note that I've omitted the possibility of indexing the grid in column major format.
    enum class GridOrder
    {
        bottomleft_to_topright,
        topleft_to_bottomright
    };

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
     * \tparam zoffset A vector giving the distance offset (in your chosen units) to Grid index 0.
     *
     * \tparam domainWrap An enum to set how the grid wraps. Affects neighbour relationships
     *
     * \tparam O The index order. Always counting left to right, but do you start on the top row or
     * the bottom row (the default)?
     */
    template < size_t n_x, size_t n_y,
               morph::vec<float, 2> dx = { 1.0f, 1.0f },
               morph::vec<float, 2> zoffset = { 0.0f, 0.0f },
               CartDomainWrap domainWrap = CartDomainWrap::None,
               GridOrder O = morph::GridOrder::bottomleft_to_topright >
    struct Grid
    {
        //! The number of elements in the grid
        static constexpr size_t n = n_x * n_y;

        // Indexing the grid will return a computed vec location. Would like this to be constexpr
        constexpr morph::vec<float, 2> operator[] (size_t index) const
        {
            morph::vec<float, 2> loc = zoffset;
            loc[0] += dx[0] * (index % n_x);
            if constexpr (O == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += dx[1] * (index / n_x);
            } else {
                loc[1] -= dx[1] * (index / n_x);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<size_t>::max()
        constexpr size_t index_ne (size_t index)
        {
            size_t rtn = std::numeric_limits<size_t>::max();
            // Compute row
            size_t r = index % n_x;
            if (r == (n_x - 1) && (domainWrap == CartDomainWrap::None || domainWrap == CartDomainWrap::Vertical)) {
                return rtn;
            } else if (r == (n_x - 1) && (domainWrap == CartDomainWrap::Horizontal || domainWrap == CartDomainWrap::Both)) {
                return index - (n_x - 1);
            } else {
                return index + 1;
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_ne (size_t index)
        {
            size_t idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the east
        constexpr bool has_ne (size_t index)
        {
            return index_ne (index) == std::numeric_limits<size_t>::max() ? false : true;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<size_t>::max()
        constexpr size_t index_nw (size_t index)
        {
            size_t rtn = std::numeric_limits<size_t>::max();
            // Compute row
            size_t r = index % n_x;
            if (r == 0 && (domainWrap == CartDomainWrap::None || domainWrap == CartDomainWrap::Vertical)) {
                return rtn;
            } else if (r == 0 && (domainWrap == CartDomainWrap::Horizontal || domainWrap == CartDomainWrap::Both)) {
                return index + (n_x - 1);
            } else {
                return index - 1;
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_nw (size_t index)
        {
            size_t idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the west
        constexpr bool has_nw (size_t index)
        {
            return index_nw (index) == std::numeric_limits<size_t>::max() ? false : true;
        }

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
