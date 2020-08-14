/*
 * Author: Seb James
 *
 * Date: 2018/07
 */
#pragma once

#include "morph/Hex.h"
#include "morph/BezCurvePath.h"
#include "morph/MathConst.h"

#include <set>
#include <list>
#include <string>
#include <array>
#include <stdexcept>
#include <deque>

namespace morph {

    /*!
     * Enumerates the way that the guidance molecules are set up
     */
    enum class HexDomainShape {
        Rectangle,
        Parallelogram,
        Hexagon,
        Boundary // The shape of the arbitrary boundary set with HexGrid::setBoundary
    };

    /*!
     * This class is used to build an hexagonal grid of hexagons. The member hexagons
     * are all arranged with a vertex pointing vertically - "point up". The extent of
     * the grid is determined by the x_span set during construction; the number of
     * hexes in the grid by d and x_span.
     *
     * Optionally, a boundary may be set by calling setBoundary (const
     * BezCurvePath&). If this is done, then the boundary is converted to a set of
     * hexes, then those hexes in the hexagonal grid lying outside the boundary are
     * removed.
     *
     * Another option for boundary setting is to pass in a list of Hexes whose
     * positions will be used to mark out the boundary.
     *
     * This class manages the integer iterators stored in each Hex (Hex::vi), which
     * may be used to index into external data structures (arrays or vectors) which
     * contain information about the 2D surface represented by the HexGrid which is to
     * be computed.
     */
    class alignas(8) HexGrid
    {
    public:
        /*!
         * Domain attributes
         * -----------------
         *
         * Vectors containing the "domain" info extracted from the list of Hexes. The
         * "domain" is the set of Hexes left over after the boundary has been applied
         * and the outer Hexes have been reduced down to a regular, somewhat
         * rectangular set.
         *
         * Each of these is prefixed d_ and is carefully aligned.
         *
         * The order in which these are populated is raster-style, from top left to
         * bottom right.
         */
        //@{
        alignas(alignof(std::vector<float>)) std::vector<float> d_x;
        alignas(alignof(std::vector<float>)) std::vector<float> d_y;
        alignas(8) std::vector<int> d_ri;
        alignas(8) std::vector<int> d_gi;
        alignas(8) std::vector<int> d_bi;

        /*!
         * Neighbour iterators. For use when the stride to the neighbour ne or nw is
         * not constant. i.e. for use when the domain of computation is not a
         * parallelogram. Note that d_ne and d_nw ARE required, because even though
         * the neighbour east or west is always +/- 1 in memory address space in the
         * parallelogram and rectangular domain cases, if the domain is hexagonal or
         * arbitrary boundary, then even this is not true.
         */
        //@{
        alignas(8) std::vector<int> d_ne;
        alignas(8) std::vector<int> d_nne;
        alignas(8) std::vector<int> d_nnw;
        alignas(8) std::vector<int> d_nw;
        alignas(8) std::vector<int> d_nsw;
        alignas(8) std::vector<int> d_nse;
        //@}

        /*!
         * Flags, such as "on boundary", "inside boundary", "outside boundary", "has
         * neighbour east", etc.
         */
        alignas(8) std::vector<unsigned int> d_flags;

        /*!
         * Distance to boundary for any hex.
         */
        alignas(8) std::vector<float> d_distToBoundary;

        /*!
         * The length of a row in the domain. The first Hex in the first row will
         * overhang to the left.
         */
        unsigned int d_rowlen = 0;

        /*!
         * The number of rows in the domain.
         */
        unsigned int d_numrows = 0;

        /*!
         * d_rowlen * d_numrows is the domain size in number of hexes. Client code
         * will create vectors of length d_size and hold the variables pertaining to
         * the Hex domain therein.
         */
        unsigned int d_size = 0;

        /*!
         * How many additional hexes to grow out to the left and right; top and
         * bottom? Set this to a larger number if the boundary is expected to grow
         * during a simulation.
         */
        //@{
        unsigned int d_growthbuffer_horz = 5;
        unsigned int d_growthbuffer_vert = 0;
        //@}

        /*!
         * Add entries to all the d_ vectors for the Hex pointed to by hi.
         */
        void d_push_back (std::list<Hex>::iterator hi);

