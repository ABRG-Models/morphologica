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
     */
    struct Grid
    {
    private:
        // Members that are template arguments in morph::Grid. Set only via constructor or setters

        //! n_x Number of elements that the grid is wide
        size_t n_x = 1;
        //! n_y Number of elements that the grid is high
        size_t n_y = 1;
        //! dx A two element morph::vec providing the horizontal distance between
        //! horizontally adjacent grid element centres (element 0) and the vertical
        //! distance between vertically adjacent grid element centres (element 1).
        morph::vec<float, 2> dx = { 1.0f, 1.0f };
        //! g_offset A vector giving the distance offset (in your chosen units) to Grid
        //! index 0.
        morph::vec<float, 2> g_offset = { 0.0f, 0.0f };
        //! d_wrap An enum to set how the grid wraps. Affects neighbour relationships
        GridDomainWrap d_wrap = GridDomainWrap::None;
        //! g_order The index order. Always counting left to right (row-major), but do
        //! you start on the top row or the bottom row (the default)?
        GridOrder g_order = morph::GridOrder::bottomleft_to_topright;

    public:
        //! Setter for n_x
        void set_n_x (const size_t _n_x) { this->n_x = _n_x; this->init(); }
        //! Setter for n_y
        void set_n_y (const size_t _n_y) { this->n_y = _n_y; this->init(); }
        //! Setter for dx
        void set_dx (const morph::vec<float, 2> _dx) { this->dx = _dx; this->init(); }
        //! Setter for g_offset
        void set_g_offset (const morph::vec<float, 2> _g_offset) { this->g_offset = _g_offset; this->init(); }

        //! Setter for most of the grid parameters to be carried out all in one function
        void set_grid_params (const morph::vec<size_t, 2> dims,
                              const morph::vec<float, 2> spacing,
                              const morph::vec<float, 2> grid_offset)
        {
            this->n_x = dims[0];
            this->n_y = dims[1];
            this->dx = spacing;
            this->g_offset = grid_offset;
            this->init();
        }

        // Note: No setters for d_wrap or g_order. I'm assuming noone will want to change these at runtime

        // Getters
        size_t get_n_x() const { return this->n_x; }
        size_t get_n_y() const { return this->n_y; }
        morph::vec<float, 2> get_dx() const { return this->dx; }
        morph::vec<float, 2> get_g_offset() const { return this->g_offset; }
        GridDomainWrap get_d_wrap() const { return this->d_wrap; }
        GridOrder get_g_order() const { return this->g_order; }

        //! The number of elements in the grid. Public, but don't change it manually.
        size_t n = n_x * n_y;

        //! Constructor
        Grid (const size_t _n_x, const size_t _n_y,
              const morph::vec<float, 2> _dx = { 1.0f, 1.0f },
              const morph::vec<float, 2> _g_offset = { 0.0f, 0.0f },
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
            this->n = this->n_x * this->n_y;
            this->d_x.resize (this->n);
            this->d_y.resize (this->n);
            morph::vec<float, 2> c = { 0.0f, 0.0f };
            for (size_t i = 0; i < this->n; ++i) {
                c = this->coord (i);
                this->d_x[i] = c[0];
                this->d_y[i] = c[1];
            }
        }

        //! Indexing the grid will return a memorized vec location.
        morph::vec<float, 2> operator[] (const size_t index) const
        {
            return morph::vec<float, 2>({ this->d_x[index], this->d_y[index] });
        }

        //! A named function that does the same as operator[]
        morph::vec<float, 2> coord_lookup (const size_t index) const
        {
            return morph::vec<float, 2>({ this->d_x[index], this->d_y[index] });
        }

        //! Compute and return the coordinate with the given index
        morph::vec<float, 2> coord (const size_t index) const
        {
            if (index >= this->n) { return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}); }
            morph::vec<float, 2> loc = this->g_offset;
            loc[0] += this->dx[0] * (index % this->n_x);
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += this->dx[1] * (index / this->n_x);
            } else {
                loc[1] -= this->dx[1] * (index / this->n_x);
            }
            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<size_t>::max()
        size_t index_ne (const size_t index) const
        {
            size_t r = this->row (index);
            if (r == (n_x - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<size_t>::max();
            } else if (r == (n_x - 1) && (d_wrap == GridDomainWrap::Horizontal || d_wrap == GridDomainWrap::Both)) {
                return index - (n_x - 1);
            } else {
                return index + 1;
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {float_max, float_max}
        morph::vec<float, 2> coord_ne (const size_t index) const
        {
            size_t idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the east
        bool has_ne (const size_t index) const { return index_ne (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<size_t>::max()
        size_t index_nw (const size_t index) const
        {
            size_t r = this->row (index);
            if (r == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<size_t>::max();
            } else if (r == 0 && (d_wrap == GridDomainWrap::Horizontal || d_wrap == GridDomainWrap::Both)) {
                return index + (n_x - 1);
            } else {
                return index - 1;
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {float_max, float_max}
        morph::vec<float, 2> coord_nw (const size_t index) const
        {
            size_t idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the west
        bool has_nw (const size_t index) const { return index_nw (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<size_t>::max()
        size_t index_nn (const size_t index) const
        {
            size_t c = this->col (index);
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == (n_y - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == (n_y - 1) && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
                }
            } else if (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == 0 && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
                }
            } else {
                return std::numeric_limits<size_t>::max();
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {float_max, float_max}
        morph::vec<float, 2> coord_nn (const size_t index) const
        {
            size_t idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the north
        bool has_nn (const size_t index) const { return index_nn (index) != std::numeric_limits<size_t>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<size_t>::max()
        size_t index_ns (const size_t index) const
        {
            size_t c = this->col (index);
            if (g_order == morph::GridOrder::bottomleft_to_topright) {
                if (c == 0 && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == 0 && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index + (n_x * (n_y - 1));
                } else {
                    return index - n_x;
                }
            } else if (g_order == morph::GridOrder::topleft_to_bottomright) {
                if (c == (n_y - 1) && (d_wrap == GridDomainWrap::None || d_wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<size_t>::max();
                } else if (c == (n_y - 1) && (d_wrap == GridDomainWrap::Vertical || d_wrap == GridDomainWrap::Both)) {
                    return index - (n_x * (n_y - 1));
                } else {
                    return index + n_x;
                }
            } else {
                return std::numeric_limits<size_t>::max();
            }
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {float_max, float_max}
        morph::vec<float, 2> coord_ns (const size_t index) const
        {
            size_t idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }
        //! Return true if the index has a neighbour to the south
        bool has_ns (const size_t index) const { return index_ns (index) != std::numeric_limits<size_t>::max(); }

        //! Neighbour north east
        bool has_nne (const size_t index) const { return has_ne (index) && has_nn (index); }
        size_t index_nne (const size_t index) const
        {
            size_t nn = this->index_nn (index);
            return nn < n ? index_ne (nn) : std::numeric_limits<size_t>::max();
        }
        morph::vec<float, 2> coord_nne (const size_t index) const
        {
            size_t idx = this->index_nne (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour north west
        bool has_nnw (const size_t index) const { return has_nw (index) && has_nn (index); }
        size_t index_nnw (const size_t index) const
        {
            size_t nn = this->index_nn (index);
            return nn < n ? index_nw (nn) : std::numeric_limits<size_t>::max();
        }
        morph::vec<float, 2> coord_nnw (const size_t index) const
        {
            size_t idx = this->index_nnw (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour south east
        bool has_nse (const size_t index) const { return has_ne (index) && has_ns (index); }
        size_t index_nse (const size_t index) const
        {
            size_t ns = this->index_ns (index);
            return ns < n ? index_ne (ns) : std::numeric_limits<size_t>::max();
        }
        morph::vec<float, 2> coord_nse (const size_t index) const
        {
            size_t idx = this->index_nse (index);
            return idx < n ? (*this)[idx] : morph::vec<float, 2>({std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
        }

        //! Neighbour south west
        bool has_nsw (const size_t index) const { return has_nw (index) && has_ns (index); }
        size_t index_nsw (const size_t index) const
        {
            size_t ns = this->index_ns (index);
            return ns < n ? index_nw (ns) : std::numeric_limits<size_t>::max();
        }
        morph::vec<float, 2> coord_nsw (const size_t index) const
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
        float width() const { return dx[0] * n_x; }

        //! Return the width of the grid if drawn as pixels
        float width_of_pixels() const { return dx[0] * n_x + dx[0]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        float height() const { return dx[1] * n_y; }

        float area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        float height_of_pixels() const { return dx[1] * n_y + dx[1]; }

        //! Return the area of the grid, if drawn as pixels
        float area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        float xmin() const { return (*this)[0][0]; }
        float xmax() const { return (*this)[n_x-1][0]; }
        float ymin() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[0][1] : (*this)[n_x * (n_y-1)][1]; }
        float ymax() const { return g_order == GridOrder::bottomleft_to_topright ? (*this)[n_x * (n_y-1)][1] : (*this)[0][1]; }

        //! Extents {xmin, xmax, ymin, ymax}
        morph::vec<float, 4> extents() const { return morph::vec<float, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        morph::vec<float, 2> centre() const { return morph::vec<float, 2>({ xmax() - xmin(), ymax() - ymin() }) * 0.5f; }

        //! Return the row for the index
        size_t row (const size_t index) const { return index < n ? index % n_x : std::numeric_limits<size_t>::max(); }

        //! Return the col for the index
        size_t col (const size_t index) const { return index < n ? index / n_x : std::numeric_limits<size_t>::max(); }

        //! Two vector structures that contains the coords for this grid.
        morph::vvec<float> d_x;
        morph::vvec<float> d_y;
    };

} // namespace morph
