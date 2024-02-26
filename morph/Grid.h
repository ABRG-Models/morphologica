/*!
 * \file
 *
 * This is a simple Cartesian Grid class. Simpler than morph::CartGrid. There is no option to
 * define an arbitrary boundary to your domain.
 *
 * \author Seb James
 * \date February 2024
 */

#pragma once

#include <string>
#include <sstream>
#include <morph/vec.h>

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

    // A nice simple grid class. This provides coordinates for each element in a grid along with
    // neighbour relationships. The data associated with the grid is expected to be stored in an
    // array or vector. Use the index of the array/vector to obtain info about the grid
    // locations. The size of the grid is not expected to change and so is provided as template args
    template <size_t n_x = 1, size_t n_y = 1,
              GridOrder O = morph::GridOrder::bottomleft_to_topright>
    struct Grid
    {
        // The number of elements in the grid
        static constexpr size_t n = n_x * n_y;

        // What is the width of a grid? Is it the distance from the centre of the left-most pixel to
        // the centre of the right-most pixel, OR is it the distance from the left edge of the
        // left-most pixel to the right edge of the right-most pixel?
        // BUT this is the number of pixels across

        //! The horizontal distance between adjacent grid element centres
        float d = 1.0f;

        //! The vertical distance between adjacent grid element centres
        float v = 1.0f;

        morph::vec<float, 2> zero_location = {0.0f, 0.0f};

        // Indexing the grid will return a computed vec location
        morph::vec<float, 2> operator[] (size_t index) const
        {
            morph::vec<float, 2> loc = zero_location;
            loc[0] += d * (index % n_x);
            if constexpr (O == morph::GridOrder::bottomleft_to_topright) {
                loc[1] += v * (index / n_x);
            } else {
                loc[1] -= v * (index / n_x);
            }
            // compute location for the index
            return loc;
        }

        /*!
         * Output some text information about the hexgrid.
         */
        std::string output() const
        {
            std::stringstream ss;
            ss << n_x << " x " << n_y << " grid (" << n << " elements).";

        }

    };

} // namespace