        /*!
         * Once Hex::di attributes have been set, populate d_nne and friends.
         */
        void populate_d_neighbours (void);

        /*!
         * Clear out all the d_ vectors
         */
        void d_clear (void);
        //@}

        /*!
         * Save this HexGrid (and all the Hexes in it) into the HDF5 file at the
         * location @path.
         */
        void save (const std::string& path);

        /*!
         * Populate this HexGrid from the HDF5 file at the location @path.
         */
        void load (const std::string& path);

        /*!
         * Default constructor
         */
        HexGrid();

        /*!
         * Construct then load from file.
         */
        HexGrid(const std::string& path);

        /*!
         * Construct the hexagonal hex grid with a hex to hex distance of @a d_
         * (centre to centre) and approximate diameter of @a x_span_. Set z to @a z_
         * which may be useful as an identifier if several HexGrids are being managed
         * by client code, but it not otherwise made use of.
         */
        HexGrid (float d_, float x_span_, float z_ = 0.0f,
                 HexDomainShape shape = HexDomainShape::Parallelogram);

        /*!
         * Initialise with the passed-in parameters; a hex to hex distance of @a d_
         * (centre to centre) and approximate diameter of @a x_span_. Set z to @a z_
         * which may be useful as an identifier if several HexGrids are being managed
         * by client code, but it not otherwise made use of.
         */
        void init (float d_, float x_span_, float z_ = 0.0f);

        /*!
         * Compute the centroid of the passed in list of Hexes.
         */
        std::pair<float, float> computeCentroid (const std::list<Hex>& pHexes);

        /*!
         * Sets boundary to match the list of hexes passed in as @a pHexes. Note, that
         * unlike void setBoundary (const BezCurvePath& p), this method does not apply
         * any offset to the positions of the hexes in @a pHexes.
         */
        void setBoundary (const std::list<Hex>& pHexes);

        /*!
         * Sets boundary to \a p, then runs the code to discard hexes lying outside
         * this boundary. Finishes up by calling morph::HexGrid::discardOutside.
         *
         * The BezCurvePath's centroid may not be 0,0. If loffset has its default value
         * of true, then this method offsets the boundary so that when it is applied to
         * the HexGrid, the centroid IS (0,0). If \a loffset is false, then \a p is not
         * translated in this way.
         */
        void setBoundary (const BezCurvePath<float>& p, bool loffset = true);

        /*!
         * Sets the boundary of the hexgrid to \a bpoints, then runs the code to discard
         * hexes lying outside this boundary. Finishes up by calling
         * HexGrid::discardOutside. By default, this method translates \a bpoints so
         * that when the boundary is applied to the HexGrid, its centroid is (0,0). If
         * the default value of \a loffset is changed to false, \a bpoints is NOT
         * translated.
         */
        void setBoundary (std::vector<BezCoord<float>>& bpoints, bool loffset = true);

        /*!
         * Set all the outer hexes as being "boundary" hexes. This makes it possible
         * to create the default hexagon of hexes, then mark the outer hexes as being
         * the boundary.
         *
         * Works only on the initial hexagonal layout of hexes.
         */
        void setBoundaryOnOuterEdge (void);

        /*!
         * Get all the boundary hexes in a list. This assumes that a boundary has
         * already been set with one of the setBoundary() methods and so there is
         * therefore a set of Hexes which are already marked as being on the boundary
         * (with the attribute Hex::boundaryHex == true) Do this by going around the
         * boundary neighbour to neighbour?
         *
         * Now a getter for this->bhexen.
         */
        std::list<Hex> getBoundary (void) const;

        //! Compute an elliptical boundary using the elliptical radii \a a and \a b.
        std::vector<BezCoord<float>> ellipseCompute (const float a, const float b);

        //! Compute the length of the perimeter of an ellipse with radii \a a and \a b.
        float ellipsePerimeter (const float a, const float b);

        /*!
         * Set the boundary to be an ellipse with the given radii parameters a and b.
         */
        void setEllipticalBoundary (const float a, const float b);

        /*!
         * Set the boundary to be a circle with the given radius a.
         */
        void setCircularBoundary (const float a);

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
        std::string output (void) const;

        /*!
         * Show the coordinates of the vertices of the overall hex grid generated.
         */
        std::string extent (void) const;

