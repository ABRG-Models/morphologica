/*
 * Author: Seb James
 *
 * Date: 2018/07
 */

#ifndef _HEXGRID_H_
#define _HEXGRID_H_

#include "Hex.h"
#include "BezCurvePath.h"

#include <set>
#include <list>
#include <string>
#include <array>

using std::set;
using std::list;
using std::array;
using std::string;
using morph::BezCurvePath;
using morph::Hex;

namespace morph {

    /*!
     * Enumerates the way that the guidance molecules are set up
     */
    enum class HexDomainShape {
        Rectangle,
        Parallelogram,
        Hexagon,
        Boundary, // The shape of the arbitrary boundary set with HexGrid::setBoundary
        SubParallelograms // Arbitrary boundary shape with data in a
                          // set of sub-parallelograms, automatically determined.
    };

    const float SQRT_OF_3_OVER_2_F = 0.866025404;

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
     * Another option for boundary setting is to pass in a list of
     * Hexes whose positions will be used to mark out the boundary.
     *
     * This class manages the integer iterators stored in each Hex
     * (Hex::vi), which may be used to index into external data
     * structures (arrays or vectors) which contain information about
     * the 2D surface represented by the HexGrid which is to be
     * computed.
     */
    class alignas(8) HexGrid
    {
    public:
        /*!
         * Domain attributes
         * -----------------
         *
         * Vectors containing the "domain" info extracted from the
         * list of Hexes. The "domain" is the set of Hexes left over
         * after teh boundary has been applied and the outer Hexes
         * have been reduced down to a regular, somewhat rectangular
         * set.
         *
         * Each of these is prefixed d_ and is carefully aligned.
         *
         * The order in which these are populated is raster-style,
         * from top left to bottom right.
         */
        //@{
        // NB: For these to be alignas(4) need to do some sort of packing.
        alignas(8) vector<float> d_x;
        alignas(8) vector<float> d_y;

        alignas(8) vector<int> d_ri;
        alignas(8) vector<int> d_gi;
        alignas(8) vector<int> d_bi;

        /*!
         * Neighbour iterators. For use when the stride to the
         * neighbour ne or nw is not constant. i.e. for use when the
         * domain of computation is not a parallelogram. Note that
         * d_ne and d_nw ARE required, because even though the
         * neighbour east or west is always +/- 1 in memory address
         * space in the parallelogram and rectangular domain cases, if
         * the domain is hexagonal or arbitrary boundary, then even
         * this is not true.
         */
        //@{
        alignas(8) vector<int> d_ne;
        alignas(8) vector<int> d_nne;
        alignas(8) vector<int> d_nnw;
        alignas(8) vector<int> d_nw;
        alignas(8) vector<int> d_nsw;
        alignas(8) vector<int> d_nse;
        //@}

        /*!
         * An additional address; the index of the vector into which
         * d_ne, d_nne etc point. -1 means d_ne addresses into d_
         * vectors. 0 and up are the index of the sp_ vectors.
         */
        //@{
        alignas(8) vector<int> d_v_ne;
        alignas(8) vector<int> d_v_nne;
        alignas(8) vector<int> d_v_nnw;
        alignas(8) vector<int> d_v_nw;
        alignas(8) vector<int> d_v_nsw;
        alignas(8) vector<int> d_v_nse;
        //@}

        /*!
         * Flags, such as "on boundary", "inside boundary", "outside
         * boundary", "has neighbour east", etc.
         */
        alignas(8) vector<unsigned int> d_flags;

        /*!
         * Distance to boundary for any hex.
         */
        alignas(8) vector<float> d_distToBoundary;

        /*!
         * The length of a row in the domain. The first Hex in the
         * first row will overhang to the left.
         */
        unsigned int d_rowlen = 0;

        /*!
         * The number of rows in the domain.
         */
        unsigned int d_numrows = 0;

        /*!
         * d_rowlen * d_numrows is the domain size in number of
         * hexes. Client code will create vectors of length d_size and
         * hold the variables pertaining to the Hex
         * domain therein.
         */
        unsigned int d_size = 0;

        /*!
         * How many additional hexes to grow out to the left and
         * right; top and bottom? Set this to a larger number if the
         * boundary is expected to grow during a simulation.
         */
        //@{
        unsigned int d_growthbuffer_horz = 5;
        unsigned int d_growthbuffer_vert = 0;
        //@}

        /*!
         * Add entries to all the d_ vectors for the Hex pointed to by
         * hi.
         */
        void d_push_back (list<Hex>::iterator hi);

