/*!
 * \file
 *
 * This file contains the definition for morph::Gridct<>, a simple Cartesian Grid class. It is much
 * simpler than morph::CartGrid because there is no option to define an arbitrary boundary to your
 * domain. This code is a few hundred lines of code versus about 2600 lines in CartGrid.
 *
 * I'm writing this as a fully compile-time class for which the programmer sets grid parameters as
 * template arguments. This requires C++-20 which permits floating point values (and the values of
 * user-defined classes) to be passed as template args.
 *
 * The runtime-configurable version of this class, morph::Grid, is more convenient to code with.
 *
 * \author Seb James
 * \date February 2024
 */

#pragma once

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/GridFeatures.h>

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
     * This class allows you to specify (at compile time, hence the 'ct' in the class name) the
     * dimensions of the rectangular grid, specifying the number of elements on each side of the
     * grid and the inter-element distances. You can also specify and offset coordinate of the
     * zeroth element of your grid, so that all the locations you retrieve become offset (this is
     * useful for specifying zero-centred grids). Two more template arguments are important for
     * specifying the coordinates that a given index in your arrays maps to. One is whether or not
     * the grid should be considered to be 'wrappable' - if horizontally wrappable, then the
     * neighbour-to-the-east of the east-most element is the west-most element in the same row. The
     * other is the 'element order'. You could index a square grid by starting at the bottom left,
     * counting to the right and then moving up in the y direction for the next row. You could
     * equally define your indices to start at the top left and count to the right and down for each
     * row. These are 'row-major' schemes. Either is an option for this class. Column-major schemes
     * are possible, but omitted for now (they would not be difficult to code up).
     *
     * Lastly, there is a template argument to choose whether to compute each element
     * when instantiating the class (which takes a few milliseconds on my laptop) and
     * then look up the coordinate from memory when requested. This is the default as it
     * appears to be faster. The alternative is to always compute the coordinate when it
     * is requested, which results in (almost) no class instatiation time.
     *
     * \tparam I The Index type. unsigned int is a good choice.
     *
     * \tparam C The coordinate type. float is a good choice.
     *
     * \tparam w Number of elements that the grid is wide
     *
     * \tparam h Number of elements that the grid is high
     *
     * \tparam dx A two element morph::vec providing the horizontal distance between
     * horizontally adjacent grid element centres (element 0) and the vertical distance
     * between vertically adjacent grid element centres (element 1).
     *
     * \tparam offset A vector giving the distance offset (in your chosen units) to
     * Grid index 0.
     *
     * \tparam memory_coords If true, then populate two vvecs with the x and y
     * coordinates for each grid element. The vvecs are called Gridct<>::v_x and
     * Gridct<>::v_y. On your compute architecture, it may be faster to retrieve an
     * element's position information by memory lookup than by carrying out the
     * computation. To test, you can run the program morphologica/tests/profileGrid. NB:
     * *neighbour* relationships are always computed (though they could in principle be
     * stored in d_ vectors as they are in CartGrid).
     *
     * \tparam wrap An enum to set how the grid wraps. Affects neighbour relationships
     *
     * \tparam order The index order. Always counting left to right (row-major), but
     * do you start on the top row or the bottom row (the default)?
     */
    template < typename I, typename C, I w, I h,
               morph::vec<C, 2> dx = { 1.0f, 1.0f },
               morph::vec<C, 2> offset = { 0.0f, 0.0f },
               bool memory_coords = true,
               GridDomainWrap wrap = GridDomainWrap::None,
               GridOrder order = morph::GridOrder::bottomleft_to_topright >
    struct Gridct
    {
        //! The number of elements in the grid
        static constexpr I n = w * h;

        // Getters as an interface
        constexpr I get_w() const { return w; }
        constexpr I get_h() const { return h; }
        constexpr morph::vec<I, 2> get_dims() const { return morph::vec<I, 2>({w, h}); }
        constexpr morph::vec<C, 2> get_dx() const { return dx; }
        constexpr morph::vec<C, 2> get_offset() const { return offset; }
        constexpr GridDomainWrap get_wrap() const { return wrap; }
        constexpr GridOrder get_order() const { return order; }

        //! Constructor only required to populate v_x/v_y
        Gridct()
        {
            static_assert (std::numeric_limits<I>::is_integer, "The index type I must be an integer type");
            static_assert (std::numeric_limits<C>::is_signed,
                           "The coordinate type C must be an signed type (floating point or integer)");

            if constexpr (memory_coords == true) {
                this->v_x.resize (n);
                this->v_y.resize (n);
                morph::vec<C, 2> c = { 0.0f, 0.0f };
                for (I i = 0; i < n; ++i) {
                    c = this->coord (i);
                    this->v_x[i] = c[0];
                    this->v_y[i] = c[1];
                }
            }
        }

        // Indexing the grid will return a computed (or memorized) vec location.
        constexpr morph::vec<C, 2> operator[] (const I index) const
        {
            if constexpr (memory_coords == true) {
                return morph::vec<C, 2>({ this->v_x[index], this->v_y[index] });
            } else {
                return this->coord (index);
            }
        }

        //! Return the coordinate with the given index
        constexpr morph::vec<C, 2> coord (const I index) const
        {
            if (index >= n) { return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()}); }
            morph::vec<C, 2> loc = offset;
            loc[0] += dx[0] * (index % w);
            if constexpr (order == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += dx[1] * (index / w);
            } else {
                loc[1] -= dx[1] * (index / w);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<I>::max()
        constexpr I index_ne (const I index) const
        {
            I c = this->col (index);
            if (c == (w - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (c == (w - 1) && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index - (w - 1);
            } else {
                return index + 1;
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {C_max, C_max}
        constexpr morph::vec<C, 2> coord_ne (const I index) const
        {
            I idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the east
        constexpr bool has_ne (const I index) const { return index_ne (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<I>::max()
        constexpr I index_nw (const I index) const
        {
            I c = this->col (index);
            if (c == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (c == 0 && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index + (w - 1);
            } else {
                return index - 1;
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {C_max, C_max}
        constexpr morph::vec<C, 2> coord_nw (const I index) const
        {
            I idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the west
        constexpr bool has_nw (const I index) const { return index_nw (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<I>::max()
        constexpr I index_nn (const I index) const
        {
            I r = this->row (index);
            if constexpr (order == morph::GridOrder::bottomleft_to_topright) {
                if (r == (h - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - 1) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index - (w * (h - 1));
                } else {
                    return index + w;
                }
            } else if constexpr (order == morph::GridOrder::topleft_to_bottomright) {
                if (r == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - 1));
                } else {
                    return index - w;
                }
            } else {
                []<bool flag = false>() { static_assert(flag, "unexpected value for order template arg"); }();
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {C_max, C_max}
        constexpr morph::vec<C, 2> coord_nn (const I index) const
        {
            I idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the north
        constexpr bool has_nn (const I index) const { return index_nn (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<I>::max()
        constexpr I index_ns (const I index) const
        {
            I r = this->row (index);
            if constexpr (order == morph::GridOrder::bottomleft_to_topright) {
                if (r == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - 1));
                } else {
                    return index - w;
                }
            } else if constexpr (order == morph::GridOrder::topleft_to_bottomright) {
                if (r == (h - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - 1) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index - (w * (h - 1));
                } else {
                    return index + w;
                }
            } else {
                []<bool flag = false>() { static_assert(flag, "unexpected value for order template arg"); }();
            }
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {C_max, C_max}
        constexpr morph::vec<C, 2> coord_ns (const I index) const
        {
            I idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the south
        constexpr bool has_ns (const I index) const { return index_ns (index) != std::numeric_limits<I>::max(); }

        //! Neighbour north east
        constexpr bool has_nne (const I index) const { return has_ne (index) && has_nn (index); }
        constexpr I index_nne (const I index) const
        {
            I nn = this->index_nn (index);
            return nn < n ? index_ne (nn) : std::numeric_limits<I>::max();
        }
        constexpr morph::vec<C, 2> coord_nne (const I index) const
        {
            I idx = this->index_nne (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour north west
        constexpr bool has_nnw (const I index) const { return has_nw (index) && has_nn (index); }
        constexpr I index_nnw (const I index) const
        {
            I nn = this->index_nn (index);
            return nn < n ? index_nw (nn) : std::numeric_limits<I>::max();
        }
        constexpr morph::vec<C, 2> coord_nnw (const I index) const
        {
            I idx = this->index_nnw (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour south east
        constexpr bool has_nse (const I index) const { return has_ne (index) && has_ns (index); }
        constexpr I index_nse (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_ne (ns) : std::numeric_limits<I>::max();
        }
        constexpr morph::vec<C, 2> coord_nse (const I index) const
        {
            I idx = this->index_nse (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour south west
        constexpr bool has_nsw (const I index) const { return has_nw (index) && has_ns (index); }
        constexpr I index_nsw (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_nw (ns) : std::numeric_limits<I>::max();
        }
        constexpr morph::vec<C, 2> coord_nsw (const I index) const
        {
            I idx = this->index_nsw (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
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
        constexpr C width() const { return dx[0] * (w - I{1}); }

        //! Return the width of the grid if drawn as pixels
        constexpr C width_of_pixels() const { return dx[0] * w; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        constexpr C height() const { return dx[1] * (h - I{1}); }

        constexpr C area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        constexpr C height_of_pixels() const { return dx[1] * h; }

        //! Return the area of the grid, if drawn as pixels
        constexpr C area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        constexpr C xmin() const { return (*this)[0][0]; }
        constexpr C xmax() const { return (*this)[w-1][0]; }
        constexpr C ymin() const { return order == GridOrder::bottomleft_to_topright ? (*this)[0][1] : (*this)[w * (h-1)][1]; }
        constexpr C ymax() const { return order == GridOrder::bottomleft_to_topright ? (*this)[w * (h-1)][1] : (*this)[0][1]; }

        //! Extents {xmin, xmax, ymin, ymax}
        constexpr morph::vec<C, 4> extents() const { return morph::vec<C, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        constexpr morph::vec<C, 2> centre() const { return morph::vec<C, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }

        //! Return the row for the index
        constexpr I row (const I index) const { return index < n ? index / w : std::numeric_limits<I>::max(); }

        //! Return the col for the index
        constexpr I col (const I index) const { return index < n ? index % w : std::numeric_limits<I>::max(); }

        //! Two vector structures that contains the coords for this grid. Populated only if template arg
        //! memory_coords is true.
        morph::vvec<C> v_x;
        morph::vvec<C> v_y;
    };

} // namespace morph