        /*!
         * Returns the width of the HexGrid (from -x to +x)
         */
        float width (void);

        /*!
         * Returns the 'depth' of the HexGrid (from -y to +y)
         */
        float depth (void);

        /*!
         * Getter for d.
         */
        float getd (void) const;

        /*!
         * Getter for v - vertical hex spacing.
         */
        float getv (void) const;

        /*!
         * Get the shortest distance from the centre to the perimeter. This is the
         * "short radius".
         */
        float getSR (void) const;

        /*!
         * The distance from the centre of the Hex to any of the vertices. This is the
         * "long radius".
         */
        float getLR (void) const;

        /*!
         * The vertical distance from the centre of the hex to the "north east" vertex
         * of the hex.
         */
        float getVtoNE (void) const;

        /*!
         * Compute and return the area of one hex in the grid.
         */
        float getHexArea (void) const;

        /*!
         * Find the minimum or maximum value of x' on the HexGrid, where x' is the x
         * axis rotated by phi degrees.
         */
        //@{
        float getXmin (float phi = 0.0f) const;
        float getXmax (float phi = 0.0f) const;
        //@}

        /*!
         * Run through all the hexes and compute the distance to the nearest boundary
         * hex.
         */
        void computeDistanceToBoundary (void);

        /*!
         * Populate d_ vectors. simple version. (Finds extents, then calls
         * populate_d_vectors(const array<int, 6>&)
         */
        void populate_d_vectors (void);

        /*!
         * Populate d_ vectors, paying attention to domainShape.
         */
        void populate_d_vectors (const std::array<int, 6>& extnts);

        /*!
         * Get a vector of Hex pointers for all hexes that are inside/on the path
         * defined by the BezCurvePath @p, thus this gets a 'region of hexes'. The Hex
         * flags "region" and "regionBoundary" are used, temporarily to mark out the
         * region. The idea is that client code will then use the vector of Hex* to
         * work with the region however it needs to.
         *
         * The centroid of the region is placed in @regionCentroid
         * (i.e. @regionCentroid is a return argument)
         *
         * It's assumed that the BezCurvePath defines a closed region.
         *
         * If applyOriginalBoundaryCentroid is true, then the region is translated by
         * the same amount that the overall boundary was translated to ensure that the
         * boundary's centroid is at 0,0.
         *
         * Returns a vector of iterators to the Hexes that make up the region.
         */
        //@{
        std::vector<std::list<Hex>::iterator> getRegion (BezCurvePath<float>& p, std::pair<float, float>& regionCentroid,
                                                         bool applyOriginalBoundaryCentroid = true);
        std::vector<std::list<Hex>::iterator> getRegion (std::vector<BezCoord<float>>& bpoints, std::pair<float, float>& regionCentroid,
                                                         bool applyOriginalBoundaryCentroid = true);
        //@}

        /*!
         * For every hex in hexen, unset the flags HEX_IS_REGION_BOUNDARY and
         * HEX_INSIDE_REGION
         */
        void clearRegionBoundaryFlags (void);

