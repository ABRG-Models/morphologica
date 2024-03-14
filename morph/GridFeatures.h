/*!
 * \file GridFeatures.h
 *
 * This contains some definitions that are used by CartGrid.h, Grid.h and Gridct.h, all of which
 * define Cartesian grids.
 *
 * \author Seb James
 * \date Feb 2024
 */
#pragma once

namespace morph {

    //! The shape of the cartesian grid. Only used by CartGrid.h as Grid.h only specifies rectangular grids.
    enum class GridDomainShape {
        Rectangle,
        Boundary // The shape of the arbitrary boundary set with CartGrid::setBoundary
    };

    //! The wrapping employed for the Cartesian grid.
    enum class GridDomainWrap {
        None,        // No wrapping
        Horizontal,  // The eastern neighbour of the most eastern element is the most western element on that row
        Vertical,    // The northern neighbour of the most northern element is the most southern element on that col
        Both
    };

    /*!
     * What's the ordering of a rectangular grid?
     *
     * An example grid of width 4 and height 2 should illustrate:
     *
     * bottomleft_to_topright:
     *
     * 4 5 6 7
     * 0 1 2 3
     *
     * topleft_to_bottomright:
     *
     * 0 1 2 3
     * 4 5 6 7
     *
     * bottomleft_to_topright_colmaj:
     *
     * 1 3 5 7
     * 0 2 4 6
     *
     * topleft_to_bottomright_colmaj:
     *
     * 0 2 4 6
     * 1 3 5 7
     */
    enum class GridOrder
    {
        bottomleft_to_topright,
        topleft_to_bottomright,
        bottomleft_to_topright_colmaj,
        topleft_to_bottomright_colmaj
    };

    /*!
     * How to visualize a grid. You could draw a triangle map with vertices at the centres of the
     * elements or you could draw a rectangular pixel for each element. Triangles is
     * faster. RectInterp gives a nice pixellated rendering.
     */
    enum class GridVisMode
    {
        Triangles,  // Render triangles with a triangle vertex at the centre of each Rect.
        RectInterp  // Render each rect as an actual rectangle made of 4 triangles.
    };

} // namespace morph
