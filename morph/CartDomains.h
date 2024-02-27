/*!
 * \file CartDomains.h
 *
 * This contains some definitions that are used by CartGrid.h and Grid.h, both of which define
 * cartesian grids.
 *
 * \author Seb James
 * \date Feb 2024
 */
#pragma once

namespace morph {

    //! The shape of the cartesian grid. Only used by CartGrid.h as Grid.h only specifies rectangular grids.
    enum class CartDomainShape {
        Rectangle,
        Boundary // The shape of the arbitrary boundary set with CartGrid::setBoundary
    };

    //! The wrapping employed for the Cartesian grid.
    enum class CartDomainWrap {
        None,        // No wrapping
        Horizontal,  // The eastern neighbour of the most eastern element is the most western element on that row
        Vertical,
        Both
    };

} // namespace morph