        /*!
         * Using this HexGrid as the domain, convolve the domain data \a data with the
         * kernel data \a kerneldata, which exists on another HexGrid, \a
         * kernelgrid. Return the result in \a result.
         */
        template<typename T>
        void convolve (const HexGrid& kernelgrid, const std::vector<T>& kerneldata, const std::vector<T>& data, std::vector<T>& result)
        {
            if (result.size() != this->hexen.size()) {
                throw std::runtime_error ("The result vector is not the same size as the HexGrid.");
            }
            if (result.size() != data.size()) {
                throw std::runtime_error ("The data vector is not the same size as the HexGrid.");
            }
            if (kernelgrid.getd() != this->d) {
                throw std::runtime_error ("The kernel HexGrid must have same d as this HexGrid to carry out convolution.");
            }
            if (&data == &result) {
                throw std::runtime_error ("Pass in separate memory for the result.");
            }

            // For each hex in this HexGrid, compute the convolution kernel
            std::list<Hex>::iterator hi = this->hexen.begin();
            for (; hi != this->hexen.end(); ++hi) {
                T sum = T{0};
                // For each kernel hex, sum up.
                for (auto kh : kernelgrid.hexen) {
                    std::list<Hex>::iterator dhi = hi;
                    // Kernel hex coords r,g are: kh.ri, kh.gi, which may be (are EXPECTED to be) +ve or -ve
                    //
                    // Origin hex coords are h.ri, h.gi
                    //
                    // To get the hex whose data we want to multiply with kh's value,
                    // can go via neighbour relations, but must be prepared to take a
                    // variable path because going directly in r direction then directly
                    // in g direction could take us temporarily outside the boundary of
                    // the HexGrid.
                    int rr = kh.ri;
                    int gg = kh.gi;
                    bool failed = false;
                    bool finished = false;
                    //while (gg != 0 && rr != 0) {
                    while (!finished) {
                        bool moved = false;
                        // Try to move in r direction
                        if (rr > 0) {
                            if (dhi->has_ne()) {
                                dhi = dhi->ne;
                                --rr;
                                moved = true;
                            } // Didn't move in +r direction
                        } else if (rr < 0) {
                            if (dhi->has_nw()) {
                                dhi = dhi->nw;
                                ++rr;
                                moved = true;
                            } // Didn't move in -r direction
                        }
                        // Try to move in g direction
                        if (gg > 0) {
                            if (dhi->has_nne()) {
                                dhi = dhi->nne;
                                --gg;
                                moved = true;
                            } // Didn't move in +g direction
                        } else if (gg < 0) {
                            if (dhi->has_nsw()) {
                                dhi = dhi->nsw;
                                ++gg;
                                moved = true;
                            } // Didn't move in -g direction
                        }

                        if (rr == 0 && gg == 0) {
                            finished = true;
                            break;
                        }

                        if (!moved) {
                            // We're stuck; Can't move in r or g direction, so can't add a contribution
                            failed = true;
                            break;
                        }
                    }

                    if (!failed) {
                        // Can do the sum
                        sum +=  data[dhi->vi] * kerneldata[kh.vi];
                    }
                }

                result[hi->vi] = sum;
            }
        }

        /*!
         * What shape domain to set? Set this to the non-default BEFORE calling
         * HexGrid::setBoundary (const BezCurvePath& p) - that's where the domainShape
         * is applied.
         */
        HexDomainShape domainShape = HexDomainShape::Parallelogram;

        /*!
         * The list of hexes that make up this HexGrid.
         */
        std::list<Hex> hexen;

        /*!
         * Once boundary secured, fill this vector. Experimental - can I do parallel
         * loops with vectors of hexes? Ans: Not very well.
         */
        std::vector<Hex*> vhexen;

        /*!
         * While determining if boundary is continuous, fill this maps container of
         * hexes.
         */
        std::list<Hex*> bhexen; // Not better as a separate list<Hex>?

        /*!
         * Store the centroid of the boundary path. The centroid of a read-in
         * BezCurvePath [see void setBoundary (const BezCurvePath& p)] is subtracted
         * from each generated point on the boundary path so that the boundary once it
         * is expressed in the HexGrid will have a (2D) centroid of roughly
         * (0,0). Hence, this is usually roughly (0,0).
         */
        std::pair<float, float> boundaryCentroid;

        /*!
         * Holds the centroid of the boundary before all points on the boundary were
         * translated so that the centroid of the boundary would be 0,0
         */
        std::pair<float, float> originalBoundaryCentroid;

    private:
        /*!
         * Initialise a grid of hexes in a hex spiral, setting neighbours as the grid
         * spirals out. This method populates hexen based on the grid parameters set
         * in d and x_span.
         */
        void init (void);

        /*!
         * Starting from @a startFrom, and following nearest-neighbour relations, find
         * the closest Hex in hexen to the coordinate point @a point, and set its
         * Hex::onBoundary attribute to true.
         *
         * return An iterator into hexen which refers to the closest Hex to @a point.
         */
        std::list<Hex>::iterator setBoundary (const BezCoord<float>& point, std::list<Hex>::iterator startFrom);

        /*!
         * Determine whether the boundary is contiguous. Whilst doing so, populate a
         * list<Hex> containing just the boundary Hexes.
         */
        bool boundaryContiguous (void);