        /*!
         * Once Hex::di attributes have been set, populate d_nne and
         * friends.
         */
        void populate_d_neighbours (void);

        /*!
         * Clear out all the d_ vectors
         */
        void d_clear (void);
        //@}

        /*!
         * Sub-parallelogram data containers
         *
         * Sub-parallelograms have the following features:
         *
         * a) There is always the same sized stride to the NNW, NNE,
         * NSW and NSE hexes
         *
         * b) All hexes in the sub-parallelogram have a neighbour in
         * every direction, though neighbours may be found in the d_
         * vectors.
         *
         * c) There may be many sub-parallelograms, though I will code
         * for a single, central, big one first.
         */
        //@{
        alignas(8) vector<vector<float> > sp_x;
        alignas(8) vector<vector<float> > sp_y;

        alignas(8) vector<vector<int> > sp_ri;
        alignas(8) vector<vector<int> > sp_gi;
        alignas(8) vector<vector<int> > sp_bi;

        alignas(8) vector<vector<float> > sp_distToBoundary;

        /*!
         * You might not think that the neighbour iterators were
         * required here, because the whole point of the parallelogram
         * shape is that the NW is rowlen-1 from origin, NE is rowlen
         * from origin and so on. *However*, to compute for the hexes
         * around the edge of a sub-parallelogram, we need to access
         * the neighbours in the d_ vectors surrounding the
         * sub-parallelogram. So, where the value sp_d_ne is -1, then
         * the neighbour east is found by 'parallelogram stride
         * rules', If sp_d_ne is some number, such as 239, then the
         * neighour east is in a d_ vector element [239] or another
         * sp_ vector (i.e. a vector of a different parallelogram). In
         * htese cases, the number vector into which to index is found
         * in sp_d_v_ variables.
         */
        //@{
        alignas(8) vector<vector<int> > sp_ne;
        alignas(8) vector<vector<int> > sp_nne;
        alignas(8) vector<vector<int> > sp_nnw;
        alignas(8) vector<vector<int> > sp_nw;
        alignas(8) vector<vector<int> > sp_nsw;
        alignas(8) vector<vector<int> > sp_nse;

        alignas(8) vector<vector<int> > sp_v_ne;
        alignas(8) vector<vector<int> > sp_v_nne;
        alignas(8) vector<vector<int> > sp_v_nnw;
        alignas(8) vector<vector<int> > sp_v_nw;
        alignas(8) vector<vector<int> > sp_v_nsw;
        alignas(8) vector<vector<int> > sp_v_nse;
        //@}

        /*!
         * Flags, such as "on boundary", "inside boundary", "outside
         * boundary", "has neighbour east", etc.
         */
        alignas(8) vector<vector<unsigned int> > sp_flags;

        /*!
         * The length of a row for each sub-parallelogram vector in sp_x.
         */
        alignas(8) vector<unsigned int> sp_rowlens;

        /*!
         * The number of rows in each sub-parallelogram
         */
        alignas(8) vector<unsigned int> sp_numrows;

        /*!
         * The number of hexes in each sub-parallelogram
         */
        alignas(8) vector<unsigned int> sp_veclen;

        /*!
         * Fill with the number of sub-parallelograms
         */
        alignas(8) unsigned int sp_numvecs = 0;

        /*!
         * Allocate the sub-parallelograms
         */
        void allocateSubPgrams (void);

        /*!
         * Add entries to all the sp_ vectors for the Hex pointed to
         * by hi.
         */
        void sp_push_back (list<Hex>::iterator hi);

        /*!
         * Once Hex::di attributes have been set, populate d_nne and
         * friends and sp_nne and friends.
         */
        void populate_sp_d_neighbours (void);

        //@}

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
        HexGrid (float d_, float x_span_, float z_ = 0.0f,
                 HexDomainShape shape = HexDomainShape::Parallelogram);

        /*!
         * Initialise with the passed-in parameters; a hex to hex
         * distance of @a d_ (centre to centre) and approximate
         * diameter of @a x_span_. Set z to @a z_ which may be useful
         * as an identifier if several HexGrids are being managed by
         * client code, but it not otherwise made use of.
         */
        void init (float d_, float x_span_, float z_ = 0.0f);

