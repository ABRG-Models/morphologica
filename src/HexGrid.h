/*
 * Author: Seb James
 *
 * Date: 2018/07
 */

#ifndef _HEXGRID_H_
#define _HEXGRID_H_

#include "Hex.h"
#include <morph/BezCurvePath.h>

#include <list>
#include <string>

using std::list;
using std::string;
using morph::Hex;

namespace morph {

    /*!
     * This class is used to build an hexagonal grid of hexagons. The
     * member hexagons are all arranged with a vertex pointing
     * vertically - "point up". The extent of the grid is determined
     * by the x_span set during construction; the number of hexes in
     * the grid by d and x_span.
     *
     * Optionally, a boundary may be set by calling setBoundary (const
     * BezCurvePath&). If this is done, then the boundary is converted
     * to a set of hexes, then those hexes in the hexagonal grid lying
     * outside the boundary are removed.
     *
     * This class manages the integer iterators stored in each Hex
     * (Hex::vi), which may be used to index into external data
     * structures (arrays or vectors) which contain information about
     * the 2D surface represented by the HexGrid which is to be
     * computed.
     */
    class HexGrid
    {
    public:
        /*!
         * Default constructor
         */
        HexGrid();

        /*!
         * Construct the hexagonal hex grid with a hex to hex distance
         * of @a d_ (centre to centre) and approximate diameter of @a
         * x_span_. Set z to @a z_ which may be useful as an
         * identifier if several HexGrids are being managed by client
         * code, but it not otherwise made use of.
         */
        HexGrid (float d_, float x_span_, float z_ = 0.0f);

        /*!
         * Initialise with the passed-in parameters; a hex to hex
         * distance of @a d_ (centre to centre) and approximate
         * diameter of @a x_span_. Set z to @a z_ which may be useful
         * as an identifier if several HexGrids are being managed by
         * client code, but it not otherwise made use of.
         */
        void init (float d_, float x_span_, float z_ = 0.0f);

        /*!
         * Sets boundry to @a p, then runs the code to discard hexes
         * lying outside this boundary. Finishes up by calling
         * setupNeighbours.
         */
        void setBoundary (const BezCurvePath& p);

        /*!
         * \brief Accessor for the size of hexen.
         *
         * return The number of hexes in the grid.
         */
        unsigned int num (void) const;

        /*!
         * \brief Obtain the vector index of the last Hex in hexen.
         *
         * return Hex::vi from the last Hex in the grid.
         */
        unsigned int lastVectorIndex (void) const;

        /*!
         * Output some text information about the hexgrid.
         */
        string output (void) const;

        /*!
         * Show the coordinates of the vertices of the overall hex
         * grid generated.
         */
        string extent (void) const;

        /*!
         * Getter for d.
         */
        float getd (void) const;

        /*!
         * Find the minimum or maximum value of x' on the HexGrid,
         * where x' is the x axis rotated by phi degrees.
         */
        //@{
        float getXmin (float phi = 0.0f) const;
        float getXmax (float phi = 0.0f) const;
        //@}

        /*!
         * The list of hexes that make up this HexGrid.
         */
        list<Hex> hexen;

    private:
        /*!
         * Initialise a grid of hexes in a hex spiral, setting
         * neighbours as the grid spirals out. This method populates
         * hexen based on the grid parameters set in d and x_span.
         */
        void init (void);

        /*!
         * Starting from @a startFrom, and following nearest-neighbour
         * relations, find the closest Hex in hexen to the coordinate
         * point @a point, and set its Hex::onBoundary attribute to
         * true.
         *
         * return An iterator into hexen which refers to the closest
         * Hex to @a point.
         */
        list<Hex>::iterator setBoundary (const BezCoord& point, list<Hex>::iterator startFrom);

        /*!
         * Determine whether the boundary is contiguous
         */
        bool boundaryContiguous (void) const;

        /*!
         * Determine whether the boundary is contiguous, starting from
         * the boundary Hex iterator #bhi.
         */
        bool boundaryContiguous (list<Hex>::const_iterator bhi) const;

        /*!
         * Find a hex, any hex, that's on the boundary specified by
         * #boundary. This assumes that setBoundary (const
         * BezCurvePath&) has been called to mark the Hexes that lie
         * on the boundary.
         */
        bool findBoundaryHex (list<Hex>::const_iterator& hi) const;

        /*!
         * Recursively mark hexes to be kept if they are inside the
         * boundary.
         */
        void markHexesInside (list<Hex>::iterator hi);

        /*!
         * Discard hexes in this->hexen that are outside the boundary
         * #boundary.
         */
        void discardOutside (void);

        /*!
         * Does what it says on the tin. Re-number the Hex::vi vector
         * index in each Hex in the HexGrid, from the start of the
         * list<Hex> hexen until the end.
         */
        void renumberVectorIndices (void);

        /*!
         * The centre to centre hex distance between adjacent members
         * of the hex grid.
         */
        float d = 1.0f;

        /*!
         * Give the hexagonal hex grid a diameter of approximately
         * x_span in the horizontal direction, which is perpendicular
         * to one of the edges of the member hexagons.
         */
        float x_span = 10.0f;

        /*!
         * The z coordinate of this hex grid layer
         */
        float z;

        /*!
         * A boundary to apply to the initial, rectangular grid.
         */
        BezCurvePath boundary;

        /*!
         * Hex references to the hexes on the vertices of the
         * hexagonal grid. Configured during init(). These will become
         * invalid when a new boundary is applied to the original
         * hexagonal grid. When this occurs, gridReducedToBoundary
         * should be set false.
         */
        //@{
        list<Hex>::iterator vertexE;
        list<Hex>::iterator vertexNE;
        list<Hex>::iterator vertexNW;
        list<Hex>::iterator vertexW;
        list<Hex>::iterator vertexSW;
        list<Hex>::iterator vertexSE;
        //@}

        /*!
         * Set true when a new boundary has been applied. This means
         * that the #vertexE, #vertexW, and similar iterators are no
         * longer valid.
         */
        bool gridReducedToBoundary = false;

    };

} // namespace morph

#endif // _HEXGRID_H_
