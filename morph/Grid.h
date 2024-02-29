/*!
 * \file
 *
 * This file contains the definition for morph::Grid<>, a simple Cartesian Grid
 * class. It is much simpler than morph::CartGrid because there is no option to define
 * an arbitrary boundary to your domain. This code is a few hundred lines of code versus
 * about 2600 lines in CartGrid.
 *
 * I'm writing this as a fully compile-time class for which the programmer sets grid
 * parameters as template arguments. This requires C++-20 which permits floating point
 * values (and the values of user-defined classes) to be passed as template args.
 *
 * \author Seb James
 * \date February 2024
 */

#pragma once

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/CartDomains.h>

namespace morph {

    /*!
     * \brief A grid class to define a rectangular Cartesian grid of locations
     *
     * This class exists to provide coordinates for each element in a rectangular grid
     * along with neighbour relationships between the elements. The idea is that during
     * a computation in which you are using state variables from arrays (or vectors or
     * vvecs or whatever) that define some kind of spatial field, you can retrieve the
     * coordinates that relate to element i and also the coordinates (or index i or
     * existence) of the neighbour to the 'East', 'West', 'North' etc.
     *
     * This class allows you to specify (at compile time) the dimensions of the
     * rectangular grid, specifying the number of elements on each side of the grid and
     * the inter-element distances. You can also specify and offset coordinate of the
     * zeroth element of your grid, so that all the locations you retrieve become offset
     * (this is useful for specifying zero-centred grids). Two more template arguments
     * are important for specifying the coordinates that a given index in your arrays
     * maps to. One is whether or not the grid should be considered to be 'wrappable' -
     * if horizontally wrappable, then the neighbour-to-the-east of the east-most
     * element is the west-most element in the same row. The other is the 'element
     * order'. You could index a square grid by starting at the bottom left, counting to
     * the right and then moving up in the y direction for the next row. You could
     * equally define your indices to start at the top left and count to the right and
     * down for each row. These are 'row-major' schemes. Either is an option for this
     * class. Column-major schemes are possible, but omitted for now (they would not be
     * difficult to code up).
     *
     * Lastly, there is a template argument to choose whether to compute each element
     * when instantiating the class (which takes a few milliseconds on my laptop) and
     * then look up the coordinate from memory when requested. This is the default as it
     * appears to be faster. The alternative is to always compute the coordinate when it
     * is requested, which results in (almost) no class instatiation time.
     *
     * \tparam n_x Number of elements that the grid is wide
     *
     * \tparam n_y Number of elements that the grid is high
     *
     * \tparam dx A two element morph::vec providing the horizontal distance between
     * horizontally adjacent grid element centres (element 0) and the vertical distance
     * between vertically adjacent grid element centres (element 1).
     *
     * \tparam g_offset A vector giving the distance offset (in your chosen units) to
     * Grid index 0.
     *
     * \tparam memory_coords If true, then populate two vvecs with the x and y
     * coordinates for each grid element. The vvecs are called Grid<>::d_x and
     * Grid<>::d_y. On your compute architecture, it may be faster to retrieve an
     * element's position information by memory lookup than by carrying out the
     * computation. To test, you can run the program morphologica/tests/profileGrid
     *
     * \tparam d_wrap An enum to set how the grid wraps. Affects neighbour relationships
     *
     * \tparam g_order The index order. Always counting left to right (row-major), but
     * do you start on the top row or the bottom row (the default)?
     */
    template < size_t n_x, size_t n_y,
               morph::vec<float, 2> dx = { 1.0f, 1.0f },
               morph::vec<float, 2> g_offset = { 0.0f, 0.0f },
               bool memory_coords = true,
               CartDomainWrap d_wrap = CartDomainWrap::None,
               GridOrder g_order = morph::GridOrder::bottomleft_to_topright >
    struct Grid
    {
        //! The number of elements in the grid
        static constexpr size_t n = n_x * n_y;

        //! Constructor only required to populate d_x/d_y
        Grid()
        {
            if constexpr (memory_coords == true) {
                this->d_x.resize (n);
                this->d_y.resize (n);
                morph::vec<float, 2> c = { 0.0f, 0.0f };
                for (size_t i = 0; i < n; ++i) {
                    c = this->coord (i);
                    this->d_x[i] = c[0];
                    this->d_y[i] = c[1];
                }
            }
        }

        // Indexing the grid will return a computed (or memorized) vec location.
        constexpr morph::vec<float, 2> operator[] (const size_t index) const
        {
            if constexpr (memory_coords == true) {
                return morph::vec<float, 2>({ this->d_x[index], this->d_y[index] });
            } else {
                return this->coord (index);
            }
        }

