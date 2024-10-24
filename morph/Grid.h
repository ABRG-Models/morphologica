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
#include <string>
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
        I w = I{1};
        //! h Number of elements that the grid is high
        I h = I{1};
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
        morph::vec<I, 2> get_dims() const { return morph::vec<I, 2>{this->w, this->h}; }
        morph::vec<C, 2> get_dx() const { return this->dx; }
        morph::vec<C, 2> get_offset() const { return this->offset; }
        GridDomainWrap get_wrap() const { return this->wrap; }
        GridOrder get_order() const { return this->order; }

        //! static method to obtain a candidate width and height for a Grid of N elements If
        //! allow_extra is true, and num_elements has no factors, then make a grid that has
        //! >num_elements. Otherwise, return {max, max} for number type I
        static morph::vec<I, 2> suggest_dims (const I num_elements, const bool allow_extra = false)
        {
            morph::vec<I, 2> w_h = { std::numeric_limits<I>::max(), std::numeric_limits<I>::max() };

            if (num_elements <= I{1}) { return w_h; }

            // Naively find factors (this is plenty quick enough for non-astronomical grid sizes)
            morph::vvec<I> factors;
            for (I i = I{2}; i < num_elements; ++i) { if (num_elements % i == I{0}) { factors.push_back (i); } }

            if (!factors.empty()) {
                morph::vvec<C> factors_minus_sqrt = factors.template as<C>() - std::sqrt(static_cast<C>(num_elements));
                size_t j = factors_minus_sqrt.abs().argmin();
                if (j < factors.size()) {
                    I f_other = num_elements / factors[j];
                    w_h[1] = std::min (factors[j], f_other);
                    w_h[0] = num_elements / w_h[1];
                } // else no argmin (return { max, max } to indicate failure)

            } else {
                // There are no factors other than 1
                constexpr I one_by_most = I{20};
                if (num_elements <= one_by_most) {
                    // Allow 1 x num_elements grids if they're small
                    w_h[0] = num_elements;
                    w_h[1] = I{1};

                } else if (allow_extra == true) {
                    // find w, h that are close enough. Add to num_elements and
                    // re-call this function until we find something that works.
                    const I possible_additional = std::numeric_limits<I>::max() - num_elements;
                    for (I j = num_elements + I{1}; j < num_elements + possible_additional; ++j) {
                        w_h = morph::Grid<I, C>::suggest_dims (j, false);
                        if (w_h != morph::vec<I, 2>{ std::numeric_limits<I>::max(), std::numeric_limits<I>::max() }) {
                            // success!
                            break;
                        }
                    }
                }
            }

            return w_h;
        }

        //! Return whether ordering is row-major (true) or column-major (false)
        bool rowmaj() const
        {
            return (this->order == morph::GridOrder::bottomleft_to_topright
                    || this->order == morph::GridOrder::topleft_to_bottomright) ? true : false;
        }

        //! Output the grid as a string, showing the indices and coordinates. Useful for debugging.
        std::string str() const
        {
            std::stringstream ss;
            if (order == morph::GridOrder::bottomleft_to_topright) {
                ss << "bottom left to top right grid order:\n";
                for (I _r = this->h; _r > 0; _r--) {
                    I r = _r-1;
                    for (I c = I{0}; c < this->w; ++c) {
                        I i = r * this->w + c;
                        ss << i << this->coord(i) << "\t";
                    }
                    ss << "\n";
                }
            } else if (order == morph::GridOrder::topleft_to_bottomright) {
                ss << "top left to bottom right grid order:\n";
                for (I r = 0; r < this->h; r++) {
                    for (I c = I{0}; c < this->w; ++c) {
                        I i = r * this->w + c;
                        ss << i << this->coord(i) << "\t";
                    }
                    ss << "\n";
                }

            } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {
                ss << "bottom left to top right (column major) grid order:\n";
                for (I _r = this->h; _r > 0; _r--) {
                    I r = _r-1;
                    for (I c = I{0}; c < this->w; ++c) {
                        I i = r + c * h;
                        ss << i << this->coord(i) << "\t";
                    }
                    ss << "\n";
                }

            } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                ss << "top left to bottom right (column major) grid order:\n";
                for (I r = 0; r < this->h; r++) {
                    for (I c = I{0}; c < this->w; ++c) {
                        I i = r + c * h;
                        ss << i << this->coord(i) << "\t";
                    }
                    ss << "\n";
                }
            }

            return ss.str();
        }

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

        //! Set up memory and populate v_c. Called if parameters w, h, offset
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

            this->n = this->w * this->h;
            this->v_c.resize (this->n);
            for (I i = 0; i < this->n; ++i) { this->v_c[i] = this->coord (i); }
        }

        //! Indexing the grid will return a memorized vec location.
        morph::vec<C, 2> operator[] (const I index) const
        {
            return index >= this->n ? morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()} : this->v_c[index];
        }

        //! A function to find the index of the grid that is closest to the given coordinate.
        //! If the coordinate is off the grid, throw an exception
        I index_lookup (const morph::vec<C, 2>& _coord)
        {
            I index = I{0};
            morph::vec<C, 2> xyf = ((_coord - this->offset) / this->dx);
            xyf[0] = std::round (xyf[0]); // theres no vec::round() function at the time of writing
            xyf[1] = std::round (xyf[1]);

            if (order == morph::GridOrder::topleft_to_bottomright
                || order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                // In case I is not signed, we have to check that xyf[1] is <= 0
                if (xyf[1] > C{0}) {
                    throw std::runtime_error ("Grid y coordinate should be negative with increasing index");
                }
                // Negate xyf[1] before converting to index
                xyf[1] = -xyf[1];
            }
            morph::vec<I, 2> xyi = xyf.template as<I>();

            if (order == morph::GridOrder::bottomleft_to_topright
                || order == morph::GridOrder::topleft_to_bottomright) {
                index = this->w * xyi[1] + xyi[0];
            } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj
                       || order == morph::GridOrder::topleft_to_bottomright_colmaj) {
                index = this->h * xyi[0] + xyi[1];
            }

            if (index >= this->w * this->h || index < I{0}) {
                std::stringstream ee;
                ee << "Grid::index_lookup: Location (" << _coord << ") is off-grid\n";
                throw std::runtime_error (ee.str());
            }

            return index;
        }

        //! A named function that does the same as operator[]
        morph::vec<C, 2> coord_lookup (const I index) const
        {
            return index >= this->n ? morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()} : this->v_c[index];
        }

        //! Compute and return the coordinate with the given index
        morph::vec<C, 2> coord (const I index) const
        {
            if (index >= this->n) { return morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()}; }
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
            I c = this->col (index);
            if (c == (w - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (c == (w - I{1}) && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index - (this->rowmaj() ? (w - I{1}) : (h * (w-I{1})));
            } else {
                return index + (this->rowmaj() ? I{1} : h);
            }
        }
        //! Return the coordinate of the neighbour to the east of index, or if there is no neighbour
        //! to the east, return {C_max, C_max}
        morph::vec<C, 2> coord_ne (const I index) const
        {
            I idx = index_ne (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
        }
        //! Return true if the index has a neighbour to the east
        bool has_ne (const I index) const { return index_ne (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return std::numeric_limits<I>::max()
        I index_nw (const I index) const
        {
            I c = this->col (index);
            if (c == 0 && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Vertical)) {
                return std::numeric_limits<I>::max();
            } else if (c == 0 && (wrap == GridDomainWrap::Horizontal || wrap == GridDomainWrap::Both)) {
                return index + (this->rowmaj() ? (w - I{1}) : (h * (w-I{1})));
            } else {
                return index - (this->rowmaj() ? I{1} : h);
            }
        }
        //! Return the coordinate of the neighbour to the west of index, or if there is no neighbour
        //! to the west, return {C_max, C_max}
        morph::vec<C, 2> coord_nw (const I index) const
        {
            I idx = index_nw (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
        }
        //! Return true if the index has a neighbour to the west
        bool has_nw (const I index) const { return index_nw (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return std::numeric_limits<I>::max()
        I index_nn (const I index) const
        {
            I r = this->row (index);
            if (order == morph::GridOrder::bottomleft_to_topright) {

                if (r == (h - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - I{1}) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index  - (w * (h - I{1}));
                } else {
                    return index + w;
                }

            } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {

                if (r == (h - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - I{1}) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (I{1} - h);
                } else {
                    return index + I{1};
                }

            } else if (order == morph::GridOrder::topleft_to_bottomright) {

                if (r == I{0} && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == I{0} && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - I{1}));
                } else {
                    return index - w;
                }

            } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {

                if (r == I{0} && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == I{0} && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (h - I{1});
                } else {
                    return index - I{1};
                }
            } // else: Unknown order (should not occur)

            return std::numeric_limits<I>::max();
        }
        //! Return the coordinate of the neighbour to the north of index, or if there is no neighbour
        //! to the north, return {C_max, C_max}
        morph::vec<C, 2> coord_nn (const I index) const
        {
            I idx = index_nn (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
        }
        //! Return true if the index has a neighbour to the north
        bool has_nn (const I index) const { return index_nn (index) != std::numeric_limits<I>::max(); }

        //! Return the index of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return std::numeric_limits<I>::max()
        I index_ns (const I index) const
        {
            I r = this->row (index);

            if (order == morph::GridOrder::bottomleft_to_topright) {

                if (r == I{0} && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == 0 && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + (w * (h - I{1}));
                } else {
                    return index - w;
                }

            } else if (order == morph::GridOrder::bottomleft_to_topright_colmaj) {

                if (r == I{0} && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == I{0} && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + h - I{1};
                } else {
                    return index - I{1};
                }

            } else if (order == morph::GridOrder::topleft_to_bottomright) {

                if (r == (h - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - I{1}) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index - (w * (h - I{1}));
                } else {
                    return index + w;
                }

            } else if (order == morph::GridOrder::topleft_to_bottomright_colmaj) {

                if (r == (h - I{1}) && (wrap == GridDomainWrap::None || wrap == GridDomainWrap::Horizontal)) {
                    return std::numeric_limits<I>::max();
                } else if (r == (h - I{1}) && (wrap == GridDomainWrap::Vertical || wrap == GridDomainWrap::Both)) {
                    return index + I{1} - h;
                } else {
                    return index + I{1};
                }
            }

            return std::numeric_limits<I>::max();
        }
        //! Return the coordinate of the neighbour to the south of index, or if there is no neighbour
        //! to the south, return {C_max, C_max}
        morph::vec<C, 2> coord_ns (const I index) const
        {
            I idx = index_ns (index);
            if (idx < n) { return (*this)[idx]; }
            return morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
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
            return idx < n ? (*this)[idx] : morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
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
            return idx < n ? (*this)[idx] : morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
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
            return idx < n ? (*this)[idx] : morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
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
            return idx < n ? (*this)[idx] : morph::vec<C, 2>{std::numeric_limits<C>::max(), std::numeric_limits<C>::max()};
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
        C width() const { return dx[I{0}] * (w - I{1}); }

        //! Return the width of the grid if drawn as pixels
        C width_of_pixels() const { return dx[I{0}] * w; }

        //! Return the distance from the centre of the bottom row to the centre of the top row
        C height() const { return dx[I{1}] * (h - I{1}); }

        C area() const { return this->width() * this->height(); }

        //! Return the height of the grid if drawn as pixels
        C height_of_pixels() const { return dx[I{1}] * h; }

        //! Return the area of the grid, if drawn as pixels
        C area_of_pixels() const { return this->width_of_pixels() * this->height_of_pixels(); }

        //! Individual extents
        C xmin() const { return (*this)[I{0}][I{0}]; }
        C xmax() const {
            if (this->rowmaj() == true) {
                return (*this)[w-I{1}][I{0}];
            } // else colmaj
            return (*this)[(w*h)-I{1}][I{0}];
        }
        C ymin() const {
            if (this->rowmaj() == true) {
                return order == GridOrder::bottomleft_to_topright ? (*this)[I{0}][I{1}] : (*this)[w * (h-I{1})][I{1}];
            } // else colmaj
            return order == GridOrder::bottomleft_to_topright_colmaj ? (*this)[I{0}][I{1}] : (*this)[h-I{1}][I{1}];
        }
        C ymax() const {
            if (this->rowmaj() == true) {
                return order == GridOrder::bottomleft_to_topright ? (*this)[w * (h-I{1})][I{1}] : (*this)[I{0}][I{1}];
            } // else colmaj
            return order == GridOrder::bottomleft_to_topright_colmaj ? (*this)[h-I{1}][I{1}] : (*this)[I{0}][I{1}];
        }

        //! Extents {xmin, xmax, ymin, ymax}
        morph::vec<C, 4> extents() const { return morph::vec<C, 4>{ xmin(), xmax(), ymin(), ymax() }; }

        //! Return the coordinates of the centre of the grid
        morph::vec<C, 2> centre() const { return morph::vec<C, 2>{ xmax() - xmin(), ymax() - ymin() } * C{0.5}; }

        //! Return the y coordinates for each row in the Grid
        morph::vvec<C> get_abscissae() const
        {
            morph::vvec<C> abscissae (w, C{0});
            if (order == GridOrder::bottomleft_to_topright || order == GridOrder::topleft_to_bottomright) {
                // abscissae is just the first width values.
                for (I i = I{0}; i < w; ++i) { abscissae[i] = v_c[i][0]; }
            } else {
                // For column major, we have to skip each row
                for (I i = I{0}; i < w; ++i) { abscissae[i] = v_c[i*h][0]; }
            }
            return abscissae;
        }

        //! Return the x coordinates for each column in the Grid
        morph::vvec<C> get_ordinates() const
        {
            morph::vvec<C> ordinates (h, C{0});
            if (order == GridOrder::bottomleft_to_topright || order == GridOrder::topleft_to_bottomright) {
                for (I i = I{0}; i < h; ++i) { ordinates[i] = v_c[i*w][1]; }
            } else {
                // For column major, ordinates is just the first height values
                for (I i = I{0}; i < h; ++i) { ordinates[i] = v_c[i][1]; }
            }
            return ordinates;
        }

        //! Return the row for the index
        I row (const I index) const {
            if (this->rowmaj() == true) {
                return index < n ? index / w : std::numeric_limits<I>::max();
            } // else colmaj
            return index < n ? index % h : std::numeric_limits<I>::max();
        }

        //! Return the col for the index
        I col (const I index) const {
            if (this->rowmaj() == true) {
                return index < n ? index % w : std::numeric_limits<I>::max();
            } // else colmaj
            return index < n ? index / h : std::numeric_limits<I>::max();
        }

        /*!
         * Resampling function (monochrome).
         *
         * \param image_data (input) The monochrome image as a vvec of floats. The image
         * is interpreted as running from bottom left to top right (matching the default
         * value of Grid::order). Thus, the very first float in the vvec is at x=0,
         * y=0. The image width is normalized to 1.0. The height of the image is
         * computed from this assumption and on the assumption that pixels are square.
         *
         * \param image_pixelwidth (input) The number of pixels that the image is wide
         * \param image_scale (input) The size that the image should be resampled to (same units as Grid)
         * \param image_offset (input) An offset in Grid units to shift the image wrt to the Grid's origin
         * \param sigma (input) The sigma for the 2D resampling Gaussian
         *
         * \return A new data vvec containing the resampled (and renormalised) hex pixel values
         */
        morph::vvec<float> resample_image (const morph::vvec<float>& image_data,
                                           const unsigned int image_pixelwidth,
                                           const morph::vec<float, 2>& image_scale,
                                           const morph::vec<float, 2>& image_offset) const
        {
            if (this->order != morph::GridOrder::bottomleft_to_topright) {
                throw std::runtime_error ("Grid::resample_image: resampling assumes image has morph::GridOrder::bottomleft_to_topright, so your Grid should, too.");
            }

            morph::vvec<float> expr_resampled(this->w * this->h, 0.0f);

            // Before resampling, check if all the values in image_data are identical. In this case,
            // we can short-cut the resampling process.
            float i0 = image_data[0];
            bool all_same = true;
            for (auto id : image_data) {
                if (id != i0) {
                    all_same = false;
                    break;
                }
            }
            if (all_same) {
                // Short-cut - just set all values in the resampled data to the same as in the input data
                expr_resampled.set_from (i0);
                return expr_resampled;
            }

            unsigned int csz = image_data.size();
            morph::vec<unsigned int, 2> image_pixelsz = {image_pixelwidth, csz / image_pixelwidth};

            // Before scaling, image assumed to have width 1, height whatever
            morph::vec<float, 2> image_dims = { 1.0f, 0.0f };
            image_dims[1] = 1.0f / (image_pixelsz[0] - 1u) * (image_pixelsz[1] - 1u);
            // Now scale the image dims to have the same width as *this:
            image_dims *= this->width();
            // Then apply any manual scaling requested:
            image_dims *= image_scale;

            // Distance per pixel in the image. This defines the Gaussian width (sigma) for the
            // resample. Compute this from the image dimensions, assuming pixels are square
            morph::vec<float, 2> dist_per_pix = image_dims / (image_pixelsz - 1u);

            // Parameters for the Gaussian computation
            morph::vec<float, 2> params = 1.0f / (2.0f * dist_per_pix * dist_per_pix);
            morph::vec<float, 2> threesig = 3.0f * dist_per_pix;

#pragma omp parallel for // parallel on this outer loop gives best result (5.8 s vs 7 s)
            for (typename std::vector<float>::size_type xi = 0u; xi < this->v_c.size(); ++xi) {
                float expr = 0.0f;
                for (unsigned int i = 0; i < csz; ++i) {
                    // Get x/y pixel coords:
                    morph::vec<unsigned int, 2> idx = {(i % image_pixelsz[0]), (i / image_pixelsz[0])};
                    // Get the coordinates of the input pixel at index idx (in target units):
                    morph::vec<float, 2> posn = (dist_per_pix * idx) + image_offset;
                    // Distance from input pixel to output pixel:
                    morph::vec<float, 2> _v_c = this->v_c[xi] - posn;
                    // Compute contributions to each Grid pixel, using 2D (elliptical) Gaussian
                    if (_v_c < threesig) { // Testing for distance gives slight speedup
                        expr += std::exp ( - ( (params[0] * _v_c[0] * _v_c[0]) + (params[1] * _v_c[1] * _v_c[1]) ) ) * image_data[i];
                    }
                }
                expr_resampled[xi] = expr;
            }

            expr_resampled /= expr_resampled.max(); // renormalise result
            return expr_resampled;
        }

        //! This vector structure contains the coords for this grid. Note that it is public and so
        //! acccessible by client code
        morph::vvec<morph::vec<C, 2>> v_c;
    };

} // namespace morph
