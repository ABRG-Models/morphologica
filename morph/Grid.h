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
#include <morph/vvec.h>
#include <morph/CartDomains.h>

namespace morph {

    /*!
     * A grid class. This provides coordinates for each element in a grid along with neighbour
     * relationships. The data associated with the grid is expected to be stored in an array or
     * vector. Use the index of the array/vector to obtain info about the grid locations. The size
     * of the grid is not expected to change and so is provided as template args
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
     * \tparam memory_coords If true, then populate some vvecs with the coordinates
     *
     * \tparam d_wrap An enum to set how the grid wraps. Affects neighbour relationships
     *
     * \tparam g_order The index order. Always counting left to right, but do you start on the top row or
     * the bottom row (the default)?
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

        //! Constructor only required to populate memory_coords
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

        /*
         * What is the width of a grid? Is it the distance from the centre of the left-most pixel to
         * the centre of the right-most pixel, OR is it the distance from the left edge of the
         * left-most pixel to the right edge of the right-most pixel? It could be either, so I
         * provide width() and width_of_pixels() as well as height() and height_of_pixels().
         */

        //! Return the distance from the centre of the left element column to the centre of the
        //! right element column
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

        constexpr morph::vec<float, 2> centre() const { return morph::vec<float, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }


        //! Return the row for the index
        constexpr size_t row (const size_t index) const { return index < n ? index % n_x : std::numeric_limits<size_t>::max(); }

        //! Return the col for the index
        constexpr size_t col (const size_t index) const { return index < n ? index / n_x : std::numeric_limits<size_t>::max(); }

        //! Output some text information about the hexgrid.
        constexpr std::string output() const
        {
            std::stringstream ss;
            ss << n_x << " x " << n_y << " grid (" << n << " elements).";
            return ss.str();
        }

        //! Two vector structures that contains the coords for this grid. Populated only if template arg
        //! memory_coords is true.
        morph::vvec<float> d_x;
        morph::vvec<float> d_y;
    };

} // namespace morph