        /*!
         * Determine whether the boundary is contiguous, starting from the boundary
         * Hex iterator #bhi.
         *
         * The overload with bhexes takes a list of Hex pointers and populates it with
         * pointers to the hexes on the boundary.
         */
        //@{
        bool boundaryContiguous (std::list<Hex>::const_iterator bhi, std::list<Hex>::const_iterator hi, std::set<unsigned int>& seen);
        //@}

        /*!
         * Set the hex closest to point as being on the region boundary. Region
         * boundaries are supposed to be temporary, so that client code can find a
         * region, extract the pointers to all the Hexes in that region and store that
         * information for later use.
         */
        std::list<Hex>::iterator setRegionBoundary (const BezCoord<float>& point, std::list<Hex>::iterator startFrom);

        /*!
         * Determine whether the region boundary is contiguous, starting from the
         * boundary Hex iterator #bhi.
         */
        bool regionBoundaryContiguous (std::list<Hex>::const_iterator bhi,
                                       std::list<Hex>::const_iterator hi, std::set<unsigned int>& seen);

        /*!
         * Find a hex, any hex, that's on the boundary specified by #boundary. This
         * assumes that setBoundary (const BezCurvePath&) has been called to mark the
         * Hexes that lie on the boundary.
         */
        bool findBoundaryHex (std::list<Hex>::const_iterator& hi) const;

        /*!
         * Find the hex near @point, starting from startFrom, which should be as close
         * as possible to point in order to reduce computation time.
         */
        std::list<Hex>::iterator findHexNearPoint (const BezCoord<float>& point, std::list<Hex>::iterator startFrom);

        /*!
         * Mark hexes as being inside the boundary given that @hi refers to a boundary
         * Hex and at least one adjacent hex to @hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the
         * boundary hex is the inside)
         *
         * By changing bdryFlag and insideFlag, it's possible to use this method with
         * region boundaries.
         */
        //@{
        void markFromBoundary (std::list<Hex*>::iterator hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY);
        void markFromBoundary (std::list<Hex>::iterator hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY);
        void markFromBoundary (Hex* hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY);
        //@}

        /*!
         * Common code used by markFromBoundary()
         */
        void markFromBoundaryCommon (std::list<Hex>::iterator first_inside, unsigned short firsti,
                                     unsigned int bdryFlag = HEX_IS_BOUNDARY,
                                     unsigned int insideFlag = HEX_INSIDE_BOUNDARY);

        /*!
         * Given the current boundary hex iterator, bhi and the n_recents last boundary
         * hexes in recently_seen, and assuming that bhi has had all its adjacent inside
         * hexes marked as insideBoundary, find the next boundary hex.
         *
         * \param bhi The boundary hex iterator. From this hex, find the next boundary
         * hex.
         *
         * \param recently_seen a deque containing the recently processed boundary
         * hexes. for a boundary which is always exactly one hex thick, you only need a
         * memory of the last boundary hex to keep you going in the right direction
         * around the boundary BUT if your boundary has some "double thickness"
         * sections, then you need to know a few more recent hexes to avoid looping
         * around and returning to the start!
         *
         * \param n_recents The number of hexes to record in \a recently_seen. The
         * actual number you will need depends on the "thickness" of your boundary -
         * does it have sections that are two hexes thick, or sections that are six
         * hexes thick? It also depends on the length along which the boundary may be
         * two hexes thick. In theory, if you have a boundary section two hexes thick
         * for 5 pairs, then you need to store 10 previous hexes. However, due to the
         * way that this algorithm tests hexes (always testing direction '0' which is
         * East first, then going anti-clockwise to the next direction; North-East and
         * so on), n_recents=2 appears to be sufficient for a thickness 2 boundary,
         * which is what can occur when setting a boundary using the method
         * HexGrid::setEllipticalBoundary. Boundaries that are more than thickness 2
         * shouldn't really occur, whereas a boundary with a short section of thickness
         * 2 can quite easily occur, as in setEllipticalBoundary, where insisting that
         * the boundary was strictly always only 1 hex thick would make that algorithm
         * more complex.
         *
         * \param bdryFlag The flag used to recognise a boundary hex.
         *
         * \param insideFlag The flag used to recognise a hex that is inside the
         * boundary.
         *
         * \return true if a next boundary neighbour was found, false otherwise.
         */
        bool findNextBoundaryNeighbour (std::list<Hex>::iterator& bhi,
                                        std::deque<std::list<Hex>::iterator>& recently_seen,
                                        size_t n_recents = 2,
                                        unsigned int bdryFlag = HEX_IS_BOUNDARY,
                                        unsigned int insideFlag = HEX_INSIDE_BOUNDARY) const;

