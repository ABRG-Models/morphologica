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
     * \tparam T The type for storing and retrieving coordinates. Probably floating point, but could
     * be a signed integer type.
     *
     * \tparam I The integer type for specifying/retrieving number of pixels etc
     */
    template<typename T = float, typename I = unsigned int>
    struct Grid
    {
    private:
        // Members that are template arguments in morph::Grid. Set only via constructor or setters

        //! n_x Number of elements that the grid is wide
        I n_x = 1;
        //! n_y Number of elements that the grid is high
        I n_y = 1;
        //! dx A two element morph::vec providing the horizontal distance between
        //! horizontally adjacent grid element centres (element 0) and the vertical
        //! distance between vertically adjacent grid element centres (element 1).
        morph::vec<T, 2> dx = { T{1}, T{1} };
        //! g_offset A vector giving the distance offset (in your chosen units) to Grid
        //! index 0.
        morph::vec<T, 2> g_offset = { T{0}, T{0} };
        //! d_wrap An enum to set how the grid wraps. Affects neighbour relationships
        GridDomainWrap d_wrap = GridDomainWrap::None;
        //! g_order The index order. Always counting left to right (row-major), but do
        //! you start on the top row or the bottom row (the default)?
        GridOrder g_order = morph::GridOrder::bottomleft_to_topright;

    public:
        //! Setter for n_x
        void set_n_x (const I _n_x) { this->n_x = _n_x; this->init(); }
        //! Setter for n_y
        void set_n_y (const I _n_y) { this->n_y = _n_y; this->init(); }
        //! Setter for dx
        void set_dx (const morph::vec<T, 2> _dx) { this->dx = _dx; this->init(); }
        //! Setter for g_offset
        void set_g_offset (const morph::vec<T, 2> _g_offset) { this->g_offset = _g_offset; this->init(); }

        //! Setter for most of the grid parameters to be carried out all in one function
        void set_grid_params (const morph::vec<I, 2> dims,
                              const morph::vec<T, 2> spacing,
                              const morph::vec<T, 2> grid_offset)
        {
            this->n_x = dims[0];
            this->n_y = dims[1];
            this->dx = spacing;
            this->g_offset = grid_offset;
            this->init();
        }

        // Note: No setters for d_wrap or g_order. I'm assuming noone will want to change these at runtime

        // Getters
        I get_n_x() const { return this->n_x; }
        I get_n_y() const { return this->n_y; }
        morph::vec<I, 2> get_dims() const { return morph::vec<I, 2>({this->n_x, this->n_y}); }
        morph::vec<T, 2> get_dx() const { return this->dx; }
        morph::vec<T, 2> get_g_offset() const { return this->g_offset; }
        GridDomainWrap get_d_wrap() const { return this->d_wrap; }
        GridOrder get_g_order() const { return this->g_order; }

        //! The number of elements in the grid. Public, but don't change it manually.
        I n = n_x * n_y;

        //! Constructor
        Grid (const I _n_x, const I _n_y,
              const morph::vec<T, 2> _dx = { T{1}, T{1} },
              const morph::vec<T, 2> _g_offset = { T{0}, T{0} },
              const GridDomainWrap _d_wrap = GridDomainWrap::None,
              const GridOrder _g_order = morph::GridOrder::bottomleft_to_topright)
            : n_x(_n_x)
            , n_y(_n_y)
            , dx(_dx)
            , g_offset(_g_offset)
            , d_wrap(_d_wrap)
            , g_order(_g_order)
        {
            this->init();
        }

        //! Set up memory and populate d_x/d_y. Called if parameters n_x, n_y, g_offset
        //! or g_order change. Does not need to change if d_wrap changes, as neighbour
        //! relationships are always runtime computed.
        void init()
        {
            static_assert (std::numeric_limits<I>::is_integer, "I must be an integer type");
            static_assert (std::numeric_limits<T>::is_signed, "T must be an signed type (floating point or integer)");

            if constexpr (std::is_signed_v<I> == true) {

                if (this->n_x < I{0} || this->n_y < I{0}) {
                    std::stringstream ee;
                    throw std::runtime_error ("Specify your grid with positive width and height");
                }

                long long int test = this->n_x * this->n_y;
                if (test > std::numeric_limits<I>::max()) {
                    std::stringstream ee;
                    ee << "Use a larger capacity type for I if you need a grid of size "
                       << n_x << " * " << n_y;
                    throw std::runtime_error (ee.str());
                }

            } else {
                unsigned long long int test = this->n_x * this->n_y;
                if (test > std::numeric_limits<I>::max()) {
                    std::stringstream ee;
                    ee << "Use a larger capacity type for I if you need a grid of size "
                       << n_x << " * " << n_y;
                    throw std::runtime_error (ee.str());
                }
            }
            this->n = this->n_x * this->n_y;
            this->d_x.resize (this->n);
            this->d_y.resize (this->n);
            morph::vec<T, 2> c = { T{0}, T{0} };
            for (I i = 0; i < this->n; ++i) {
                c = this->coord (i);
                this->d_x[i] = c[0];
                this->d_y[i] = c[1];
            }
        }

        //! Indexing the grid will return a memorized vec location.
        morph::vec<T, 2> operator[] (const I index) const
        {
            return morph::vec<T, 2>({ this->d_x[index], this->d_y[index] });
        }

        //! A named function that does the same as operator[]
        morph::vec<T, 2> coord_lookup (const I index) const
        {
            return morph::vec<T, 2>({ this->d_x[index], this->d_y[index] });
        }

        //! Compute and return the coordinate with the given index
        morph::vec<T, 2> coord (const I index) const
        {
            if (index >= this->n) { return morph::vec<T, 2>({std::numeric_limits<T>::max(), std::numeric_limits<T>::max()}); }
            morph::vec<T, 2> loc = this->g_offset;
            loc[0] += this->dx[0] * (index % this->n_x);
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += this->dx[1] * (index / this->n_x);
            } else {
                loc[1] -= this->dx[1] * (index / this->n_x);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<I>::max()
        I index_ne (const I index) const
        {
            I r = this->row (index);
            if (r == (n_x - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (r == (n_x - 1) && (d_wrap == GridDomainWrap::Horizontal || d_wrap == GridDomainWrap::Both)) {
                return index - (n_x - 1);
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
            if (r == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (r == 0 && (d_wrap == GridDomainWrap::Horizontal || d_wrap == GridDomainWrap::Both)) {
                return index + (n_x - 1);
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
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == (n_y - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == (n_y - 1) && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
                }
            } else if (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == 0 && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
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
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == 0 && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
                }
            } else if (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == (n_y - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (c == (n_y - 1) && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
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
        T width() const { return dx[0] * n_x; }

        //! Return the width of the grid if drawn as pixels
        T width_of_pixels() const { return dx[0] * n_x + dx[0]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        T height() const { return dx[1] * n_y; }

        T area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        T height_of_pixels() const { return dx[1] * n_y + dx[1]; }

        //! Return the area of the grid, if drawn as pixels
        T area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        T xmin() const { return (*this)[0][0]; }
        T xmax() const { return (*this)[n_x-1][0]; }
        T ymin() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[0][1] : (*this)[n_x * (n_y-1)][1]; }
        T ymax() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[n_x * (n_y-1)][1] : (*this)[0][1]; }

        //! Extents {xmin, xmax, ymin, ymax}
        morph::vec<T, 4> extents() const { return morph::vec<T, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        morph::vec<T, 2> centre() const { return morph::vec<T, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }

        //! Return the row for the index
        I row (const I index) const { return index < n ? index % n_x : std::numeric_limits<I>::max(); }

        //! Return the col for the index
        I col (const I index) const { return index < n ? index / n_x : std::numeric_limits<I>::max(); }

        //! Two vector structures that contains the coords for this grid.
        morph::vvec<T> d_x;
        morph::vvec<T> d_y;
    };

} // namespace morph