        //! Return the coordinate with the given index
        constexpr morph::vec<float, 2> coord (const size_t index) const
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
        constexpr size_t index_ne (const size_t index) const
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
        constexpr morph::vec<float, 2> coord_ne (const size_t index) const
        {
            size_t idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the east
        constexpr bool has_ne (const size_t index) const { return index_ne (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<size_t>::max()
        constexpr size_t index_nw (const size_t index) const
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
        constexpr morph::vec<float, 2> coord_nw (const size_t index) const
        {
            size_t idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the west
        constexpr bool has_nw (const size_t index) const { return index_nw (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<size_t>::max()
        constexpr size_t index_nn (const size_t index) const
        {
            size_t c = this->col (index);
            if constexpr (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == (n_y - 1) && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == (n_y - 1) && (d_wrap == CartDomainWrap::Vertical || d_wrap == CartDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
                }
            } else if constexpr (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == 0 && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == 0 && (d_wrap == CartDomainWrap::Vertical || d_wrap == CartDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
                }
            } else {
                []<bool flag = false>() { static_assert(flag, "unexpected value for g_order template arg"); }();
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_nn (const size_t index) const
        {
            size_t idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the north
        constexpr bool has_nn (const size_t index) const { return index_nn (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<size_t>::max()
        constexpr size_t index_ns (const size_t index) const
        {
            size_t c = this->col (index);
            if constexpr (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == 0 && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == 0 && (d_wrap == CartDomainWrap::Vertical || d_wrap == CartDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
                }
            } else if constexpr (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == (n_y - 1) && (d_wrap == CartDomainWrap::None || d_wrap == CartDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == (n_y - 1) && (d_wrap == CartDomainWrap::Vertical || d_wrap == CartDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
                }
            } else {
                []<bool flag = false>() { static_assert(flag, "unexpected value for g_order template arg"); }();
            }
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {float_max, float_max}
        constexpr morph::vec<float, 2> coord_ns (const size_t index) const
        {
            size_t idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the south
        constexpr bool has_ns (const size_t index) const { return index_ns (index) != std::numeric_limits<size_t>::max(); }

        //! Neighbour north east
        constexpr bool has_nne (const size_t index) const { return has_ne (index) && has_nn (index); }
        constexpr size_t index_nne (const size_t index) const
        {
            size_t nn = this->index_nn (index);
            return nn < n ? index_ne (nn) : std::numeric_limits<size_t>::max();
        }
        constexpr morph::vec<float, 2> coord_nne (const size_t index) const
        {
            size_t idx = this->index_nne (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour north west
        constexpr bool has_nnw (const size_t index) const { return has_nw (index) && has_nn (index); }
        constexpr size_t index_nnw (const size_t index) const
        {
            size_t nn = this->index_nn (index);
            return nn < n ? index_nw (nn) : std::numeric_limits<size_t>::max();
        }
        constexpr morph::vec<float, 2> coord_nnw (const size_t index) const
        {
            size_t idx = this->index_nnw (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour south east
        constexpr bool has_nse (const size_t index) const { return has_ne (index) && has_ns (index); }
        constexpr size_t index_nse (const size_t index) const
        {
            size_t ns = this->index_ns (index);
            return ns < n ? index_ne (ns) : std::numeric_limits<size_t>::max();
        }
        constexpr morph::vec<float, 2> coord_nse (const size_t index) const
        {
            size_t idx = this->index_nse (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour south west
        constexpr bool has_nsw (const size_t index) const { return has_nw (index) && has_ns (index); }
        constexpr size_t index_nsw (const size_t index) const
        {
            size_t ns = this->index_ns (index);
            return ns < n ? index_nw (ns) : std::numeric_limits<size_t>::max();
        }
        constexpr morph::vec<float, 2> coord_nsw (const size_t index) const
        {
            size_t idx = this->index_nsw (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        /*!
         * Return the distance from the centre of the left element column to the centre of the
         * right element column
         *
         * A note on widths:
         *
         * What is the width of a grid? Is it the distance from the centre of the left-most pixel to
         * the centre of the right-most pixel, OR is it the distance from the left edge of the
         * left-most pixel to the right edge of the right-most pixel? It could be either, so I
         * provide width() and width_of_pixels() as well as height() and height_of_pixels().
         */
        constexpr float width() const { return dx[0] * n_x; }

        //! Return the width of the grid if drawn as pixels
        constexpr float width_of_pixels() const { return dx[0] * n_x + dx[0]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        constexpr float height() const { return dx[1] * n_y; }

        constexpr float area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        constexpr float height_of_pixels() const { return dx[1] * n_y + dx[1]; }

        //! Return the area of the grid, if drawn as pixels
        constexpr float area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        constexpr float xmin() const { return (*this)[0][0]; }
        constexpr float xmax() const { return (*this)[n_x-1][0]; }
        constexpr float ymin() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[0][1] : (*this)[n_x * (n_y-1)][1]; }
        constexpr float ymax() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[n_x * (n_y-1)][1] : (*this)[0][1]; }

        //! Extents {xmin, xmax, ymin, ymax}
        constexpr morph::vec<float, 4> extents() const { return morph::vec<float, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        constexpr morph::vec<float, 2> centre() const { return morph::vec<float, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }

        //! Return the row for the index
        constexpr size_t row (const size_t index) const { return index < n ? index % n_x : std::numeric_limits<size_t>::max(); }

        //! Return the col for the index
        constexpr size_t col (const size_t index) const { return index < n ? index / n_x : std::numeric_limits<size_t>::max(); }

        //! Two vector structures that contains the coords for this grid. Populated only if template arg
        //! memory_coords is true.
        morph::vvec<float> d_x;
        morph::vvec<float> d_y;
    };

} // namespace morph