        /*!
         * Mark hexes as insideBoundary if they are inside the boundary. Starts from
         * @hi which is assumed to already be known to refer to a hex lying inside the
         * boundary.
         */
        void markHexesInside (std::list<Hex>::iterator hi,
                              unsigned int bdryFlag = HEX_IS_BOUNDARY,
                              unsigned int insideFlag = HEX_INSIDE_BOUNDARY);

        /*!
         * Recursively mark hexes to be kept if they are inside the rectangular hex
         * domain.
         */
        void markHexesInsideRectangularDomain (const std::array<int, 6>& extnts);

        /*!
         * Mark hexes to be kept if they are in a parallelogram domain.
         */
        void markHexesInsideParallelogramDomain (const std::array<int, 6>& extnts);

        /*!
         * Mark ALL hexes as inside the domain
         */
        void markAllHexesInsideDomain (void);

        /*!
         * Discard hexes in this->hexen that are outside the boundary #boundary.
         */
        void discardOutsideBoundary (void);

        /*!
         * Discard hexes in this->hexen that are outside the rectangular hex domain.
         */
        void discardOutsideDomain (void);

        /*!
         * Find the extents of the boundary hexes. Find the ri for the left-most hex
         * and the ri for the right-most hex (elements 0 and 1 of the return
         * array). Find the gi for the top most hex and the gi for the bottom most
         * hex. Assumes bi is 0.
         *
         * Return object contains: {ri-left, ri-right, gi-bottom, gi-top, gi at
         * ri-left, gi at ri-right}
         *
         * gi at ri-left, gi at ri-right are returned so that the bottom left hex can
         * be set correctly and the entire boundary is enclosed - it's important to
         * know if the bottom line is parity-matched with the line on which the left
         * and right most boundary hexes are found.
         */
        std::array<int, 6> findBoundaryExtents (void);

        /*!
         * setDomain() will define a regular domain, then discard those hexes outside
         * the regular domain and populate all the d_ vectors.
         */
        void setDomain (void);

        /*!
         * Find the Hex in the Hex grid which is closest to the x,y position given by
         * pos.
         */
        std::list<Hex>::iterator findHexNearest (const std::pair<float, float>& pos);

        /*!
         * Does what it says on the tin. Re-number the Hex::vi vector index in each
         * Hex in the HexGrid, from the start of the list<Hex> hexen until the end.
         */
        void renumberVectorIndices (void);

        /*!
         * The centre to centre hex distance between adjacent members of the hex grid.
         */
        float d = 1.0f;

        /*!
         * The centre to centre hex distance between hexes on adjacent rows - the
         * 'vertical' distance.
         */
        float v = 1.0f * SQRT_OF_3_OVER_2_F;

        /*!
         * Give the hexagonal hex grid a diameter of approximately x_span in the
         * horizontal direction, which is perpendicular to one of the edges of the
         * member hexagons.
         */
        float x_span = 10.0f;

        /*!
         * The z coordinate of this hex grid layer
         */
        float z;

        /*!
         * A boundary to apply to the initial, rectangular grid.
         */
        BezCurvePath<float> boundary;

        /*!
         * Hex references to the hexes on the vertices of the hexagonal
         * grid. Configured during init(). These will become invalid when a new
         * boundary is applied to the original hexagonal grid. When this occurs,
         * gridReduced should be set false.
         */
        //@{
        std::list<Hex>::iterator vertexE;
        std::list<Hex>::iterator vertexNE;
        std::list<Hex>::iterator vertexNW;
        std::list<Hex>::iterator vertexW;
        std::list<Hex>::iterator vertexSW;
        std::list<Hex>::iterator vertexSE;
        //@}

        /*!
         * Set true when a new boundary or domain has been applied. This means that
         * the #vertexE, #vertexW, and similar iterators are no longer valid.
         */
        bool gridReduced = false;

    };

} // namespace morph
