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
     * \tparam I I for 'Index type'. This is the integer type for indexing the pixels in the grid. The
     * capacity of this type will limit the size of Grid that can be defined. A runtime test will be
     * performed to ensure that the capacity of I is sufficient for the width and height of the
     * requested grid.
     *
     * \tparam C C for 'Coordinate type". The type for storing and retrieving coordinates. In most
     * cases a floating point type will be used, but this could also be a signed integer type. A
     * compiled time test will be performed to ensure it is a signed type.
     *
     */
    template<typename I = unsigned int, typename C = float>
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
        morph::vec<C, 2> dx = { C{1}, C{1} };
        //! offset A vector giving the distance offset (in your chosen units) to Grid
        //! index 0.
        morph::vec<C, 2> offset = { C{0}, C{0} };
        //! wrap An enum to set how the grid wraps. Affects neighbour relationships
        GridDomainWrap wrap = GridDomainWrap::None;
        //! order The index order. Always counting left to right (row-major), but do
        //! you start on the top row or the bottom row (the default)?
        GridOrder order = morph::GridOrder::bottomleft_to_topright;
        //! Record whether ordering is row-major (true) or column-major (false)
        bool rowmaj = true;

    public:
        //! Setter for w
        void set_w (const I _w) { this->w = _w; this->init(); }
        //! Setter for h
        void set_h (const I _h) { this->h = _h; this->init(); }
        //! Setter for dx
        void set_dx (const morph::vec<C, 2> _dx) { this->dx = _dx; this->init(); }
        //! Setter for g_offset
        void set_offset (const morph::vec<C, 2> _offset) { this->offset = _offset; this->init(); }

        //! Setter for most of the grid parameters to be carried out all in one function
        void set_grid_params (const morph::vec<I, 2> dims,
                              const morph::vec<C, 2> spacing,
                              const morph::vec<C, 2> grid_offset)
        {
            this->w = dims[0];
            this->h = dims[I{1}];
            this->dx = spacing;
            this->offset = grid_offset;
            this->init();
        }

        // Note: No setters for wrap or order. I'm assuming noone will want to change these at runtime

        // Getters
        I get_w() const { return this->w; }
        I get_h() const { return this->h; }
        morph::vec<I, 2> get_dims() const { return morph::vec<I, 2>({this->w, this->h}); }
        morph::vec<C, 2> get_dx() const { return this->dx; }
        morph::vec<C, 2> get_offset() const { return this->offset; }
        GridDomainWrap get_wrap() const { return this->wrap; }
        GridOrder get_order() const { return this->order; }

        //! The number of elements in the grid. Public, but don't change it manually.
        I n = w * h;

        //! Constructor
        Grid (const I _w, const I _h,
              const morph::vec<C, 2> _dx = { C{1}, C{1} },
              const morph::vec<C, 2> _offset = { C{0}, C{0} },
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
            static_assert (std::numeric_limits<I>::is_integer, "The index type I must be an integer type");
            static_assert (std::numeric_limits<C>::is_signed,
                           "The coordinate type C must be an signed type (floating point or integer)");

            if constexpr (std::is_signed_v<I> == true) {
                // I is signed, so check that neither w nor h is negative.
                if (this->w < I{0} || this->h < I{0}) {
                    throw std::runtime_error ("Specify your grid with positive width and height");
                }
                // Place the result of w*h in a very large capacity signed integer type and check
                // that it won't overflow I.
                long long int test = this->w * this->h;
                if (test > std::numeric_limits<I>::max()) {
                    throw std::runtime_error ("Use a larger capacity type for I");
                }
            } else {
                // I is unsigned. Place the result of w*h in a very large capacity unsigned integer
                // type and check if it would overflow I.
                unsigned long long int test = this->w * this->h;
                if (test > std::numeric_limits<I>::max()) {
                    throw std::runtime_error ("Use a larger capacity type for I");
                }
            }

            if (this->order == morph::GridOrder::bottomleft_to_topright_colmaj
                || order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                this->rowmaj = false;
            }

            this->n = this->w * this->h;
            this->v_x.resize (this->n);
            this->v_y.resize (this->n);
            morph::vec<C, 2> c = { C{0}, C{0} };
            for (I i = 0; i < this->n; ++i) {
                c = this->coord (i);
                this->v_x[i] = c[I{0}];
                this->v_y[i] = c[I{1}];
            }
        }

        //! Indexing the grid will return a memorized vec location.
        morph::vec<C, 2> operator[] (const I index) const
        {
            return morph::vec<C, 2>({ this->v_x[index], this->v_y[index] });
        }

        //! A named function that does the same as operator[]
        morph::vec<C, 2> coord_lookup (const I index) const
        {
            return morph::vec<C, 2>({ this->v_x[index], this->v_y[index] });
        }

        //! Compute and return the coordinate with the given index
        morph::vec<C, 2> coord (const I index) const
        {
            if (index >= this->n) { return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()}); }
            morph::vec<C, 2> loc = this->offset;
            if (order == morph::GridOrder::bottomleft_to_topright) {
                loc[I{0}] += this->dx[I{0}] * (index % this->w);
                loc[I{1}] += this->dx[I{1}] * (index / this->w);

            } else if (order == morph::GridOrder::topleft_to_bottomright) {
                loc[I{0}] += this->dx[I{0}] * (index % this->w);
                loc[I{1}] -= this->dx[I{1}] * (index / this->w);

            } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                loc[I{0}] += this->dx[I{0}] * (index / this->h);
                loc[I{1}] += this->dx[I{1}] * (index % this->h);

            } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                loc[I{0}] += this->dx[I{0}] * (index / this->h);
                loc[I{1}] -= this->dx[I{1}] * (index % this->h);

            } // else user will just get the offset coordinate

            return loc;
        }

        //! Return the index of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return std::numeric_limits<I>::max()
        I index_ne (const I index) const
        {
            I r = this->row (index);
            if (r == (w - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (r == (w - I{1}) && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index - (this->rowmaj ? (w - I{1}) : (h * (w-I{1})));
            } else {
                return index + (this->rowmaj ? I{1} : h);
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {C_max, C_max}
        morph::vec<C, 2> coord_ne (const I index) const
        {
            I idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
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
                return index + (this->rowmaj ? (w - I{1}) : (h * (w-I{1})));
            } else {
                return index - (this->rowmaj ? I{1} : h);
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {C_max, C_max}
        morph::vec<C, 2> coord_nw (const I index) const
        {
            I idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the west
        bool has_nw (const I index) const { return index_nw (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<I>::max()
        I index_nn (const I index) const
        {
            I c = this->col (index);
            if (c == (h - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                return std::numeric_limits<I>::max();
            } else if (c == (h - I{1}) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                if (order == morph::GridOrder::bottomleft_to_topright) {
                    return index  - (w * (h - I{1}));
                } else if (order == morph::GridOrder::topleft_to_bottomright) {
                    return index + (w * (h - I{1}));
                } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                    return index + (I{1} - h);
                } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                    return index + (h - I{1});
                } else {
                    return std::numeric_limits<I>::max();
                }
            } else {
                if (order == morph::GridOrder::bottomleft_to_topright) {
                    return index + w;
                } else if (order == morph::GridOrder::topleft_to_bottomright) {
                    return index - w;
                } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                    return index + I{1};
                } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                    return index - I{1};
                } else {
                    return std::numeric_limits<I>::max();
                }
            }
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {C_max, C_max}
        morph::vec<C, 2> coord_nn (const I index) const
        {
            I idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }
        //! Return true if the index has a neighbour to the north
        bool has_nn (const I index) const { return index_nn (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<I>::max()
        I index_ns (const I index) const
        {
            I c = this->col (index);
            if (c == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                return std::numeric_limits<I>::max();
            } else if (c == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {

                if (order == morph::GridOrder::bottomleft_to_topright) {
                    return index + (w * (h - I{1}));
                } else if (order == morph::GridOrder::topleft_to_bottomright) {
                    return index - (w * (h - I{1}));
                } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                    return index + h - I{1};
                } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                    return index + I{1} - h;
                } else {
                    return std::numeric_limits<I>::max();
                }
            } else {
                if (order == morph::GridOrder::bottomleft_to_topright) {
                    return index - w;
                } else if (order == morph::GridOrder::topleft_to_bottomright) {
                    return index + w;
                } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                    return index - I{1};
                } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                    return index + I{1};
                } else {
                    return std::numeric_limits<I>::max();
                }
            }
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {C_max, C_max}
        morph::vec<C, 2> coord_ns (const I index) const
        {
            I idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
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
        morph::vec<C, 2> coord_nne (const I index) const
        {
            I idx = this->index_nne (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour north west
        bool has_nnw (const I index) const { return has_nw (index) && has_nn (index); }
        I index_nnw (const I index) const
        {
            I nn = this->index_nn (index);
            return nn < n ? index_nw (nn) : std::numeric_limits<I>::max();
        }
        morph::vec<C, 2> coord_nnw (const I index) const
        {
            I idx = this->index_nnw (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour south east
        bool has_nse (const I index) const { return has_ne (index) && has_ns (index); }
        I index_nse (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_ne (ns) : std::numeric_limits<I>::max();
        }
        morph::vec<C, 2> coord_nse (const I index) const
        {
            I idx = this->index_nse (index);
            return idx < n ? (*this)[idx] : morph::vec<C, 2>({std::numeric_limits<C>::max(), std::numeric_limits<C>::max()});
        }

        //! Neighbour south west
        bool has_nsw (const I index) const { return has_nw (index) && has_ns (index); }
        I index_nsw (const I index) const
        {
            I ns = this->index_ns (index);
            return ns < n ? index_nw (ns) : std::numeric_limits<I>::max();
        }
        morph::vec<C, 2> coord_nsw (const I index) const
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
        C width() const { return dx[I{0}] * w; }

        //! Return the width of the grid if drawn as pixels
        C width_of_pixels() const { return dx[I{0}] * w + dx[I{0}]; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        C height() const { return dx[I{1}] * h; }

        C area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        C height_of_pixels() const { return dx[I{1}] * h + dx[I{1}]; }

        //! Return the area of the grid, if drawn as pixels
        C area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        C xmin() const { return (*this)[I{0}][I{0}]; }
        C xmax() const {
            if (this->rowmaj == true) {
                return (*this)[w-I{1}][I{0}];
            } // else colmaj
            return (*this)[(w*h)-I{1}][I{0}];
        }
        C ymin() const {
            if (this->rowmaj == true) {
                return order == GridOrder::bottomleft_to_topright ? (*this)[I{0}][I{1}] : (*this)[w * (h-I{1})][I{1}];
            } // else colmaj
            return order == GridOrder::bottomleft_to_topright_colmaj ? (*this)[I{0}][I{1}] : (*this)[h-I{1}][I{1}];
        }
        C ymax() const {
            if (this->rowmaj == true) {
                return order == GridOrder::bottomleft_to_topright ? (*this)[w * (h-I{1})][I{1}] : (*this)[I{0}][I{1}];
            } // else colmaj
            return order == GridOrder::bottomleft_to_topright_colmaj ? (*this)[h-I{1}][I{1}] : (*this)[I{0}][I{1}];
        }

        //! Extents {xmin, xmax, ymin, ymax}
        morph::vec<C, 4> extents() const { return morph::vec<C, 4>({ xmin(), xmax(), ymin(), ymax() }); }

        //! Return the coordinates of the centre of the grid
        morph::vec<C, 2> centre() const { return morph::vec<C, 2>({ xmax() - xmin(), ymax() - ymin() }) * C{0.5}; }

        //! Return the row for the index
        I row (const I index) const {
            if (this->rowmaj == true) {
                return index < n ? index % w : std::numeric_limits<I>::max();
            } // else colmaj
            return index < n ? index / w : std::numeric_limits<I>::max();
        }

        //! Return the col for the index
        I col (const I index) const {
            if (this->rowmaj == true) {
                return index < n ? index / w : std::numeric_limits<I>::max();
            } // else colmaj
            return index < n ? index % w : std::numeric_limits<I>::max();
        }

        //! Two vector structures that contains the coords for this grid. v_x is a vector of the x coordinates
        morph::vvec<C> v_x;
        //! v_y is a vector of the y coordinates
        morph::vvec<C> v_y;
    };

} // namespace morph