        /*!
         * Compute the centroid of the passed in list of Hexes.
         */
        pair<float, float> computeCentroid (const list<Hex>& pHexes);

#ifdef UNTESTED_UNUSED
        /*!
         * Subtract the centroid of the boundary,
         * i.e. this->boundaryCentroid, from the position of each Hex,
         * then set this->boundaryCentroid to (0,0).
         */
        void offsetCentroid (void);
#endif
        /*!
         * Sets boundary to match the list of hexes passed in as @a
         * pHexes. Note, that unlike void setBoundary (const
         * BezCurvePath& p), this method does not apply any offset to
         * the positions of the hexes in @a pHexes.
         */
        void setBoundary (const list<Hex>& pHexes);

        /*!
         * Sets boundary to @a p, then runs the code to discard hexes
         * lying outside this boundary. Finishes up by calling
         * discardOutside.
         *
         * The BezCurvePath's centroid may not be 0,0. This method
         * offsets the boundary so that when it is applied to the
         * HexGrid, the centroid IS (0,0).
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
         * Getter for v - vertical hex spacing.
         */
        float getv (void) const;

        /*!
         * Find the minimum or maximum value of x' on the HexGrid,
         * where x' is the x axis rotated by phi degrees.
         */
        //@{
        float getXmin (float phi = 0.0f) const;
        float getXmax (float phi = 0.0f) const;
        //@}

        /*!
         * Run through all the hexes and compute the distance to the
         * nearest boundary hex.
         */
        void computeDistanceToBoundary (void);

        /*!
         * Populate d_ vectors, paying attention to domainShape.
         */
        void populate_d_vectors (const array<int, 6>& extnts);

        /*!
         * What shape domain to set? Set this to the non-default
         * BEFORE calling HexGrid::setBoundary (const BezCurvePath& p)
         * - that's where the domainShape is applied.
         */
        HexDomainShape domainShape = HexDomainShape::Parallelogram;

        /*!
         * The list of hexes that make up this HexGrid.
         */
        list<Hex> hexen;

        /*!
         * Once boundary secured, fill this vector. Experimental - can
         * I do parallel loops with vectors of hexes? Ans: Not very
         * well.
         */
        vector<Hex*> vhexen;

        /*!
         * Store the centroid of the boundary path. The centroid of a
         * read-in BezCurvePath [see void setBoundary (const
         * BezCurvePath& p)] is subtracted from each generated point
         * on the boundary path so that the boundary once it is
         * expressed in the HexGrid will have a (2D) centroid of
         * roughly (0,0). Hence, this is usually roughly (0,0).
         */
        pair<float, float> boundaryCentroid;

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
        bool boundaryContiguous (list<Hex>::const_iterator bhi, list<Hex>::const_iterator hi, set<unsigned int>& seen) const;

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
         * Recursively mark hexes to be kept if they are inside the
         * rectangular hex domain.
         */
        void markHexesInsideRectangularDomain (const array<int, 6>& extnts);

        /*!
         * Mark hexes to be kept if they are in a parallelogram
         * domain.
         */
        void markHexesInsideParallelogramDomain (const array<int, 6>& extnts);

        /*!
         * Mark ALL hexes as inside the domain
         */
        void markAllHexesInsideDomain (void);

        /*!
         * Discard hexes in this->hexen that are outside the boundary
         * #boundary.
         */
        void discardOutsideBoundary (void);

        /*!
         * Discard hexes in this->hexen that are outside the
         * rectangular hex domain.
         */
        void discardOutsideDomain (void);

        /*!
         * Find the extents of the boundary hexes. Find the ri for the
         * left-most hex and the ri for the right-most hex (elements 0
         * and 1 of the return array). Find the gi for the top most
         * hex and the gi for the bottom most hex. Assumes bi is 0.
         *
         * Return object contains:
         * {ri-left, ri-right, gi-bottom, gi-top, gi at ri-left, gi at ri-right}
         *
         * gi at ri-left, gi at ri-right are returned so that the
         * bottom left hex can be set correctly and the entire
         * boundary is enclosed - it's important to know if the bottom
         * line is parity-matched with the line on which the left and
         * right most boundary hexes are found.
         */
        array<int, 6> findBoundaryExtents (void);

        void setDomain (void);

        /*!
         * Find the Hex in the Hex grid which is closest to the x,y
         * position given by pos.
         */
        list<Hex>::iterator findHexNearest (const pair<float, float>& pos);

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
         * The centre to centre hex distance between hexes on adjacent
         * rows - the 'vertical' distance.
         */
        float v = 1.0f * SQRT_OF_3_OVER_2_F;

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
         * Set true when a new boundary or domain has been
         * applied. This means that the #vertexE, #vertexW, and
         * similar iterators are no longer valid.
         */
        bool gridReduced = false;

    };

} // namespace morph

#endif // _HEXGRID_H_
