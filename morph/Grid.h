/*!
 * \file
 *
 * This file contains the definition for morph::Grid, a simple Cartesian Grid
 * class. This is the runtime-modifiable version of morph::Gridct<>
 *
 * \author Seb James
 * \date February 2024
 */

#pragma once

#include <stdexcept>
#include <sstream>
#include <limits>
#include <type_traits>
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
     * This class allows you to specify (and change at run-time) the dimensions of the
     * rectangular grid, specifying the number of elements on each side of the grid and
     * the inter-element distances. You can also specify an offset coordinate of the
     * zeroth element of your grid, so that all the locations you retrieve become offset
     * (this is useful for specifying zero-centred grids). Two more arguments are
     * important for specifying the coordinates that a given index in your arrays maps
     * to. One is whether or not the grid should be considered to be 'wrappable' - if
     * horizontally wrappable, then the neighbour-to-the-east of the east-most element
     * is the west-most element in the same row. The other is the 'element order'. You
     * could index a square grid by starting at the bottom left, counting to the right
     * and then moving up in the y direction for the next row. You could equally define
     * your indices to start at the top left and count to the right and down for each
     * row. These are 'row-major' schemes. Either is an option for this
     * class. Column-major schemes are possible, but omitted for now (they would not be
     * difficult to code up).
     *
     * Use this class instead of morph::Gridct<> if you need to change the grid parameters
     * at runtime. For example, with Grid, you could model an expanding domain by
     * changing Grid::dx over time. You could model a shift in the cooordinates that
     * your grid maps by changing Grid::g_offset.
     *
     * \tparam I The integer type for specifying/retrieving number of pixels etc
     *
     * \tparam T The type for storing and retrieving coordinates. Probably floating point, but could
     * be a signed integer type.
     *
     */
    template<typename I = unsigned int, typename T = float>
    struct Grid
    {
    private:
        // Members that are template arguments in morph::Grid. Set only via constructor or setters

        //! w Number of elements that the grid is wide
        I w = 1;
        //! h Number of elements that the grid is high
        I h = 1;
        //! dx A two element morph::vec providing the horizontal distance between
        //! horizontally adjacent grid element centres (element 0) and the vertical
        //! distance between vertically adjacent grid element centres (element 1).
        morph::vec<T, 2> dx = { T{1}, T{1} };
        //! offset A vector giving the distance offset (in your chosen units) to Grid
        //! index 0.
        morph::vec<T, 2> offset = { T{0}, T{0} };
        //! wrap An enum to set how the grid wraps. Affects neighbour relationships
        GridDomainWrap wrap = GridDomainWrap::None;
        //! order The index order. Always counting left to right (row-major), but do
        //! you start on the top row or the bottom row (the default)?
        GridOrder order = morph::GridOrder::bottomleft_to_topright;

    public:
        //! Setter for w
        void set_w (const I _w) { this->w = _w; this->init(); }
        //! Setter for h
        void set_h (const I _h) { this->h = _h; this->init(); }
        //! Setter for dx
        void set_dx (const morph::vec<T, 2> _dx) { this->dx = _dx; this->init(); }
        //! Setter for g_offset
        void set_offset (const morph::vec<T, 2> _offset) { this->offset = _offset; this->init(); }

        //! Setter for most of the grid parameters to be carried out all in one function
        void set_grid_params (const morph::vec<I, 2> dims,
                              const morph::vec<T, 2> spacing,
                              const morph::vec<T, 2> grid_offset)
        {
            this->w = dims[0];
            this->h = dims[1];
            this->dx = spacing;
            this->offset = grid_offset;
            this->init();
        }

        // Note: No setters for wrap or order. I'm assuming noone will want to change these at runtime

        // Getters
        I get_w() const { return this->w; }
        I get_h() const { return this->h; }
        morph::vec<I, 2> get_dims() const { return morph::vec<I, 2>({this->w, this->h}); }
        morph::vec<T, 2> get_dx() const { return this->dx; }
        morph::vec<T, 2> get_offset() const { return this->offset; }
        GridDomainWrap get_wrap() const { return this->wrap; }
        GridOrder get_order() const { return this->order; }

        //! The number of elements in the grid. Public, but don't change it manually.
        I n = w * h;

        //! Constructor
        Grid (const I _w, const I _h,
              const morph::vec<T, 2> _dx = { T{1}, T{1} },
              const morph::vec<T, 2> _offset = { T{0}, T{0} },
              const GridDomainWrap _wrap = GridDomainWrap::None,
              const GridOrder _order = morph::GridOrder::bottomleft_to_topright)
            : w(_w)
            , h(_h)
            , dx(_dx)
            , offset(_offset)
            , wrap(_wrap)
            , order(_order)
        {
            this->init();
        }

        //! Set up memory and populate v_x/v_y. Called if parameters w, h, offset
        //! or order change. Does not need to change if wrap changes, as neighbour
        //! relationships are always runtime computed.
        void init()
        {
            static_assert (std::numeric_limits<I>::is_integer, "I must be an integer type");
            static_assert (std::numeric_limits<T>::is_signed, "T must be an signed type (floating point or integer)");

            if constexpr (std::is_signed_v<I> == true) {

                if (this->w < I{0} || this->h < I{0}) {
                    std::stringstream ee;
                    throw std::runtime_error ("Specify your grid with positive width and height");
                }

                long long int test = this->w * this->h;
                if (test > std::numeric_limits<I>::max()) {
                    std::stringstream ee;
                    ee << "Use a larger capacity type for I if you need a grid of size "
                       << w << " * " << h;
                    throw std::runtime_error (ee.str());
                }

            } else {
                unsigned long long int test = this->w * this->h;
                if (test > std::numeric_limits<I>::max()) {
                    std::stringstream ee;
                    ee << "Use a larger capacity type for I if you need a grid of size "
                       << w << " * " << h;
                    throw std::runtime_error (ee.str());
                }
            }
            this->n = this->w * this->h;
            this->v_x.resize (this->n);
            this->v_y.resize (this->n);
            morph::vec<T, 2> c = { T{0}, T{0} };
            for (I i = 0; i < this->n; ++i) {
                c = this->coord (i);
                this->v_x[i] = c[0];
                this->v_y[i] = c[1];
            }
        }

        //! Indexing the grid will return a memorized vec location.
        morph::vec<T, 2> operator[] (const I index) const
        {
            return morph::vec<T, 2>({ this->v_x[index], this->v_y[index] });
        }

        //! A named function that does the same as operator[]
        morph::vec<T, 2> coord_lookup (const I index) const
        {
            return morph::vec<T, 2>({ this->v_x[index], this->v_y[index] });
        }

        //! Compute and return the coordinate with the given index
        morph::vec<T, 2> coord (const I index) const
        {
            if (index >= this->n) { return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()}); }
            morph::vec<T, 2> loc = this->offset;
            loc[0] += this->dx[0] * (index % this->w);
            if (order == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += this->dx[1] * (index / this->w);
            } else {
                loc[1] -= this->dx[1] * (index / this->w);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<I>::max()
        I index_ne (const I index) const
        {
            I r = this->row (index);
            if (r == (w - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (r == (w - 1) && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index - (w - 1);
            } else {
                return index + 1;
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {T_max, T_max}
        morph::vec<T, 2> coord_ne (const I index) const
        {
            I idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }
        //! Return true if the index has a neighbour to the east
        bool has_ne (const I index) const { return index_ne (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<I>::max()
        I index_nw (const I index) const
        {
            I r = this->row (index);
            if (r == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (r == 0 && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index + (w - 1);
            } else {
                return index - 1;
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {T_max, T_max}
        morph::vec<T, 2> coord_nw (const I index) const
        {
            I idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }
        //! Return true if the index has a neighbour to the west
        bool has_nw (const I index) const { return index_nw (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<I>::max()
        I index_nn (const I index) const
        {
            I c = this->col (index);
            if (order == morph::GridOrder::bottomleft_to_topright) {
                if (c == (h - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == (h - 1) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index - (w * (h - 1));
                } else {
                    return index + w;
                }
            } else if (order == morph::GridOrder::topleft_to_bottomright) {
                if (c == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - 1));
                } else {
                    return index - w;
                }
            } else {
                return std::numeric_limits<I>::max();
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {T_max, T_max}
        morph::vec<T, 2> coord_nn (const I index) const
        {
            I idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }
        //! Return true if the index has a neighbour to the north
        bool has_nn (const I index) const { return index_nn (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<I>::max()
        I index_ns (const I index) const
        {
            I c = this->col (index);
            if (order == morph::GridOrder::bottomleft_to_topright) {
                if (c == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - 1));
                } else {
                    return index - w;
                }
            } else if (order == morph::GridOrder::topleft_to_bottomright) {
                if (c == (h - 1) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == (h - 1) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index - (w * (h - 1));
                } else {
                    return index + w;
                }
            } else {
                return std::numeric_limits<I>::max();
            }
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {T_max, T_max}
        morph::vec<T, 2> coord_ns (const I index) const
        {
            I idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }
        //! Return true if the index has a neighbour to the south
        bool has_ns (const I index) const { return index_ns (index) != std::numeric_limits<I>::max(); }

        //! Neighbour north east
        bool has_nne (const I index) const { return has_ne (index) && has_nn (index); }
        I index_nne (const I index) const
        {
            I nn = this->index_nn (index);
            return nn < n ? index_ne (nn) : std::numeric_limits<I>::max();
        }
        morph::vec<T, 2> coord_nne (const I index) const
        {
            I idx = this->index_nne (index);
            return idx < n ? (*this)[idx] : morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }

        //! Neighbour north west
        bool has_nnw (const I index) const { return has_nw (index) && has_nn (index); }
        I index_nnw (const I index) const
        {
            I nn = this->index_nn (index);
            return nn < n ? index_nw (nn) : std::numeric_limits<I>::max();
        }
        morph::vec<T, 2> coord_nnw (const I index) const
        {
            I idx = this->index_nnw (index);
            return idx < n ? (*this)[idx] : morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }

        //! Neighbour south east
        bool has_nse (const I index) const { return has_ne (index) && has_ns (index); }
        I index_nse (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_ne (ns) : std::numeric_limits<I>::max();
        }
        morph::vec<T, 2> coord_nse (const I index) const
        {
            I idx = this->index_nse (index);
            return idx < n ? (*this)[idx] : morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
        }

        //! Neighbour south west
        bool has_nsw (const I index) const { return has_nw (index) && has_ns (index); }
        I index_nsw (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_nw (ns) : std::numeric_limits<I>::max();
        }
        morph::vec<T, 2> coord_nsw (const I index) const
        {
            I idx = this->index_nsw (index);
            return idx < n ? (*this)[idx] : morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()});
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
        T width() const { return dx[0] * w; }

        //! Return the width of the grid if drawn as pixels
        T width_of_pixels() const { return dx[0] * w + dx[0]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        T height() const { return dx[1] * h; }

        T area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        T height_of_pixels() const { return dx[1] * h + dx[1]; }

        //! Return the area of the grid, if drawn as pixels
        T area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        T xmin() const { return (*this)[0][0]; }
        T xmax() const { return (*this)[w-1][0]; }
        T ymin() const { return order == GridOrder::bottomleft_to_topright ? (*this)[0][1] : (*this)[w * (h-1)][1]; }
        T ymax() const { return order == GridOrder::bottomleft_to_topright ? (*this)[w * (h-1)][1] : (*this)[0][1]; }

        //! Extents {xmin, xmax, ymin, ymax}
        morph::vec<T, 4> extents() const { return morph::vec<T, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        morph::vec<T, 2> centre() const { return morph::vec<T, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }

        //! Return the row for the index
        I row (const I index) const { return index < n ? index % w : std::numeric_limits<I>::max(); }

        //! Return the col for the index
        I col (const I index) const { return index < n ? index / w : std::numeric_limits<I>::max(); }

        //! Two vector structures that contains the coords for this grid. v_x is a vector of the x coordinates
        morph::vvec<T> v_x;
        //! v_y is a vector of the y coordinates
        morph::vvec<T> v_y;
    };

} // namespace morph
