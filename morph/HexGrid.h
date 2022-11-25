/*
 * Author: Seb James
 *
 * In this iteration of HexGrid, the idea of the 'domain' has been taken out, as it
 * wasn't useful.
 *
 * Date: 2018/07
 */
#pragma once

#include <morph/Hex.h>
#include <morph/BezCurvePath.h>
#include <morph/BezCoord.h>
#include <morph/MathConst.h>
#include <morph/MathAlgo.h>
#include <morph/HdfData.h>
#include <morph/debug.h>
#include <morph/Matrix22.h>

#include <set>
#include <list>
#include <string>
#include <array>
#include <stdexcept>
#include <deque>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <limits>

namespace morph {

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
         * and the original, outer Hexes have been reduced down to those that will be
         * used in the computation.
         *
         * Each of these is prefixed d_ and is carefully aligned.
         *
         * The order in which these are populated is raster-style, from top left to
         * bottom right.
         */
        alignas(alignof(std::vector<float>)) std::vector<float> d_x;
        alignas(alignof(std::vector<float>)) std::vector<float> d_y;
        alignas(8) std::vector<int> d_ri;
        alignas(8) std::vector<int> d_gi;
        alignas(8) std::vector<int> d_bi;

        /*
         * Neighbour iterators. For use when the stride to the neighbour ne or nw is
         * not constant. i.e. for use when the domain of computation is not a
         * parallelogram. Note that d_ne and d_nw ARE required, because even though
         * the neighbour east or west is always +/- 1 in memory address space in the
         * parallelogram and rectangular domain cases, if the domain is hexagonal or
         * arbitrary boundary, then even this is not true.
         */
        alignas(8) std::vector<int> d_ne;
        alignas(8) std::vector<int> d_nne;
        alignas(8) std::vector<int> d_nnw;
        alignas(8) std::vector<int> d_nw;
        alignas(8) std::vector<int> d_nsw;
        alignas(8) std::vector<int> d_nse;

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
        unsigned int d_growthbuffer_horz = 5;
        unsigned int d_growthbuffer_vert = 0;

        //! Add entries to all the d_ vectors for the Hex pointed to by hi.
        void d_push_back (std::list<Hex>::iterator hi)
        {
            d_x.push_back (hi->x);
            d_y.push_back (hi->y);
            d_ri.push_back (hi->ri);
            d_gi.push_back (hi->gi);
            d_bi.push_back (hi->bi);
            d_flags.push_back (hi->getFlags());
            d_distToBoundary.push_back (hi->distToBoundary);

            // record in the Hex the iterator in the d_ vectors so that d_nne and friends can be set up later.
            hi->di = d_x.size()-1;
        }

        //! Once Hex::di attributes have been set, populate d_nne and friends.
        void populate_d_neighbours()
        {
            // Resize d_nne and friends
            this->d_nne.resize (this->d_x.size(), 0);
            this->d_ne.resize (this->d_x.size(), 0);
            this->d_nnw.resize (this->d_x.size(), 0);
            this->d_nw.resize (this->d_x.size(), 0);
            this->d_nsw.resize (this->d_x.size(), 0);
            this->d_nse.resize (this->d_x.size(), 0);

            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {

                if (hi->has_ne() == true) {
                    this->d_ne[hi->di] = hi->ne->di;
                } else {
                    this->d_ne[hi->di] = -1;
                }

                if (hi->has_nne() == true) {
                    this->d_nne[hi->di] = hi->nne->di;
                } else {
                    this->d_nne[hi->di] = -1;
                }

                if (hi->has_nnw() == true) {
                    this->d_nnw[hi->di] = hi->nnw->di;
                } else {
                    this->d_nnw[hi->di] = -1;
                }

                if (hi->has_nw() == true) {
                    this->d_nw[hi->di] = hi->nw->di;
                } else {
                    this->d_nw[hi->di] = -1;
                }

                if (hi->has_nsw() == true) {
                    this->d_nsw[hi->di] = hi->nsw->di;
                } else {
                    this->d_nsw[hi->di] = -1;
                }

                if (hi->has_nse() == true) {
                    this->d_nse[hi->di] = hi->nse->di;
                } else {
                    this->d_nse[hi->di] = -1;
                }

                ++hi;
            }
        }

        //! Clear out all the d_ vectors
        void d_clear()
        {
            this->d_x.clear();
            this->d_y.clear();
            this->d_ri.clear();
            this->d_gi.clear();
            this->d_bi.clear();
            this->d_flags.clear();
        }

        /*!
         * Save this HexGrid (and all the Hexes in it) into the HDF5 file at the
         * location @path.
         */
        void save (const std::string& path)
        {
            morph::HdfData hgdata (path);
            hgdata.add_val ("/d", d);
            hgdata.add_val ("/v", v);
            hgdata.add_val ("/x_span", x_span);
            hgdata.add_val ("/z", z);
            hgdata.add_val ("/d_rowlen", d_rowlen);
            hgdata.add_val ("/d_numrows", d_numrows);
            hgdata.add_val ("/d_size", d_size);
            hgdata.add_val ("/d_growthbuffer_horz", d_growthbuffer_horz);
            hgdata.add_val ("/d_growthbuffer_vert", d_growthbuffer_vert);

            // pair<float,float>
            hgdata.add_contained_vals ("/boundaryCentroid", boundaryCentroid);

            // Don't save BezCurvePath boundary - limit this to the ability to
            // save which hexes are boundary hexes and which aren't

            // Don't save vertexE, vertexNE etc. Make sure to set gridReduced
            // = true when calling load()

            // vector<float>
            hgdata.add_contained_vals ("/d_x", d_x);
            hgdata.add_contained_vals ("/d_y", d_y);
            hgdata.add_contained_vals ("/d_distToBoundary", d_distToBoundary);
            // vector<int>
            hgdata.add_contained_vals ("/d_ri", d_ri);
            hgdata.add_contained_vals ("/d_gi", d_gi);
            hgdata.add_contained_vals ("/d_bi", d_bi);

            hgdata.add_contained_vals ("/d_ne", d_ne);
            hgdata.add_contained_vals ("/d_nne", d_nne);
            hgdata.add_contained_vals ("/d_nnw", d_nnw);
            hgdata.add_contained_vals ("/d_nw", d_nw);
            hgdata.add_contained_vals ("/d_nsw", d_nsw);
            hgdata.add_contained_vals ("/d_nse", d_nse);

            // vector<unsigned int>
            hgdata.add_contained_vals ("/d_flags", d_flags);

            // list<Hex> hexen
            // for i in list, save Hex
            std::list<morph::Hex>::const_iterator h = this->hexen.begin();
            unsigned int hcount = 0;
            while (h != this->hexen.end()) {
                // Make up a path
                std::string h5path = "/hexen/" + std::to_string(hcount);
                h->save (hgdata, h5path);
                ++h;
                ++hcount;
            }
            hgdata.add_val ("/hcount", hcount);

            // What about vhexen? Probably don't save and re-call method to populate.
            this->renumberVectorIndices();

            // What about bhexen? Probably re-run/test this->boundaryContiguous() on load.
            this->boundaryContiguous();
        }

        /*!
         * Populate this HexGrid from the HDF5 file at the location @path.
         */
        void load (const std::string& path)
        {
            morph::HdfData hgdata (path, true);
            hgdata.read_val ("/d", this->d);
            hgdata.read_val ("/v", this->v);
            hgdata.read_val ("/x_span", this->x_span);
            hgdata.read_val ("/z", this->z);
            hgdata.read_val ("/d_rowlen", this->d_rowlen);
            hgdata.read_val ("/d_numrows", this->d_numrows);
            hgdata.read_val ("/d_size", this->d_size);
            hgdata.read_val ("/d_growthbuffer_horz", this->d_growthbuffer_horz);
            hgdata.read_val ("/d_growthbuffer_vert", this->d_growthbuffer_vert);

            hgdata.read_contained_vals ("/boundaryCentroid", this->boundaryCentroid);
            hgdata.read_contained_vals ("/d_x", this->d_x);
            hgdata.read_contained_vals ("/d_y", this->d_y);
            hgdata.read_contained_vals ("/d_distToBoundary", this->d_distToBoundary);
            hgdata.read_contained_vals ("/d_ri", this->d_ri);
            hgdata.read_contained_vals ("/d_gi", this->d_gi);
            hgdata.read_contained_vals ("/d_bi", this->d_bi);
            hgdata.read_contained_vals ("/d_ne", this->d_ne);
            hgdata.read_contained_vals ("/d_nne", this->d_nne);
            hgdata.read_contained_vals ("/d_nnw", this->d_nnw);
            hgdata.read_contained_vals ("/d_nw", this->d_nw);
            hgdata.read_contained_vals ("/d_nsw", this->d_nsw);
            hgdata.read_contained_vals ("/d_nse", this->d_nse);

            // Assume a boundary has been applied so set this true. Also, the HexGrid::save method doesn't
            // save HexGrid::vertexE, etc
            this->gridReduced = true;

            unsigned int hcount = 0;
            hgdata.read_val ("/hcount", hcount);
            for (unsigned int i = 0; i < hcount; ++i) {
                std::string h5path = "/hexen/" + std::to_string(i);
                morph::Hex h (hgdata, h5path);
                this->hexen.push_back (h);
            }

            // After creating hexen list, need to set neighbour relations in each Hex, as loaded in d_ne,
            // etc.
            for (morph::Hex& _h : this->hexen) {
                DBG ("Set neighbours for Hex " << _h.outputRG());
                // For each Hex, six loops through hexen:
                if (_h.has_ne() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_ne[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.ne = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour E relation...");
                    }
                }

                if (_h.has_nne() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nne[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.nne = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour NE relation...");
                    }
                }

                if (_h.has_nnw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nnw[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.nnw = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour NW relation...");
                    }
                }

                if (_h.has_nw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nw[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.nw = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour W relation...");
                    }
                }

                if (_h.has_nsw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nsw[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.nsw = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour SW relation...");
                    }
                }

                if (_h.has_nse() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nse[_h.vi];
                    std::list<morph::Hex>::iterator hi = this->hexen.begin();
                    while (hi != this->hexen.end()) {
                        if (hi->vi == neighb_it) {
                            matched = true;
                            _h.nse = hi;
                            break;
                        }
                        ++hi;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match hexen neighbour SE relation...");
                    }
                }
            }
        }

        /*!
         * Default constructor
         */
        HexGrid(): d(1.0f), x_span(1.0f), z(0.0f) {}

        /*!
         * Construct then load from file.
         */
        HexGrid (const std::string& path) : d(1.0f), x_span(1.0f), z(0.0f) { this->load (path); }

        /*!
         * Construct the hexagonal hex grid with a hex to hex distance of @a d_
         * (centre to centre) and approximate diameter of @a x_span_. Set z to @a z_
         * which may be useful as an identifier if several HexGrids are being managed
         * by client code, but is not otherwise made use of.
         */
        HexGrid (float d_, float x_span_, float z_ = 0.0f) : d(d_), x_span(x_span_), z(z_)
        {
            this->v = this->d * morph::mathconst<float>::root_3_over_2;
            this->init();
        }

        /*!
         * Initialise with the passed-in parameters; a hex to hex distance of @a d_
         * (centre to centre) and approximate diameter of @a x_span_. Set z to @a z_
         * which may be useful as an identifier if several HexGrids are being managed
         * by client code, but it not otherwise made use of.
         */
        void init (float d_, float x_span_, float z_ = 0.0f)
        {
            this->d = d_;
            this->v = this->d * morph::mathconst<float>::root_3_over_2;
            this->x_span = x_span_;
            this->z = z_;
            this->init();
        }

        /*!
         * Compute the centroid of the passed in list of Hexes.
         */
        std::pair<float, float> computeCentroid (const std::list<Hex>& pHexes)
        {
            std::pair<float, float> centroid;
            centroid.first = 0;
            centroid.second = 0;
            for (auto h : pHexes) {
                centroid.first += h.x;
                centroid.second += h.y;
            }
            centroid.first /= pHexes.size();
            centroid.second /= pHexes.size();
            return centroid;
        }

        /*!
         * Find the Hex in the Hex grid which is closest to the x,y position given by
         * pos.
         */
        std::list<Hex>::iterator findHexNearest (const std::pair<float, float>& pos)
        {
            std::list<morph::Hex>::iterator nearest = this->hexen.end();
            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            float dist = std::numeric_limits<float>::max();
            while (hi != this->hexen.end()) {
                float dx = pos.first - hi->x;
                float dy = pos.second - hi->y;
                float dl = std::sqrt (dx*dx + dy*dy);
                if (dl < dist) {
                    dist = dl;
                    nearest = hi;
                }
                ++hi;
            }
            return nearest;
        }

        /*!
         * Sets boundary to match the list of hexes passed in as @a pHexes. Note, that
         * unlike void setBoundary (const BezCurvePath& p), this method does not apply
         * any offset to the positions of the hexes in @a pHexes.
         */
        void setBoundary (const std::list<Hex>& pHexes)
        {
            this->boundaryCentroid = this->computeCentroid (pHexes);

            std::list<morph::Hex>::iterator bpoint = this->hexen.begin();
            std::list<morph::Hex>::iterator bpi = this->hexen.begin();
            while (bpi != this->hexen.end()) {
                std::list<morph::Hex>::const_iterator ppi = pHexes.begin();
                while (ppi != pHexes.end()) {
                    // NB: The assumption right now is that the pHexes are from the same dimension hex grid
                    // as this->hexen.
                    if (bpi->ri == ppi->ri && bpi->gi == ppi->gi) {
                        // Set h as boundary hex.
                        bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
                        bpoint = bpi;
                        break;
                    }
                    ++ppi;
                }
                ++bpi;
            }

            // Check that the boundary is contiguous.
            std::set<unsigned int> seen;
            std::list<morph::Hex>::iterator hi = bpoint;
            if (this->boundaryContiguous (bpoint, hi, seen) == false) {
                std::stringstream ee;
                ee << "The boundary is not a contiguous sequence of hexes.";
                throw std::runtime_error (ee.str());
            }

            this->discardOutsideBoundary();
            this->populate_d_vectors();
        }

        /*!
         * Sets boundary to \a p, then runs the code to discard hexes lying outside
         * this boundary. Finishes up by calling morph::HexGrid::discardOutside.
         * The BezCurvePath's centroid may not be 0,0. If loffset has its default value
         * of true, then this method offsets the boundary so that when it is applied to
         * the HexGrid, the centroid IS (0,0). If \a loffset is false, then \a p is not
         * translated in this way.
         */
        void setBoundary (const BezCurvePath<float>& p, bool loffset = true)
        {
            this->boundary = p;
            if (!this->boundary.isNull()) {
                // Compute the points on the boundary using half of the hex to hex
                // spacing as the step size. The 'true' argument inverts the y axis.
                this->boundary.computePoints (this->d/2.0f, true);
                std::vector<morph::BezCoord<float>> bpoints = this->boundary.getPoints();
                this->setBoundary (bpoints, loffset);
            }
        }

        /*!
         * This sets a boundary, just as morph::HexGrid::setBoundary(const
         * morph::BezCurvePath<float> p, bool offset) does but WITHOUT discarding hexes
         * outside the boundary. Also, it first clears the previous boundary flags so
         * the new ones are the only ones marked on the boundary. It does this because
         * it does not discard hexes outside the boundary or repopulate the HexGrid but
         * it draws a new boundary that can be used by client code
         */
        void setBoundaryOnly (const BezCurvePath<float>& p, bool loffset = true)
        {
            this->boundary = p;
            if (!this->boundary.isNull()) {
                this->boundary.computePoints (this->d/2.0f, true);
                std::vector<morph::BezCoord<float>> bpoints = this->boundary.getPoints();
                this->setBoundaryOnly (bpoints, loffset);
            }
        }

        /*!
         * Sets the boundary of the hexgrid to \a bpoints, then runs the code to discard
         * hexes lying outside this boundary. Finishes up by calling
         * HexGrid::discardOutside. By default, this method translates \a bpoints so
         * that when the boundary is applied to the HexGrid, its centroid is (0,0). If
         * the default value of \a loffset is changed to false, \a bpoints is NOT
         * translated.
         */
        void setBoundary (std::vector<BezCoord<float>>& bpoints, bool loffset = true)
        {
            this->boundaryCentroid = morph::BezCurvePath<float>::getCentroid (bpoints);

            auto bpi = bpoints.begin();
            // conditional executed if we reset the centre
            if (loffset) {
                while (bpi != bpoints.end()) {
                    bpi->subtract (this->boundaryCentroid);
                    ++bpi;
                }
                // Copy the centroid
                this->originalBoundaryCentroid = this->boundaryCentroid;
                // Zero out the centroid, as the boundary is now centred on 0,0
                this->boundaryCentroid = std::make_pair (0.0f, 0.0f);
                bpi = bpoints.begin();
            }

            // now proceed with centroid changed or unchanged
            std::list<morph::Hex>::iterator nearbyBoundaryPoint = this->hexen.begin(); // i.e the Hex at 0,0
            bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
            }

            // Check that the boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Hex>::iterator hi = nearbyBoundaryPoint;
                if (this->boundaryContiguous (nearbyBoundaryPoint, hi, seen) == false) {
                    std::stringstream ee;
                    ee << "The constructed boundary is not a contiguous sequence of hexes.";
                    throw std::runtime_error (ee.str());
                }
            }

            this->discardOutsideBoundary();
            this->populate_d_vectors();
        }

        /*!
         * This sets a boundary, just as
         * morph::HexGrid::setBoundary(vector<morph::BezCoord<float>& bpoints, bool offset)
         * does but WITHOUT discarding hexes outside the boundary. Also, it first clears
         * the previous boundary flags so the new ones are the only ones marked on the
         * boundary. It does this because it does not discard hexes outside the boundary
         * or repopulate the HexGrid but it draws a new boundary that can be used by
         * client code
         */
        void setBoundaryOnly (std::vector<BezCoord<float>>& bpoints, bool loffset)
        {
            this->boundaryCentroid = morph::BezCurvePath<float>::getCentroid (bpoints);

            auto bpi = bpoints.begin();
            // conditional executed if we reset the centre
            if (loffset) {
                while (bpi != bpoints.end()) {
                    bpi->subtract (this->boundaryCentroid);
                    ++bpi;
                }
                // Copy the centroid
                this->originalBoundaryCentroid = this->boundaryCentroid;
                // Zero out the centroid, as the boundary is now centred on 0,0
                this->boundaryCentroid = std::make_pair (0.0f, 0.0f);
                bpi = bpoints.begin();
            }

            // now proceed with centroid changed or unchanged. First: clear all boundary flags
            for (auto h : this->hexen) { h.unsetUserFlag (HEX_IS_BOUNDARY); }

            std::list<morph::Hex>::iterator nearbyBoundaryPoint = this->hexen.begin(); // i.e the Hex at 0,0
            bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
            }

            // Check that the boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Hex>::iterator hi = nearbyBoundaryPoint;
                if (this->boundaryContiguous (nearbyBoundaryPoint, hi, seen) == false) {
                    std::stringstream ee;
                    ee << "The constructed boundary is not a contiguous sequence of hexes.";
                    throw std::runtime_error (ee.str());
                }
            }
        }

        /*!
         * Set all the outer hexes as being "boundary" hexes. This makes it possible
         * to create the default hexagon of hexes, then mark the outer hexes as being
         * the boundary.
         *
         * Works only on the initial hexagonal layout of hexes.
         */
        void setBoundaryOnOuterEdge()
        {
            // From centre head to boundary, then mark boundary and walk
            // around the edge.
            std::list<morph::Hex>::iterator bpi = this->hexen.begin();
            while (bpi->has_nne()) { bpi = bpi->nne; }
            bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            while (bpi->has_ne()) {
                bpi = bpi->ne;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_nse()) {
                bpi = bpi->nse;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_nsw()) {
                bpi = bpi->nsw;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_nw()) {
                bpi = bpi->nw;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_nnw()) {
                bpi = bpi->nnw;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_nne()) {
                bpi = bpi->nne;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            while (bpi->has_ne() && bpi->ne->testFlags(HEX_IS_BOUNDARY) == false) {
                bpi = bpi->ne;
                bpi->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            }
            // Check that the boundary is contiguous.
            std::set<unsigned int> seen;
            std::list<morph::Hex>::iterator hi = bpi;
            if (this->boundaryContiguous (bpi, hi, seen) == false) {
                std::stringstream ee;
                ee << "The boundary is not a contiguous sequence of hexes.";
                throw std::runtime_error (ee.str());
            }

            // Boundary IS contiguous, discard hexes outside the boundary.
            this->discardOutsideBoundary();
            this->populate_d_vectors();
        }

        /*!
         * Get all the boundary hexes in a list. This assumes that a boundary has
         * already been set with one of the setBoundary() methods and so there is
         * therefore a set of Hexes which are already marked as being on the boundary
         * (with the attribute Hex::boundaryHex == true) Do this by going around the
         * boundary neighbour to neighbour?
         *
         * Now a getter for this->bhexen.
         */
        std::list<Hex> getBoundary() const
        {
            std::list<morph::Hex> bhexen_concrete;
            auto hh = this->bhexen.begin();
            while (hh != this->bhexen.end()) {
                bhexen_concrete.push_back (*(*hh));
                ++hh;
            }
            return bhexen_concrete;
        }

        /*!
         * Compute a set of coordinates arranged as a rectangle
         * \param x width
         * \param y height
         * \param c centre argument so that the rectangle centre is offset from the coordinate origin
         * \return A vector of the coordinates of points on the generated rectangle
         */
        std::vector<BezCoord<float>> rectangleCompute (const float x, const float y,
                                                       const std::pair<float, float> c = std::make_pair(0.0f, 0.0f))
        {
            std::vector<morph::BezCoord<float>> bpoints;
            throw std::runtime_error ("HexGrid::rectangleCompute: Implement me");
            return bpoints;
        }

        /*!
         * Compute a set of coordinates arranged as a parallelogram
         * \param re Number of hexes to the E
         * \param gne Number of hexes to the NE
         * \param rw Number of hexes to the W
         * \param gsw Number of hexes to the SW
         * \param c centre argument so that the parallelogram centre is offset from the coordinate origin
         * \return A vector of the coordinates of points on the generated pgram
         */
        std::vector<BezCoord<float>> parallelogramCompute (const int re, const int gne,
                                                           const int rw, const int gsw,
                                                           const std::pair<float, float> c = std::make_pair(0.0f, 0.0f))
        {
            std::vector<morph::BezCoord<float>> bpoints;
            // To to bottom left first
            float x = c.first - (rw * this->d + gsw * this->d/2.0f);
            float y = c.second - gsw * this->v;

            // 'Draw' bottom
            for (int i = 0; i < 2*(rw+re); ++i) {
                morph::BezCoord<float> b(std::make_pair(x, y));
                bpoints.push_back (b);
                x += this->d/2.0f;
            }
            // Right
            for (int i = 0; i < 2*(gsw+gne); ++i) {
                morph::BezCoord<float> b(std::make_pair(x, y));
                bpoints.push_back (b);
                x += this->d/4.0f;
                y += this->v/2.0f;
            }
            // Top
            for (int i = 0; i < 2*(rw+re); ++i) {
                morph::BezCoord<float> b(std::make_pair(x, y));
                bpoints.push_back (b);
                x -= this->d/2.0f;
            }
            // Left
            for (int i = 0; i < 2*(gsw+gne); ++i) {
                morph::BezCoord<float> b(std::make_pair(x, y));
                bpoints.push_back (b);
                x -= this->d/4.0f;
                y -= this->v/2.0f;
            }

            return bpoints;
        }

        /*!
         * Compute a set of coordinates arranged on an ellipse
         * \param a first elliptical radius
         * \param b second elliptical radius
         * \param c centre argument so that the ellipse centre is offset from the coordinate origin
         * \return A vector of the coordinates of points on the generated ellipse
         */
        std::vector<BezCoord<float>> ellipseCompute (const float a, const float b,
                                                     const std::pair<float, float> c = std::make_pair(0.0f, 0.0f))
        {
            // Compute the points on the boundary using the parametric elliptical formula and
            // half of the hex to hex spacing as the angular step size. Return as bpoints.
            std::vector<morph::BezCoord<float>> bpoints;

            // Estimate a good delta_phi based on the larger of a and b. Compute the delta_phi
            // required to travel a fraction of one hex-to-hex distance.
            double delta_phi = 0.0;
            double dfraction = this->d / 2.0;
            if (a > b) {
                delta_phi = std::atan2 (dfraction, a);
            } else {
                delta_phi = std::atan2 (dfraction, b);
            }

            // Loop around phi, computing x and y of the elliptical boundary and filling up bpoints
            for (double phi = 0.0; phi < morph::mathconst<double>::two_pi; phi+=delta_phi) {
                float x_pt = static_cast<float>(a * std::cos (phi) + c.first);
                float y_pt = static_cast<float>(b * std::sin (phi) + c.second);
                morph::BezCoord<float> b(std::make_pair(x_pt, y_pt));
                bpoints.push_back (b);
            }

            return bpoints;
        }

        /*!
         * calculater perimeter of ellipse with radii \a a and \a b
         */
        float ellipsePerimeter (const float a, const float b)
        {
            double apb = (double)a+b;
            double amb = (double)a-b;
            double h = amb * amb / (apb * apb);
            // Compute approximation to the ellipses perimeter (7 terms)
            double sum = 1.0
            + (0.25)      * h
            + (1.0/64.0)  * h * h
            + (1.0/256.0) * h * h * h
            + (25.0/16384.0) * h * h * h * h
            + (49.0/65536.0) * h * h * h * h * h
            + (441.0/1048576.0) * h * h * h * h * h * h;
            double p = morph::mathconst<double>::pi * apb * sum;

            return (float)p;
        }

        /*!
         * Set the boundary to be an ellipse with the given radii parameters a and b.
         * \param a first elliptical radius
         * \param b second elliptical radius
         * \param c allows the centre of the ellipse to be offset from the coordinate origin
         * \param offset determines if boundary is recentred or remains in place
         */
        void setEllipticalBoundary (const float a, const float b,
                                    const std::pair<float, float> c = std::make_pair(0.0f, 0.0f), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = ellipseCompute (a, b, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * Set the boundary to be a circle with the given radius a.
         * \param a The radius of the circle
         * \param c allows the centre of the circle to be offset from the coordinate origin
         * \param offset determines if boundary is recentred or remains in place
         */
        void setCircularBoundary (const float a,
                                  const std::pair<float, float> c = std::make_pair(0.0f, 0.0f), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = ellipseCompute (a, a, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * Set up a rectangular boundary of width x and height y.
         */
        void setRectangularBoundary (const float x, const float y,
                                     const std::pair<float, float> c = std::make_pair(0.0f, 0.0f), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = rectangleCompute (x, y, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * Set up a parallelogram boundary extending r hexes to the E and g hexes to the NE
         */
        void setParallelogramBoundary (const int r, const int g,
                                       const std::pair<float, float> c = std::make_pair(0.0f, 0.0f), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = parallelogramCompute (r, g, r, g, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * \brief Accessor for the size of hexen.
         *
         * return The number of hexes in the grid.
         */
        unsigned int num() const { return this->hexen.size(); }

        /*!
         * \brief Obtain the vector index of the last Hex in hexen.
         *
         * return Hex::vi from the last Hex in the grid.
         */
        unsigned int lastVectorIndex() const { return this->hexen.rbegin()->vi; }

        /*!
         * Output some text information about the hexgrid.
         */
        std::string output() const
        {
            std::stringstream ss;
            ss << "Hex grid with " << this->hexen.size() << " hexes.\n";
            auto i = this->hexen.begin();
            float lasty = this->hexen.front().y;
            unsigned int rownum = 0;
            ss << "\nRow/Ring " << rownum++ << ":\n";
            while (i != this->hexen.end()) {
                if (i->y > lasty) {
                    ss << "\nRow/Ring " << rownum++ << ":\n";
                    lasty = i->y;
                }
                ss << i->output() << std::endl;
                ++i;
            }
            return ss.str();
        }

        /*!
         * Show the coordinates of the vertices of the overall hex grid generated.
         */
        std::string extent() const
        {
            std::stringstream ss;
            if (gridReduced == false) {
                ss << "Grid vertices: \n"
                   << "           NW: (" << this->vertexNW->x << "," << this->vertexNW->y << ") "
                   << "      NE: (" << this->vertexNE->x << "," << this->vertexNE->y << ")\n"
                   << "     W: (" << this->vertexW->x << "," << this->vertexW->y << ") "
                   << "                              E: (" << this->vertexE->x << "," << this->vertexE->y << ")\n"
                   << "           SW: (" << this->vertexSW->x << "," << this->vertexSW->y << ") "
                   << "      SE: (" << this->vertexSE->x << "," << this->vertexSE->y << ")";
            } else {
                ss << "Initial grid vertices are no longer valid.";
            }
            return ss.str();
        }

        /*!
         * Returns the width of the HexGrid (from -x to +x)
         */
        float width() const
        {
            // {xmin, xmax, ymin, ymax, gi at xmin, gi at xmax}
            std::array<int, 6> extents = this->findBoundaryExtents();
            float xmin = this->d * float(extents[0]);
            float xmax = this->d * float(extents[1]);
            return (xmax - xmin);
        }

        /*!
         * Returns the 'depth' of the HexGrid (from -y to +y)
         */
        float depth() const
        {
            std::array<int, 6> extents = this->findBoundaryExtents();
            float ymin = this->v * float(extents[2]);
            float ymax = this->v * float(extents[3]);
            return (ymax - ymin);
        }

        /*!
         * Getter for d.
         */
        float getd() const { return this->d; }

        /*!
         * Getter for v - vertical hex spacing.
         */
        float getv() const { return this->v; }

        /*!
         * Get the shortest distance from the centre to the perimeter. This is the
         * "short radius".
         */
        float getSR() const { return this->d/2; }

        /*!
         * The distance from the centre of the Hex to any of the vertices. This is the
         * "long radius".
         */
        float getLR() const { return (this->d/morph::mathconst<float>::sqrt_of_3); }

        /*!
         * The vertical distance from the centre of the hex to the "north east" vertex
         * of the hex.
         */
        float getVtoNE() const { return (this->d/(2.0f*morph::mathconst<float>::sqrt_of_3)); }

        /*!
         * Compute and return the area of one hex in the grid. The area is that of 6
         * triangles: (1/2 LR * d/2) * 6 // or (d*d*3)/(2*sqrt(3)) = d * d * sqrt(3)/2
         */
        float getHexArea() const { return (this->d * this->d * morph::mathconst<float>::root_3_over_2); }

        /*!
         * Find the minimum value of x' on the HexGrid, where x' is the x axis rotated
         * by phi degrees.
         */
        float getXmin (float phi = 0.0f) const
        {
            float xmin = 0.0f;
            float x_ = 0.0f;
            bool first = true;
            for (auto h : this->hexen) {
                x_ = h.x * std::cos (phi) + h.y * std::sin (phi);
                if (first) {
                    xmin = x_;
                    first = false;
                }
                if (x_ < xmin) {
                    xmin = x_;
                }
            }
            return xmin;
        }

        /*!
         * Find the maximum value of x' on the HexGrid, where x' is the x axis rotated
         * by phi degrees.
         */
        float getXmax (float phi = 0.0f) const
        {
            float xmax = 0.0f;
            float x_ = 0.0f;
            bool first = true;
            for (auto h : this->hexen) {
                x_ = h.x * std::cos (phi) + h.y * std::sin (phi);
                if (first) {
                    xmax = x_;
                    first = false;
                }
                if (x_ > xmax) {
                    xmax = x_;
                }
            }
            return xmax;
        }

        /*!
         * Run through all the hexes and compute the distance to the nearest boundary
         * hex.
         */
        void computeDistanceToBoundary()
        {
            std::list<morph::Hex>::iterator h = this->hexen.begin();
            while (h != this->hexen.end()) {
                if (h->testFlags(HEX_IS_BOUNDARY) == true) {
                    h->distToBoundary = 0.0f;
                } else {
                    if (h->testFlags(HEX_INSIDE_BOUNDARY) == false) {
                        // Set to a dummy, negative value
                        h->distToBoundary = -100.0;
                    } else {
                        // Not a boundary hex, but inside boundary
                        std::list<morph::Hex>::iterator bh = this->hexen.begin();
                        while (bh != this->hexen.end()) {
                            if (bh->testFlags(HEX_IS_BOUNDARY) == true) {
                                float delta = h->distanceFrom (*bh);
                                if (delta < h->distToBoundary || h->distToBoundary < 0.0f) {
                                    h->distToBoundary = delta;
                                }
                            }
                            ++bh;
                        }
                    }
                }
                ++h;
            }
        }

        /*!
         * Populate the d_* vectors
         */
        void populate_d_vectors()
        {
            // The starting hex is always the centre one.
            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            // Clear the d_ vectors.
            this->d_clear();
            // Now raster through the hexes, building the d_ vectors.
            while (hi != this->hexen.end()) {
                this->d_push_back (hi);
                hi++;
            }
            // Set up the neighbour relations
            this->populate_d_neighbours();
        }

        /*!
         * Get a vector of Hex pointers for all hexes that are inside/on the path
         * defined by the BezCurvePath \a p, thus this gets a 'region of hexes'. The Hex
         * flags "region" and "regionBoundary" are used, temporarily to mark out the
         * region. The idea is that client code will then use the vector of morph::Hex* to work
         * with the region however it needs to.
         *
         * The centroid of the region is placed in \a regionCentroid (i.e. \a
         * regionCentroid is a return argument)
         *
         * It's assumed that the BezCurvePath defines a closed region.
         *
         * If \a applyOriginalBoundaryCentroid is true, then the region is translated by
         * the same amount that the overall boundary was translated to ensure that the
         * boundary's centroid is at 0,0.
         *
         * \return a vector of iterators to the Hexes that make up the region.
         */
        std::vector<std::list<Hex>::iterator> getRegion (BezCurvePath<float>& p, std::pair<float, float>& regionCentroid,
                                                         bool applyOriginalBoundaryCentroid = true)
        {
            p.computePoints (this->d/2.0f, true);
            std::vector<morph::BezCoord<float>> bpoints = p.getPoints();
            return this->getRegion (bpoints, regionCentroid, applyOriginalBoundaryCentroid);
        }

        /*!
         * The overload of getRegion that does all the work on a vector of coordinates
         */
        std::vector<std::list<Hex>::iterator> getRegion (std::vector<BezCoord<float>>& bpoints, std::pair<float, float>& regionCentroid,
                                                         bool applyOriginalBoundaryCentroid = true)
        {
            // First clear all region boundary flags, as we'll be defining a new region boundary
            this->clearRegionBoundaryFlags();

            // Compute region centroid from bpoints
            regionCentroid = morph::BezCurvePath<float>::getCentroid (bpoints);

            // A return object
            std::vector<std::list<morph::Hex>::iterator> theRegion;

            if (applyOriginalBoundaryCentroid) {
                auto bpi = bpoints.begin();
                while (bpi != bpoints.end()) {
                    bpi->subtract (this->originalBoundaryCentroid);
                    ++bpi;
                }

                // Subtract originalBoundaryCentroid from region centroid so that region centroid is translated
                regionCentroid.first = regionCentroid.first - this->originalBoundaryCentroid.first;
                regionCentroid.second = regionCentroid.second - this->originalBoundaryCentroid.second;
            }

            // Now find the hexes on the boundary of the region
            std::list<morph::Hex>::iterator nearbyRegionBoundaryPoint = this->hexen.begin(); // i.e the Hex at 0,0
            typename std::vector<morph::BezCoord<float>>::iterator bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyRegionBoundaryPoint = this->setRegionBoundary (*bpi++, nearbyRegionBoundaryPoint);
            }

            // Check that the region boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Hex>::iterator hi = nearbyRegionBoundaryPoint;
                if (this->regionBoundaryContiguous (nearbyRegionBoundaryPoint, hi, seen) == false) {
                    std::stringstream ee;
                    ee << "The constructed region boundary is not a contiguous sequence of hexes.";
                    return theRegion;
                }
            }

            // Mark hexes inside region. Use centroid of the region.
            std::list<morph::Hex>::iterator insideRegionHex = this->findHexNearest (regionCentroid);
            this->markHexesInside (insideRegionHex, HEX_IS_REGION_BOUNDARY, HEX_INSIDE_REGION);

            // Populate theRegion, then return it
            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->testFlags (HEX_INSIDE_REGION) == true) {
                    theRegion.push_back (hi);
                }
                ++hi;
            }

            return theRegion;
        }

        //! Obtain a hexagonal region of hexes around a given central hex, marked by its
        //! d_ index. This is easier than getting a properly circular region of hexes.
        std::vector<std::list<Hex>::iterator> getHexagonalRegion (unsigned int centreindex, float radius)
        {
            std::vector<std::list<morph::Hex>::iterator> theRegion;

            // Find the hex with index centreindex
            std::list<Hex>::iterator sh = this->hexen.begin(); // start hex
            while (sh != this->hexen.end()) {
                if (sh->vi == centreindex) { break; }
                sh++;
            }

            // Return if we didn't find the start hex
            if (sh == this->hexen.end()) { return theRegion; }

            theRegion.push_back (sh);
            // For each of 6 directions, step out to collect up the hexes on the disc
            // ring by ring. For rings 2 and above, also need to fill in hexes
            // (otherwise you end up with a snowflake shaped disc)
            std::list<Hex>::iterator h;
            std::list<Hex>::iterator h2; // for the tangent direction
            for (unsigned short i = 0; i < 6; ++i) {
                h = sh;
                if (h->has_neighbour(i)) {
                    h = h->get_neighbour(i);
                    theRegion.push_back (h);
                    int j = 1;
                    unsigned short tangentdir = (i+4)%6;
                    while (this->d*j < radius) {
                        if (h->has_neighbour(i)) {
                            h = h->get_neighbour(i);
                            theRegion.push_back (h);
                            h2 = h;
                            for (int k = 0; k<=(j-1); ++k) {
                                // Go in tangentdir
                                if (h2->has_neighbour (tangentdir)) {
                                    h2 = h2->get_neighbour (tangentdir);
                                    theRegion.push_back (h2);
                                }
                            }
                        } else {
                            break;
                        }
                        ++j;
                    }
                }
            }
            return theRegion;
        }

        /*!
         * For every hex in hexen, unset the flags HEX_IS_REGION_BOUNDARY and
         * HEX_INSIDE_REGION
         */
        void clearRegionBoundaryFlags()
        {
            for (auto& hh : this->hexen) {
                hh.unsetFlag (HEX_IS_REGION_BOUNDARY | HEX_INSIDE_REGION);
            }
        }

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
         * Resampling function (monochrome).
         *
         * \param image_data (input) The monochrome image as a vVector of floats.
         * \param image_pixelwidth (input) The number of pixels that the image is wide
         * \param image_scale (input) The size that the image should be resampled to (same units as HexGrid)
         * \param image_offset (input) An offset in HexGrid units to shift the image wrt to the HexGrid's origin
         * \param sigma (input) The sigma for the 2D resampling Gaussian
         *
         * \return A new data vVector containing the resampled (and renormalised) hex pixel values
         */
        morph::vVector<float> resampleImage (const morph::vVector<float>& image_data,
                                             const unsigned int image_pixelwidth,
                                             const morph::Vector<float, 2>& image_scale,
                                             const morph::Vector<float, 2>& image_offset)
        {
            unsigned int csz = image_data.size();
            morph::Vector<unsigned int, 2> image_pixelsz = {image_pixelwidth, csz / image_pixelwidth};
            // distance per pixel in the image. This defines the Gaussian width (sigma) for the resample:
            morph::Vector<float, 2> dist_per_pix = image_scale / (image_pixelsz-1);
            morph::Vector<float, 2> half_scale = image_scale * 0.5f;
            morph::Vector<float, 2> params = 1.0f / (2.0f * dist_per_pix * dist_per_pix);
            morph::Vector<float, 2> threesig = 3.0f * dist_per_pix;

            morph::vVector<float> expr_resampled(this->num(), 0.0f);
#pragma omp parallel for // parallel on this outer loop gives best result (5.8 s vs 7 s)
            for (size_t xi = 0; xi < this->d_x.size(); ++xi) {
                float expr = 0.0f;
//#pragma omp parallel for reduction(+:expr)
                for (unsigned int i = 0; i < csz; ++i) {
                    // Get x/y pixel coords:
                    morph::Vector<unsigned int, 2> idx = {(i % image_pixelsz[0]), (image_pixelsz[1] - (i / image_pixelsz[1]))};
                    // Get the coordinates of the pixel at index i:
                    morph::Vector<float, 2> posn = (dist_per_pix * idx) - half_scale + image_offset;
                    // Distance from input pixel to output hex:
                    float _d_x = this->d_x[xi] - posn[0];
                    float _d_y = this->d_y[xi] - posn[1];
                    // Compute contributions to each hex pixel, using 2D (elliptical) Gaussian
                    if (_d_x < threesig[0] && _d_y < threesig[1]) { // Testing for distance gives slight speedup
                        expr += std::exp ( - ( (params[0] * _d_x * _d_x) + (params[1] * _d_y * _d_y) ) ) * image_data[i];
                    }
                }
                expr_resampled[xi] = expr;
            }

            expr_resampled /= expr_resampled.max(); // renormalise result
            return expr_resampled;
        }

        // Member attributes for visualising the compute_hex_overlap stuff. Put in class HexOverlapGeometry or something
        Vector<float, 2> sw_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> nw_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> ne_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> se_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> n_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> s_loc = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        // Original location of the zero hex
        Vector<float, 2> sw_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> nw_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> ne_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> se_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> n_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> s_0 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        //
        Vector<float, 2> sw_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> nw_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> ne_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> se_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> n_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> s_sft = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        //
        Vector<float, 2> p1 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q1 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p2 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q2 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p3 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q3 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p4 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q4 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p5 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q5 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p6 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q6 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q7 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> p8 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> q8 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        //
        // Usually, top left of rectangle a1 is p2, but it may be q4 if i5 is to 'left' of i1
        Vector<float, 2> a1_tl = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> a1_bl = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        //
        Vector<float, 2> i1 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> i2 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> i3 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> i4 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> i5 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> i6 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};

        // More vectors for visualisation. The 60 and 300 degree unit vectors are used to compute area of parallelogram.
        Vector<float, 2> unit_60 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_300 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_120 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_150 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_240 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_210 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> unit_30 = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};

        Vector<float, 2> pll1_top = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> pll1_br = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> pll2_bot = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
        Vector<float, 2> pll2_tr = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};

        static constexpr bool debug_hexshift = false;

        // Shift data by dx, with wrapping if set for the hexgrid
        template <typename T>
        bool shiftdata (morph::vVector<T>& image_data, const morph::Vector<float, 2>& dx)
        {
            unsigned int csz = image_data.size();
            morph::vVector<T> shifted(csz, T{0});

            // If g and r are purely integral, then the transform is simple; copy data
            // from hex(i,j) in image_data to hex(i+r,j+g) in shifted, where the +r and
            // +g are steps via neighbour relations (to ensure wrapping works)

            // Otherwise, distribute data from hex(i,j) of image_data proportionally between 4 hexes.

            // How many 'r' steps and how many 'g' steps does the vector dx represent?
            if constexpr (debug_hexshift) { std::cout << "d = " << this->d << ", dx = " << dx << std::endl; }
            Vector<float, 2> rg = {
                (1.0f/this->d) * (dx[0] - dx[1] * morph::mathconst<float>::one_over_root_3),
                (1.0f/this->d) * (dx[1] * morph::mathconst<float>::two_over_root_3)
            };
            if constexpr (debug_hexshift) { std::cout << "Movement expressed as r/g is rg=" << rg << std::endl; }
            // How many integral steps in r and g axes?
            Vector<float, 2> int_rg_f = rg.trunc();
            // Convert to int
            Vector<int, 2> int_rg = { static_cast<int>(std::round (int_rg_f[0])), static_cast<int>(std::round (int_rg_f[1])) };
            if constexpr (debug_hexshift) { std::cout << "integral steps: " << int_rg << std::endl; }
            Vector<float, 2> int_xy = {
                (int_rg_f[0] * this->d + int_rg_f[1] * this->d * 0.5f),
                (int_rg_f[1] * this->v)
            };
            // Compute remainder
            Vector<float, 2> rem_rg = rg - int_rg_f;

            if constexpr (debug_hexshift) { std::cout << "Remainder r: " << rem_rg[0] << ", and remainder g: " << rem_rg[1] << std::endl; }
            // The remainder movement in Cartesian coordinates
            Vector<float, 2> rem_xy = {
                (rem_rg[0] * this->d + rem_rg[1] * this->d * 0.5f),
                (rem_rg[1] * this->v)
            };
            if constexpr (debug_hexshift) { std::cout << "Remainder x: " << rem_xy[0] << ", and remainder y: " << rem_xy[1] << std::endl; }

            // Corners of base hex 0
            sw_loc = { (-d*0.5f), (-d*morph::mathconst<float>::one_over_2_root_3) };
            nw_loc = { (-d*0.5f), ( d*morph::mathconst<float>::one_over_2_root_3) };
            ne_loc = {  (d*0.5f), ( d*morph::mathconst<float>::one_over_2_root_3) };
            se_loc = {  (d*0.5f), (-d*morph::mathconst<float>::one_over_2_root_3) };
            n_loc =  {   0.0f   , ( d*morph::mathconst<float>::one_over_root_3)   };
            s_loc =  {   0.0f   , (-d*morph::mathconst<float>::one_over_root_3)   };

            // Origin hex
            sw_0 = sw_loc - int_xy;
            nw_0 = nw_loc - int_xy;
            ne_0 = ne_loc - int_xy;
            se_0 = se_loc - int_xy;
            n_0 = n_loc - int_xy;
            s_0 = s_loc - int_xy;

            // Corners of the shifted hex
            sw_sft = sw_loc + rem_xy;
            nw_sft = nw_loc + rem_xy;
            ne_sft = ne_loc + rem_xy;
            se_sft = se_loc + rem_xy;
            n_sft = n_loc + rem_xy;
            s_sft = s_loc + rem_xy;

            Vector<float, 19> overlap = this->compute_hex_overlap (rem_xy);

            if (overlap[0] == -100.0f) {
                if constexpr (debug_hexshift) { std::cout << "overlap[0] is -100\n"; }
                return false;
            }

            static constexpr bool debugdata = false;

            std::list<Hex>::iterator h = this->hexen.begin();
            while (h != this->hexen.end()) {
                // image_data[i] is the data to shift.
                bool datatocopy = false;
                if constexpr (debugdata) {
                    datatocopy = image_data[h->vi] > T{0} ? true : false;
                }
                std::list<Hex>::iterator dest_hex = h;
                if (datatocopy) std::cout << "Copying hex data at " << h->outputRG() << "...";
                if (int_rg[1] > 0) {
                    for (int j = 0; j < int_rg[1] && dest_hex->has_nne(); ++j) {
                        dest_hex = dest_hex->nne;
                    }
                } else {
                    for (int j = 0; j > int_rg[1] && dest_hex->has_nsw(); --j) {
                        dest_hex = dest_hex->nsw;
                    }
                }
                if (int_rg[0] > 0) {
                    for (int j = 0; j < int_rg[0] && dest_hex->has_ne(); ++j) {
                        dest_hex = dest_hex->ne;
                    }
                } else {
                    for (int j = 0; j > int_rg[0] && dest_hex->has_nw(); --j) {
                        dest_hex = dest_hex->nw;
                    }
                }
                // dest_hex should now be set
                if constexpr (debugdata) { if (datatocopy) { std::cout << " to desthex: " << dest_hex->outputRG() << std::endl; } }

                // Having computed all the overlaps:
                if (datatocopy && overlap[0]) std::cout << "Adding [0] " << (overlap[0]*100.0f) << "\% to dest_hex itself\n";
                shifted[dest_hex->vi] += overlap[0] * image_data[h->vi];

                if (dest_hex->has_ne()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[1]) { std::cout << "Adding [1] " << (overlap[1]*100.0f) << "\% to dest_hex ne\n"; } }
                    shifted[dest_hex->ne->vi] += overlap[1] * image_data[h->vi];

                    if (dest_hex->ne->has_ne()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[8]) { std::cout << "Adding [8] " << (overlap[8]*100.0f) << "\% to dest_hex ne->ne\n";} }
                        shifted[dest_hex->ne->ne->vi] += overlap[8] * image_data[h->vi];
                    }
                    if (dest_hex->ne->has_nne()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[9]) { std::cout << "Adding [9] " << (overlap[9]*100.0f) << "\% to dest_hex ne->nne\n"; } }
                        shifted[dest_hex->ne->nne->vi] += overlap[9] * image_data[h->vi];
                    }
                } else {
                    std::cout << "No Neighbour E?? dest_hex " << dest_hex->outputCart() << " has no neighbour east.\n";
                }

                if (dest_hex->has_nne()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[2]) { std::cout << "Adding [2] " << (overlap[2]*100.0f) << "\% to dest_hex nne\n"; } }
                    shifted[dest_hex->nne->vi] += overlap[2] * image_data[h->vi];
                    if (dest_hex->nne->has_nne()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[10]) { std::cout << "Adding [10] " << (overlap[10]*100.0f) << "\% to dest_hex nne->nne\n"; } }
                        shifted[dest_hex->nne->nne->vi] += overlap[10] * image_data[h->vi];
                    }
                    if (dest_hex->nne->has_nnw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[11]) { std::cout << "Adding [11] " << (overlap[11]*100.0f) << "\% to dest_hex nne->nnw\n"; } }
                        shifted[dest_hex->nne->nnw->vi] += overlap[11] * image_data[h->vi];
                    }
                }
                if (dest_hex->has_nnw()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[3]) { std::cout << "Adding [3] " << (overlap[3]*100.0f) << "\% to dest_hex nnw\n"; } }
                    shifted[dest_hex->nnw->vi] += overlap[3] * image_data[h->vi];
                    if (dest_hex->nnw->has_nnw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[12]) { std::cout << "Adding [12] " << (overlap[12]*100.0f) << "\% to dest_hex nnw->nnw\n"; } }
                        shifted[dest_hex->nnw->nnw->vi] += overlap[12] * image_data[h->vi];
                    }
                    if (dest_hex->nnw->has_nw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[13]) { std::cout << "Adding [13] " << (overlap[13]*100.0f) << "\% to dest_hex nnw->nw\n"; } }
                        shifted[dest_hex->nnw->nw->vi] += overlap[13] * image_data[h->vi];
                    }
                }
                if (dest_hex->has_nw()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[4]) { std::cout << "Adding [4] " << (overlap[4]*100.0f) << "\% to dest_hex nw\n"; } }
                    shifted[dest_hex->nw->vi] += overlap[4] * image_data[h->vi];
                    if (dest_hex->nw->has_nw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[14]) { std::cout << "Adding [14] " << (overlap[14]*100.0f) << "\% to dest_hex nw->nw\n"; } }
                        shifted[dest_hex->nw->nw->vi] += overlap[14] * image_data[h->vi];
                    }
                    if (dest_hex->nw->has_nsw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[15]) { std::cout << "Adding [15] " << (overlap[15]*100.0f) << "\% to dest_hex nw->nsw\n"; } }
                        shifted[dest_hex->nw->nsw->vi] += overlap[15] * image_data[h->vi];
                    }
                }
                if (dest_hex->has_nsw()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[5]) { std::cout << "Adding [5] " << (overlap[5]*100.0f) << "\% to dest_hex nsw\n"; } }
                    shifted[dest_hex->nsw->vi] += overlap[5] * image_data[h->vi];
                    if (dest_hex->nsw->has_nsw()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[16]) { std::cout << "Adding [16] " << (overlap[16]*100.0f) << "\% to dest_hex nsw->nsw\n"; } }
                        shifted[dest_hex->nsw->nsw->vi] += overlap[16] * image_data[h->vi];
                    }
                    if (dest_hex->nsw->has_nse()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[17]) { std::cout << "Adding [17] " << (overlap[17]*100.0f) << "\% to dest_hex nsw->nse\n"; } }
                        shifted[dest_hex->nsw->nse->vi] += overlap[17] * image_data[h->vi];
                    }
                }
                if (dest_hex->has_nse()) {
                    if constexpr (debugdata) { if (datatocopy && overlap[6]) { std::cout << "Adding [6] " << (overlap[6]*100.0f) << "\% to dest_hex nse\n"; } }
                    shifted[dest_hex->nse->vi] += overlap[6] * image_data[h->vi];
                    if (dest_hex->nse->has_nse()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[18]) { std::cout << "Adding [18] " << (overlap[18]*100.0f) << "\% to dest_hex nse->nse\n"; } }
                        shifted[dest_hex->nse->nse->vi] += overlap[18] * image_data[h->vi];
                    }
                    if (dest_hex->nse->has_ne()) {
                        if constexpr (debugdata) { if (datatocopy && overlap[7]) { std::cout << "Adding [7] " << (overlap[7]*100.0f) << "\% to dest_hex nse->ne\n"; } }
                        shifted[dest_hex->nse->ne->vi] += overlap[7] * image_data[h->vi];
                    }
                } else {
                    if constexpr (debugdata) { std::cout << "No nse for hex " << dest_hex->outputRG() << "\n"; }
                }

                ++h;
            }

            std::copy (shifted.begin(), shifted.end(), image_data.begin());
            return true;
        }

        /*!
         * Find the intersection point between two line segments. The first segment runs
         * from position _p1 to _q1 and the second from position _p2 to _q2. If no
         * intersect, return object should contain the minimum representation of a
         * float.
         * See https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
         */
        Vector<float, 2> intersection (const Vector<float, 2> _p1, const Vector<float, 2> _q1,
                                       const Vector<float, 2> _p2, const Vector<float, 2> _q2)
        {
            Vector<float, 2> isect;
            isect.set_from (std::numeric_limits<float>::quiet_NaN());

            // _p1 = p; _q1 = p + r
            // _p2 = q; _q2 = q + s
            float q_m_pxr = (_p2-_p1).cross(_q1-_p1);
            float rxs = (_q1-_p1).cross(_q2-_p2);
            float u = -1.0f;
            float t = -1.0f;
            if (rxs != 0.0f) {
                u = q_m_pxr / rxs; // u = (q-p) x r/(r x s)
                float den = (_q1-_p1).cross(_q2-_p2);
                if (den != 0.0f) {
                    t = (_p2-_p1).cross(_q2-_p2) / den;
                }
            }
            if (rxs == 0.0f && q_m_pxr == 0.0f) {
                // Colinear, figure out if overlapping
                float t_0 = (_p2 - _p1).dot (_q1-_p1) / (_q1-_p1).dot(_q1-_p1);
                float t_1 = (_q2 - _p1).dot (_q1-_p1) / (_q1-_p1).dot(_q1-_p1);
                if (t_0 > 0.0f || t_0 < 1.0f || t_1 > 0.0f || t_1 < 1.0f) {
                    isect = _p1 + t_0 * (_q1-_p1);
                } else {
                    isect[1] = 0.0f; // isect[0] remains nan
                }

            } else if (rxs == 0.0f && q_m_pxr != 0.0f) {
                // Parallel and non-intersecting. Place distance between lines into isect[0], leave other one NaN.
                // Get normal to p2-q2
                morph::Matrix22<float> rot90;
                rot90.rotate (morph::mathconst<float>::pi_over_2);
                Vector<float, 2> nor = rot90 * (_q2-_p2);
                // Project p2-p1 onto normal
                float d_p1 = (_p2-_p1).dot(nor);
                isect[0] = d_p1; // isect[1] remains nan

            } else if (rxs != 0.0f && t > 0.0f && t < 1.0f && u > 0.0f && u < 1.0f) {
                // Lines intersect
                isect = _p2 + u * (_q2-_p2);
            } // else non intersecting

            return isect.as_float();
        }

        /*!
         * Compute the overlap of a (grid sized) hex shifted by the 2D Cartesian vector
         * 'shift' on the adjacent hexes. The return Vector has 13 elements that give
         * the overlap between the shifted hex and the underlying hexes, which are 0:
         * centre; 1:ne, 2:nw, 3:w 4:sw 5:se 6:e 7:ene 8:nn 9:wnw 10:wsw 11:ss 12:ese An
         * assumption is that shift is small.
         */
        Vector<float, 19> compute_hex_overlap (Vector<float, 2> shift)
        {
            Vector<float, 19> overlap;
            overlap.zero();
            float lr = this->getLR();

            // hexvectors from centre to points:
            morph::Vector<float, 2> hv_ne = { morph::mathconst<float>::root_3_over_2 * lr, 0.5f * lr };
            morph::Vector<float, 2> hv_n =  { 0, lr };
            morph::Vector<float, 2> hv_nw = {-morph::mathconst<float>::root_3_over_2 * lr, 0.5f * lr};
            morph::Vector<float, 2> hv_sw = {-morph::mathconst<float>::root_3_over_2 * lr, -0.5f * lr};
            morph::Vector<float, 2> hv_s =  { 0, -lr };
            morph::Vector<float, 2> hv_se = { morph::mathconst<float>::root_3_over_2 * lr, -0.5f * lr};

            // Find intersections between sides of the base hex and the shifted
            // hex. This will inform which of the different area computation algorithms to

            // Intersection 1 is between n->ne of base and nw->n of shifted
            Vector<float, 2> isct1 = this->intersection (n_loc, ne_loc, nw_sft, n_sft);
            Vector<float, 2> isct2 = this->intersection (nw_loc, n_loc, sw_sft, nw_sft);
            Vector<float, 2> isct3 = this->intersection (sw_loc, nw_loc, s_sft, sw_sft);
            Vector<float, 2> isct4 = this->intersection (s_loc, sw_loc, se_sft, s_sft);
            Vector<float, 2> isct5 = this->intersection (se_loc, s_loc, ne_sft, se_sft);
            Vector<float, 2> isct6 = this->intersection (ne_loc, se_loc, n_sft, ne_sft);
            if constexpr (debug_hexshift) { std::cout << "isects: " << isct1 << ", " << isct2 << ", " << isct3 << ", "
                                                      << isct4 << ", " << isct5 << ", " << isct6 << "\n"; }

            // Parallel intersections
            Vector<float, 2> isct7 = this->intersection (nw_loc, n_loc, nw_sft, n_sft);
            Vector<float, 2> isct8 = this->intersection (sw_loc, nw_loc, sw_sft, nw_sft);
            Vector<float, 2> isct9 = this->intersection (s_loc, sw_loc, s_sft, sw_sft);
            if constexpr (debug_hexshift) { std::cout << "colinear isects: " << isct7 << ", " << isct8 << ", " << isct9 << std::endl; }
            // Note: the segments could be parallel but no longer intersecting and there
            // would STILL be the possiblity of an overlap. However, I think that the
            // scheme of choosing integer hex-jumps may avoid this case from occurring.

            // Colinear intersections with adjacent hexes
            Vector<float, 2> isct10 = this->intersection (n_loc, n_loc + hv_n, sw_sft, nw_sft);
            Vector<float, 2> isct11 = this->intersection (nw_loc, nw_loc + hv_nw, s_sft, sw_sft);
            Vector<float, 2> isct12 = this->intersection (sw_loc, sw_loc + hv_sw, se_sft, s_sft);
            Vector<float, 2> isct13 = this->intersection (s_loc, s_loc + hv_s, ne_sft, se_sft);
            Vector<float, 2> isct14 = this->intersection (se_loc, se_loc + hv_se, n_sft, ne_sft);
            Vector<float, 2> isct15 = this->intersection (ne_loc, ne_loc + hv_ne, nw_sft, n_sft);
            if constexpr (debug_hexshift) { std::cout << "extra colinear isects1-3: " << isct10 << ", " << isct11 << ", " << isct12 << std::endl; }
            if constexpr (debug_hexshift) { std::cout << "extra colinear isects4-6: " << isct13 << ", " << isct14 << ", " << isct15 << std::endl; }

            // Nearly finally, intersects that occur when the hex is moved to the point at
            // which the moved hex has just one point inside the base hex
            Vector<float, 2> isct16 = this->intersection (n_loc, ne_loc, nw_sft, sw_sft);
            Vector<float, 2> isct17 = this->intersection (nw_loc, n_loc, sw_sft, s_sft);
            Vector<float, 2> isct18 = this->intersection (sw_loc, nw_loc, s_sft, se_sft);
            Vector<float, 2> isct19 = this->intersection (s_loc, sw_loc, se_sft, ne_sft);
            Vector<float, 2> isct20 = this->intersection (se_loc, s_loc, ne_sft, n_sft);
            Vector<float, 2> isct21 = this->intersection (ne_loc, se_loc, n_sft, nw_sft);
            if constexpr (debug_hexshift) { std::cout << "corner isects1-3: " << isct16 << ", " << isct17 << ", " << isct18 << std::endl; }
            if constexpr (debug_hexshift) { std::cout << "corner isects4-6: " << isct19 << ", " << isct20 << ", " << isct21 << std::endl; }

            // And intersects that may occur if the hex is moved slightly beyond the
            // original base hex (next to base hex NE edge in rotation=0 case)
            Vector<float, 2> isct22 = this->intersection (ne_loc, ne_loc+hv_ne, s_sft, sw_sft);
            Vector<float, 2> isct23 = this->intersection (n_loc, n_loc+hv_n, se_sft, s_sft);
            Vector<float, 2> isct24 = this->intersection (nw_loc, nw_loc+hv_nw, se_sft, ne_sft);
            Vector<float, 2> isct25 = this->intersection (sw_loc, sw_loc+hv_sw, n_sft, ne_sft);
            Vector<float, 2> isct26 = this->intersection (s_loc, s_loc+hv_s, nw_sft, n_sft);
            Vector<float, 2> isct27 = this->intersection (se_loc, se_loc+hv_se, sw_sft, nw_sft);
            if constexpr (debug_hexshift) { std::cout << "far isects1-3: " << isct22 << ", " << isct23 << ", " << isct24 << std::endl; }
            if constexpr (debug_hexshift) { std::cout << "far isects4-6: " << isct25 << ", " << isct26 << ", " << isct27 << std::endl; }

            // And intersects that may occur if the hex is moved slightly beyond the
            // original base hex (next to base hex E edge in rotation=0 case)
            Vector<float, 2> isct28 = this->intersection (ne_loc, ne_loc+hv_ne, sw_sft, nw_sft);
            Vector<float, 2> isct29 = this->intersection (n_loc, n_loc+hv_n, s_sft, sw_sft);
            Vector<float, 2> isct30 = this->intersection (nw_loc, nw_loc+hv_nw, se_sft, s_sft);
            Vector<float, 2> isct31 = this->intersection (sw_loc, sw_loc+hv_sw, ne_sft, se_sft);
            Vector<float, 2> isct32 = this->intersection (s_loc, s_loc+hv_s, n_sft, ne_sft);
            Vector<float, 2> isct33 = this->intersection (se_loc, se_loc+hv_se, nw_sft, n_sft);
            if constexpr (debug_hexshift) { std::cout << "far2 isects1-3: " << isct28 << ", " << isct29 << ", " << isct30 << std::endl; }
            if constexpr (debug_hexshift) { std::cout << "far2 isects4-6: " << isct31 << ", " << isct32 << ", " << isct33 << std::endl; }

            // If angle of shift is within this threshold of pi/6, 3pi/6, 5pi/6 etc, then we compute_overlap_colinear.
            float anglethreshold = 2.0f * std::numeric_limits<float>::epsilon();

            // Depending on values in isct1-6, select a this->compute_overlap() overload
            // or pass a relevant rotation in.
            if (!isct1.has_nan()) {
                overlap = this->compute_overlap (0);
            } else if (!isct2.has_nan()) {
                overlap = this->compute_overlap (1); // with 60 degree rotation
            } else if (!isct3.has_nan()) {
                overlap = this->compute_overlap (2);
            } else if (!isct4.has_nan()) {
                overlap = this->compute_overlap (3);
            } else if (!isct5.has_nan()) {
                overlap = this->compute_overlap (4);
            } else if (!isct6.has_nan()) {
                overlap = this->compute_overlap (5);

            } else if (!isct16.has_nan()) {
                overlap = this->compute_overlap_corner(0);
            } else if (!isct17.has_nan()) {
                overlap = this->compute_overlap_corner(1);
            } else if (!isct18.has_nan()) {
                overlap = this->compute_overlap_corner(2);
            } else if (!isct19.has_nan()) {
                overlap = this->compute_overlap_corner(3);
            } else if (!isct20.has_nan()) {
                overlap = this->compute_overlap_corner(4);
            } else if (!isct21.has_nan()) {
                overlap = this->compute_overlap_corner(5);

            } else if (!isct22.has_nan()) {
                overlap = this->compute_overlap_far (0);
            } else if (!isct23.has_nan()) {
                overlap = this->compute_overlap_far (1);
            } else if (!isct24.has_nan()) {
                overlap = this->compute_overlap_far (2);
            } else if (!isct25.has_nan()) {
                overlap = this->compute_overlap_far (3);
            } else if (!isct26.has_nan()) {
                overlap = this->compute_overlap_far (4);
            } else if (!isct27.has_nan()) {
                overlap = this->compute_overlap_far (5);

            } else if (!isct28.has_nan()) {
                overlap = this->compute_overlap_far2 (0);
            } else if (!isct29.has_nan()) {
                overlap = this->compute_overlap_far2 (1);
            } else if (!isct30.has_nan()) {
                overlap = this->compute_overlap_far2 (2);
            } else if (!isct31.has_nan()) {
                overlap = this->compute_overlap_far2 (3);
            } else if (!isct32.has_nan()) {
                overlap = this->compute_overlap_far2 (4);
            } else if (!isct33.has_nan()) {
                overlap = this->compute_overlap_far2 (5);

            } else if (!isct10.has_nan() && std::isnan(isct13[0])) {
                overlap = this->compute_overlap_colinear2 (0);
            } else if (!isct11.has_nan()) {
                overlap = this->compute_overlap_colinear2 (1);
            } else if (!isct12.has_nan()) {
                overlap = this->compute_overlap_colinear2 (2);
            } else if (!isct13.has_nan()) {
                overlap = this->compute_overlap_colinear2 (3);
            } else if (!isct14.has_nan()) {
                overlap = this->compute_overlap_colinear2 (4);
            } else if (!isct15.has_nan()) {
                overlap = this->compute_overlap_colinear2 (5);

            } else if (!isct10.has_nan() && !std::isnan(isct13[0])) {
                overlap = this->compute_overlap_colinear3 (0);
            } else if (!isct11.has_nan() && !std::isnan(isct14[0])) {
                overlap = this->compute_overlap_colinear3 (1);
            } else if (!isct12.has_nan() && !std::isnan(isct15[0])) {
                overlap = this->compute_overlap_colinear3 (2);
            } else if (!isct13.has_nan() && !std::isnan(isct10[0])) {
                overlap = this->compute_overlap_colinear3 (3);
            } else if (!isct14.has_nan() && !std::isnan(isct11[0])) {
                overlap = this->compute_overlap_colinear3 (4);
            } else if (!isct15.has_nan() && !std::isnan(isct12[0])) {
                overlap = this->compute_overlap_colinear3 (5);

            } else if (!isct7.has_nan() || !isct8.has_nan() || !isct9.has_nan() // 1st three test for colinear edges
                       || (std::abs(shift.angle()    -   morph::mathconst<float>::pi_over_6) <= anglethreshold) // rest test for shift angle
                       || (std::abs(shift.angle()    +   morph::mathconst<float>::pi_over_6) <= anglethreshold) // rest test for shift angle
                       || (std::abs(shift.angle() - 3.0f*morph::mathconst<float>::pi_over_6) <= anglethreshold)
                       || (std::abs(shift.angle() + 3.0f*morph::mathconst<float>::pi_over_6) <= anglethreshold)
                       || (std::abs(shift.angle() - 5.0f*morph::mathconst<float>::pi_over_6) <= anglethreshold)
                       || (std::abs(shift.angle() + 5.0f*morph::mathconst<float>::pi_over_6) <= anglethreshold)) {
                overlap = this->compute_overlap_colinear();

            } else {
                if constexpr (debug_hexshift) { std::cout << "huh? shift.angle() = " << shift.angle() << std::endl; }
            }
            if constexpr (debug_hexshift) {
                VAR(overlap);
                VAR(overlap.sum());
            }
            if (overlap.sum() == 0.0f) {
                overlap[0] = -100.0f; // Used as a signal, later
            }

            return overlap;
        }

        /*!
         * Simpler overlap computation for hexes sliding along parallel edges Call this
         * after you have determined that any of the hexagon sides are colinear
         */
        Vector<float, 19> compute_overlap_colinear()
        {
            if constexpr (debug_hexshift) { std::cout << __FUNCTION__ << " called\n"; }

            // Store the hex overlap proportions in the return object
            Vector<float, 19> rtn;
            rtn.zero();

            float hexarea = this->hexen.begin()->getArea();

            // Pairs of corners. Assume they don't go beyond each other.
            Vector<float, 2> n_s = s_loc - n_sft;
            Vector<float, 2> s_n = n_loc - s_sft;

            Vector<float, 2> ne_sw = sw_loc - ne_sft;
            Vector<float, 2> sw_ne = ne_loc - sw_sft;

            Vector<float, 2> se_nw = nw_loc - se_sft;
            Vector<float, 2> nw_se = se_loc - nw_sft;

            // Find the one with the minimum distance and make sure this distance is less than 2*getLR()
            Vector<float, 6> pps; // point to point distances
            pps[0] = n_s.length();
            pps[1] = s_n.length();
            pps[2] = ne_sw.length();
            pps[3] = sw_ne.length();
            pps[4] = se_nw.length();
            pps[5] = nw_se.length();
            float minpp = pps.min();
            float a1 = 0.0f;
            float t1 = 0.0f;
            float lr = this->getLR();

            if (minpp < 2.0f*lr) {
                // Ok, one of pps is less than the long distance across a hex
                if (minpp >= lr) {
                    // Area is triangles plus rectangle
                    a1 = (minpp-lr) * this->d;
                    t1 = 0.5f * this->d * lr ;

                    // The overlap on adjacent hexes is a parallelogram of side lr and end dimension 2*lr - minpp
                    float pw = (2.0f * lr - minpp) * morph::mathconst<float>::root_3_over_2;

                    // use pps.argmin to find out which of the directions the shift is in
                    unsigned int hidx = pps.argmin();

                    rtn[0] = (a1 + t1) / hexarea;

                    // From Hex.h. HEX_NEIGHBOUR_POS_E = 0; HEX_NEIGHBOUR_POS_NE = 1
                    // etc. Shift by one to go into our return object.
                    switch (hidx) {
                    case 0: // slide to s
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_SW] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_SE] = pw * lr / hexarea;
                        break;
                    }
                    case 1: // slide to n
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_NE] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_NW] = pw * lr / hexarea;
                        break;
                    }
                    case 2: // slide to se
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_W] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_SW] = pw * lr / hexarea;
                        break;
                    }
                    case 3: // slide to ne
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_E] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_NE] = pw * lr / hexarea;
                        break;
                    }
                    case 4: // slide to nw
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_W] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_NW] = pw * lr / hexarea;
                        break;
                    }
                    case 5: // slide to se
                    {
                        rtn[1+HEX_NEIGHBOUR_POS_SE] = pw * lr / hexarea;
                        rtn[1+HEX_NEIGHBOUR_POS_E] = pw * lr / hexarea;
                        break;
                    }
                    default: {
                        std::cout << "Unknown case: " << hidx << " (fixme)\n";
                        break;
                    }
                    }

                } else if (minpp < lr) {
                    // Reduced triangles
                    throw std::runtime_error ("compute_overlap_colinear: writeme for reduced triangles");
                }
            } else {
                throw std::runtime_error ("compute_overlap_colinear: unexpected case.");
            }

            return rtn;
        }

        //! Deals with another kind of "colinear overlap".
        Vector<float, 19> compute_overlap_colinear2 (const unsigned int _rotation)
        {
            if constexpr (debug_hexshift) {std::cout << __FUNCTION__ << " called with rotation " << _rotation << "\n"; }
            Vector<float, 19> rtn;
            rtn.zero();
            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);
            unit_60 = rotn * morph::Vector<float, 2>({ 0.5f, morph::mathconst<float>::root_3_over_2 });
            // Main parallelogram is defined by points p1, q1, q4.
            float ap1 = std::abs((p1-q4).dot(unit_60)) * this->getLR() / this->hexen.begin()->getArea();
            if constexpr (debug_hexshift) {
                std::cout << "Place ap1="<<ap1<<" into [0] and [" << 1+_rotation << "] with remainder = 1-2ap1 = "
                          << (1.0f - 2.0f * ap1) << " going in [" << ((2+_rotation)%6) << "]\n";
            }
            rtn[0] = ap1;
            rtn[1+_rotation] = ap1; // [ 1 2 3 4 5 6 ]
            rtn[(2+_rotation)%6] = 1.0f - 2.0f * ap1; // [ 2 3 4 5 0 1 ]
            return rtn;
        }
        //! And the other 6 permutations of colinear overlap
        Vector<float, 19> compute_overlap_colinear3 (const unsigned int _rotation)
        {
            if constexpr (debug_hexshift) { std::cout << __FUNCTION__ << " called with rotation " << _rotation << "\n"; }
            Vector<float, 19> rtn;
            rtn.zero();
            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);
            unit_120 = rotn * morph::Vector<float, 2>({ -0.5f, morph::mathconst<float>::root_3_over_2});
            // Main parallelogram is defined by points p2, q2 and q3.
            float ap1 = std::abs((p2-q3).dot(unit_120)) * this->getLR() / this->hexen.begin()->getArea();
            if constexpr (debug_hexshift) {
                std::cout << "Place ap1="<<ap1<<" into [0] and [" << 1+_rotation << "] with remainder = 1-2ap1 = "
                          << (1.0f - 2.0f * ap1) << " going in [" << ((2+_rotation)%6) << "]\n";
            }
            rtn[0] = ap1;
            rtn[1+_rotation] = ap1; // [ 1 2 3 4 5 6 ]
            rtn[1+(_rotation+11)%6] = 1.0f - 2.0f * ap1; // [6 1 2 3 4 5]
            return rtn;
        }

        //! Common function to set up p1, p2, etc based on the current rotational
        //! orientation of the shifted hex - that is - is the shifted hex to the
        //! east-ish of the base hex, or is it in one of the other 5 rotationally
        //! symmetric positions? Depending on rotatation, p1-6 and q1-6 are defined in
        //! differing (but rotationally symmetric) locations.
        morph::Matrix22<float> setup_hexoverlap_geometry (const unsigned int _rotation)
        {
            float lr = this->getLR();
            morph::Matrix22<float> rotn;
            // hexvectors from centre to points:
            morph::Vector<float, 2> hv_ne = { morph::mathconst<float>::root_3_over_2 * lr, 0.5f * lr };
            morph::Vector<float, 2> hv_n =  { 0, lr };
            morph::Vector<float, 2> hv_nw = {-morph::mathconst<float>::root_3_over_2 * lr, 0.5f * lr};
            morph::Vector<float, 2> hv_sw = {-morph::mathconst<float>::root_3_over_2 * lr, -0.5f * lr};
            morph::Vector<float, 2> hv_s =  { 0, -lr };
            morph::Vector<float, 2> hv_se = { morph::mathconst<float>::root_3_over_2 * lr, -0.5f * lr};

            switch (_rotation) {
            case 0:
            default:
            {
                // Lines to find i1 intersection:
                p1 = n_loc; q1 = ne_loc; p2 = nw_sft; q2 = n_sft;
                // Lines to find i5 intersection:
                p3 = se_loc; q3 = s_loc; p4 = s_sft; q4 = sw_sft;
                p5 = ne_sft;
                p6 = ne_loc + hv_ne + hv_n;
                q6 = ne_loc + hv_ne; q5 = n_loc + hv_n + hv_ne;
                q7 = se_sft;
                q8 = se_loc + hv_se + hv_ne;
                p8 = ne_loc + hv_se + hv_ne;
                break;
            }
            case 1:
            {
                p1 = nw_loc; q1 = n_loc; p2 = sw_sft; q2 = nw_sft;
                p3 = ne_loc; q3 = se_loc; p4 = se_sft; q4 = s_sft;
                rotn.rotate (morph::mathconst<float>::pi_over_3);
                p5 = n_sft;
                p6 = n_loc + hv_n + hv_nw;
                q6 = n_loc + hv_n; q5 = nw_loc + hv_nw + hv_n;
                q7 = ne_sft;
                q8 = ne_loc + hv_ne + hv_n;
                p8 = n_loc + hv_ne + hv_n;
                break;
            }
            case 2:
            {
                p1 = sw_loc; q1 = nw_loc; p2 = s_sft; q2 = sw_sft;
                p3 = n_loc; q3 = ne_loc; p4 = ne_sft; q4 = se_sft;
                rotn.rotate (morph::mathconst<float>::two_pi_over_3);
                p5 = nw_sft;
                p6 = nw_loc + hv_nw + hv_sw;
                q6 = nw_loc + hv_nw; q5 = sw_loc + hv_sw + hv_nw;
                q7 = n_sft;
                q8 = n_loc + hv_n + hv_nw;
                p8 = nw_loc + hv_n + hv_nw;
                break;
            }
            case 3:
            {
                p1 = s_loc; q1 = sw_loc; p2 = se_sft; q2 = s_sft;
                p3 = nw_loc; q3 = n_loc; p4 = n_sft; q4 = ne_sft;
                rotn.rotate (morph::mathconst<float>::pi);
                p5 = sw_sft;
                p6 = sw_loc + hv_sw + hv_s;
                q6 = sw_loc + hv_sw; q5 = s_loc + hv_s + hv_sw;
                q7 = nw_sft;
                q8 = nw_loc + hv_nw + hv_sw;
                p8 = sw_loc + hv_nw + hv_sw;
                break;
            }
            case 4:
            {
                p1 = se_loc; q1 = s_loc; p2 = ne_sft; q2 = se_sft;
                p3 = sw_loc; q3 = nw_loc; p4 = nw_sft; q4 = n_sft;
                rotn.rotate (morph::mathconst<float>::four_pi_over_3);
                p5 = s_sft;
                p6 = s_loc + hv_s + hv_se;
                q6 = s_loc + hv_s; q5 = se_loc + hv_se + hv_s;
                q7 = sw_sft;
                q8 = sw_loc + hv_sw + hv_s;
                p8 = s_loc + hv_sw + hv_s;
                break;
            }
            case 5:
            {
                p1 = ne_loc; q1 = se_loc; p2 = n_sft; q2 = ne_sft;
                p3 = s_loc; q3 = sw_loc; p4 = sw_sft; q4 = nw_sft;
                rotn.rotate (morph::mathconst<float>::five_pi_over_3);
                p5 = se_sft;
                p6 = se_loc + hv_se + hv_ne;
                q6 = se_loc + hv_se; q5 = ne_loc + hv_ne + hv_se;
                q7 = s_sft;
                q8 = s_loc + hv_s + hv_se;
                p8 = se_loc + hv_s + hv_se;
                break;
            }
            }

            return rotn;
        }

        // Like compute_overlap_far, but with one edge parallel to the east edge of the
        // base hex.
        Vector<float, 19> compute_overlap_far2 (const unsigned int _rotation)
        {
            if constexpr (debug_hexshift) { std::cout << __FUNCTION__ << " called\n"; }

            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);

            // basis vectors
            unit_150 = { -morph::mathconst<float>::root_3_over_2, 0.5f };
            unit_60 = { 0.5f, morph::mathconst<float>::root_3_over_2 };
            Vector<float, 2> uvh = {1.0f, 0.0f};

            // Rotate 'em
            unit_60 = rotn * unit_60;
            unit_150 = rotn * unit_150;
            uvh = rotn * uvh;

            float hex_area = this->hexen.begin()->getArea();
            float lr = this->getLR();

            // Variables to hold the areas of rectangles a1 and a2 and triangles t1 and t2.
            float a1 = 0.0f;
            float t1 = 0.0f;
            float t2 = 0.0f;
            float a2 = 0.0f;

            Vector<float, 19> rtn;
            rtn.zero();

            // Compute relevant intersections. After this, the computation is same as in
            // compute_overlap(). Difference is the elements in the return vector into
            // which the data is placed.
            i1 = this->intersection (p2, q4, q1, q6);
            i5 = this->intersection (p4, q7, p8, q8);

            // Is i5 to right of i1, wrt unit_150?
            bool i5_to_right = true;
            if ((i5-i1).dot(unit_150) > 0.0f) {
                i5_to_right = false;
            } // +ve means "to right". Leave true if not right or left.

            // Top parallelogram defined by i1, p2 and q6
            float ap1 = std::abs((q6-i1).dot(uvh)) * (i1-p2).length() / hex_area;
            rtn[1+(1+_rotation)%6] = ap1; // [2 3 4 5 6 1]

            // Bottom right parallelogram is i5, p8 and q7
            float ap2 = std::abs((q7-i5).dot(uvh)) * (i5-p8).length() / hex_area;
            rtn[(_rotation*2)+8] = ap2;

            // Now the 'two triangles and a rectangle.
            a1_tl = q4;
            a1_bl = p4; // used in visualisation, but not for area computation

            if (i5_to_right == false) {
                // different set of points. i5 is the new i1
                morph::Vector<float, 2> tmp = i1; i1 = i5; i5 = tmp;
                a1_tl = p4;
                a1_bl = q4;
                // Also unit_150/unit_60 have to be swapped
                tmp = unit_60;
                unit_60 = unit_150;
                unit_150 = tmp;
            }

            // Now compute the two triangles+rectangle area
            // The NE one goes in rtn[1+(1+_rotation)%6]

            // Now reason out i2, i3 and i4.
            i2 = i1 - unit_150 * (i1-a1_tl).dot(unit_150);
            i3 = i1 - unit_150 * ((i1-a1_tl).dot(unit_150) + lr);
            // i4 is i1 mirrored about the x axis of the shifted hex, or equivalently:
            i4 = i1 - unit_150 * (2.0f * (i1-a1_tl).dot(unit_150) + lr);
            // i6 for visualization only:
            i6 = i5 + unit_150 * (2.0f * (i1-a1_tl).dot(unit_150) + lr);

            // Rectangle a1 area
            float vside = lr;
            float hside = (i2 - a1_tl).length();
            a1 = vside * hside;

            // Area of top triangle is defined by the points i1, i2 and a1_tl
            vside = (i1-i2).length(); // hside unchanged
            t1 = vside * hside * 0.5f;

            // Area of bottom triangle defined by i3, i4 and (sw_sft), but hside is unchanged
            vside = (i3-i4).length();
            t2 = vside * hside * 0.5f; // always same as t1?

            // Lastly, a2, the middle strip is based on the last intersect, i5
            if (i5.has_nan()) {
                throw std::runtime_error ("No intersection i5, deal with this...");
            } else {
                a2 = (i1-i4).length() * std::abs((i5-i1).dot(unit_60));
            }

            float ov_area_prop = ((a1 + t1 + t2) * 2.0f + a2) / hex_area;

            rtn[1+_rotation] = ov_area_prop;

            // The area remainder goes in rtn[7+(2*_rotation+2)%12] i.e. [9, 11, 13, 15, 17, 7]
            rtn[7+(2*_rotation+2)%12] = 1.0f - ov_area_prop - ap1 - ap2;

            return rtn;
        }

        // A compute overlap for the case that the shifted hex is actually shifted
        // *beyond* the base hex (and thus contributes nothing to rtn[0]). This is an
        // edge case. If the shift goes beyond this, the base hex is shifted by another
        // full hex in the r or g directions. This has one edge parallel to the NE edge
        // of the base hex.
        Vector<float, 19> compute_overlap_far (const unsigned int _rotation)
        {
            if constexpr (debug_hexshift) { std::cout << __FUNCTION__ << " called rotation " << _rotation << "\n"; }

            // We'll want a unit vector in the vertical direction that we can rotate
            Vector<float, 2> uvv = {0.0f, 1.0f};
            Vector<float, 2> uvh = {1.0f, 0.0f};
            // Unit vector in 60 degree direction
            unit_60 = {0.5f, morph::mathconst<float>::root_3_over_2};
            unit_300 = {0.5f, -morph::mathconst<float>::root_3_over_2};

            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);
            float hex_area = this->hexen.begin()->getArea();

            // Rotate our basis vectors
            uvv = rotn * uvv;
            uvh = rotn * uvh;
            unit_60 = rotn * unit_60;
            unit_300 = rotn * unit_300;

            // Variables to hold the areas of rectangles a1 and a2 and triangles t1 and t2.
            float a1 = 0.0f;
            float t1 = 0.0f;
            float t2 = 0.0f;
            float a2 = 0.0f;

            // Compute relevant intersections. After this, the computation is same as in
            // compute_overlap(). Difference is the elements in the return vector into
            // which the data is placed.
            i1 = this->intersection (q5, p6, p2, q2);
            i5 = this->intersection (q1, q6, p4, q4);

            // Is i5 to the +uvh of i1? In zero-rotn frame, "Is i5 to the right of i1?"
            bool i5_to_right = true;
            if ((i5-i1).dot(uvh) < 0.0f) { i5_to_right = false; } // +ve means "to right". Leave true if not right or left.

            // Usually, top left of rectangle a1 is p2
            a1_tl = p2;
            a1_bl = q4; // used in visualisation, but not for area computation
            pll1_top = q2;
            pll1_br = q1;
            pll2_bot = p4;
            pll2_tr = p3;

            // Additional areas. In the default 'move to the right' case, there is
            // overlap with the hex to the NE, the one to the east and the one to the
            // south east. With rotations, these will cycle around.
            // NE. Parallelogram defined by i1 (red), pll1_br (q1) (green), pll1_top (q2) (blue).
            float ap1 = std::abs((pll1_top-i1).dot(unit_60)) * (pll1_br-i1).length() / hex_area;

            // SE. Parallelogram defined by i5 (black), p3 (blue) and p4 (green)
            float ap2 = std::abs((pll2_bot-i5).dot(unit_300)) * (pll2_tr-i5).length() / hex_area;

            if (i5_to_right == false) {
                // different set of points. i5 is the new i1
                morph::Vector<float, 2> tmp = i1; i1 = i5; i5 = tmp;
                // uvv now has to reverse direction
                uvv = -uvv;
                // q4 is the new p2
                a1_tl = q4;
                a1_bl = p2;
                // Also unit_60/unit_300 have to be swapped
                tmp = unit_60;
                unit_60 = unit_300;
                unit_300 = tmp;
            }

            // Now reason out i2, i3 and i4.
            i2 = i1 - uvv * (i1-a1_tl).dot(uvv);
            i3 = i1 - uvv * ((i1-a1_tl).dot(uvv) + this->hexen.begin()->getLR());
            // i4 is i1 mirrored about the x axis of the shifted hex, or equivalently:
            i4 = i1 - uvv * (2.0f * (i1-a1_tl).dot(uvv) + this->hexen.begin()->getLR());
            // i6 for visualization only:
            i6 = i5 + uvv * (2.0f * (i1-a1_tl).dot(uvv) + this->hexen.begin()->getLR());

            // Rectangle a1 area
            float vside = d * morph::mathconst<float>::one_over_root_3;
            float hside = (i2 - a1_tl).length();
            a1 = vside * hside;

            // Area of top triangle is defined by the points i1, i2 and p2
            vside = (i1-i2).length();
            hside = (i2-a1_tl).length();
            t1 = vside * hside * 0.5f;

            // Area of bottom triangle defined by i3, i4 and (sw_sft), but hside is unchanged
            vside = (i3-i4).length();
            t2 = vside * hside * 0.5f; // always same as t1?

            // Lastly, a2, the middle strip is based on the last intersect, i5
            if (i5.has_nan()) {
                //throw std::runtime_error ("No intersection i5, deal with this...");
                std::cout << "No intersection i5?\n";
            } else {
                a2 = (i1-i4).length() * std::abs((i5-i1).dot(uvh));
            }

            float ov_area_prop = ((a1 + t1 + t2) * 2.0f + a2) / hex_area;

            // Store the overlap proportion in the return object
            Vector<float, 19> rtn;
            rtn.zero();
            rtn[2+_rotation] = ov_area_prop;

            // This is NE (2 hexes) for zero rotation, and thus goes in rtn[10] for _rotation==0
            rtn[8+(2*_rotation+2)%12] = ap1;
            // SE. Parallelogram defined by i5 (black), p3 (blue) and p4 (green)
            rtn[1+_rotation] = ap2;
            // E. Area defined by triangle below NE parallelogram, rectangle, and
            // another triangle. Similar computation as the one under the '0th' overlap,
            // but can solve as Hex area - the others.
            rtn[7+(2*_rotation+2)%12] = 1.0f - ov_area_prop - ap1 - ap2;

            return rtn;
        }

        /*!
         * Compute hexagon overlap for an east shift, applying the given rotation
         * increment. _rotation=0 means 0 degrees; _rotation=1 means 60 degrees
         * anticlockwise, and so on.
         */
        Vector<float, 19> compute_overlap (const unsigned int _rotation)
        {
            if constexpr (debug_hexshift) { std::cout << __FUNCTION__ << " called for rotation=" << _rotation << "\n"; }

            // We'll want a unit vector in the vertical direction that we can rotate
            Vector<float, 2> uvv = {0.0f, 1.0f};
            Vector<float, 2> uvh = {1.0f, 0.0f};
            // Unit vector in 60 degree direction
            unit_60 = {0.5f, morph::mathconst<float>::root_3_over_2};
            unit_300 = {0.5f, -morph::mathconst<float>::root_3_over_2};

            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);
            float hex_area = this->hexen.begin()->getArea();

            // Rotate our basis vectors
            uvv = rotn * uvv;
            uvh = rotn * uvh;
            unit_60 = rotn * unit_60;
            unit_300 = rotn * unit_300;

            // Variables to hold the areas of rectangles a1 and a2 and triangles t1 and t2.
            float a1 = 0.0f;
            float t1 = 0.0f;
            float t2 = 0.0f;
            float a2 = 0.0f;

            // Compute the relevant intersections
            i1 = this->intersection (p1, q1, p2, q2);
            i5 = this->intersection (p3, q3, p4, q4);

            // Is i5 to the +uvh of i1? In zero-rotn frame, "Is i5 to the right of i1?"
            bool i5_to_right = true;
            if ((i5-i1).dot(uvh) < 0.0f) { i5_to_right = false; } // +ve means "to right". Leave true if not right or left.

            // Usually, top left of rectangle a1 is p2
            a1_tl = p2;
            a1_bl = q4; // used in visualisation, but not for area computation
            pll1_top = q2;
            pll1_br = q1;
            pll2_bot = p4;
            pll2_tr = p3;

            // Does this go before or after the swapping stuff bit??
            // NE. Parallelogram defined by i1 (red), pll1_br (q1) (green), pll1_top (q2) (blue).
            float ap1 = std::abs((pll1_top-i1).dot(unit_60)) * (pll1_br-i1).length() / hex_area;
            if constexpr (debug_hexshift) { std::cout << "'NW' parallelogram ap1: " << ap1 << std::endl; }

            // SE. Parallelogram defined by i5 (black), p3 (blue) and p4 (green)
            float ap2 = std::abs((pll2_bot-i5).dot(unit_300)) * (pll2_tr-i5).length() / hex_area;
            std::cout << "'SE' parallelogram ap2: " << ap2 << std::endl;

            if (i5_to_right == false) {
                std::cout << "to right is false, swapping stuff...\n";
                // different set of points. i5 is the new i1
                morph::Vector<float, 2> tmp = i1; i1 = i5; i5 = tmp;
                // uvv/uvh now have to reverse direction
                uvv = -uvv;
                // q4 is the new p2
                a1_tl = q4;
                a1_bl = p2;
                // Also unit_60/unit_300 have to be swapped
                tmp = unit_60;
                unit_60 = unit_300;
                unit_300 = tmp;
            }

            float lr = this->getLR();
            // Now reason out i2, i3 and i4.
            i2 = i1 - uvv * (i1-a1_tl).dot(uvv);
            i3 = i1 - uvv * ((i1-a1_tl).dot(uvv) + lr);
            // i4 is i1 mirrored about the x axis of the shifted hex, or equivalently:
            i4 = i1 - uvv * (2.0f * (i1-a1_tl).dot(uvv) + lr);
            // i6 for visualization only:
            i6 = i5 + uvv * (2.0f * (i1-a1_tl).dot(uvv) + lr);

            // Rectangle a1 area
            float vside = lr;
            float hside = (i2-a1_tl).length();
            VAR(hside);
            a1 = vside * hside;
            VAR (a1/hex_area);
            // Area of top triangle is defined by the points i1, i2 and p2
            vside = (i1-i2).length();
            t1 = vside * hside * 0.5f;
            VAR (t1/hex_area);

            // Area of bottom triangle defined by i3, i4 and (sw_sft), but hside is unchanged
            vside = (i3-i4).length();
            t2 = vside * hside * 0.5f; // always same as t1?
            VAR (t2/hex_area);

            // Lastly, a2, the middle strip is based on the last intersect, i5
            if (i5.has_nan()) {
                //throw std::runtime_error ("No intersection i5, deal with this...");
                std::cout << "No intersection i5?\n";
            } else {
                a2 = (i1-i4).length() * std::abs((i5-i1).dot(uvh));
                VAR (a2/hex_area);
            }

            float ov_area_prop = ((a1 + t1 + t2) * 2.0f + a2) / hex_area;
            std::cout << "Triangles and rectangles: " << ov_area_prop << "\n";

            // Store the overlap proportion in the return object
            Vector<float, 19> rtn;
            rtn.zero();
            rtn[0] = ov_area_prop;

            // Additional areas. In the default 'move to the right' case, there is
            // overlap with the hex to the NE, the one to the east and the one to the
            // south east. With rotations, these will cycle around.

            // This is NE for zero rotation, and thus goes in rtn[2] for _rotation==0
            std::cout << "ap1 set into rtn["<< (1+(1+_rotation)%6) << "]\n";
            rtn[1+(1+_rotation)%6] = ap1;
            // SE. Parallelogram defined by i5 (black), p3 (blue) and p4 (green)
            std::cout << "ap2 set into rtn["<< (1+(5+_rotation)%6) << "]\n";
            rtn[1+(5+_rotation)%6] = ap2;
            // E. Area defined by triangle below NE parallelogram, rectangle, and
            // another triangle. Similar computation as the one under the '0th' overlap,
            // but can solve as Hex area - the others.
            std::cout << "Setting remainder quadrilateral (NE) into rtn[" << 1+_rotation << "]\n";
            rtn[1+_rotation] = 1.0f - ov_area_prop - ap1 - ap2;

            return rtn;
        }

        // Similar to compute_overlap(), but slightly permuted. This is called when the
        // shifted hex overlaps on by 'one corner'.
        Vector<float, 19> compute_overlap_corner (const unsigned int _rotation)
        {
            std::cout << __FUNCTION__ << " called rotation: " << _rotation << "\n";

            Vector<float, 19> rtn;
            rtn.zero();

            // Unit vectors
            Vector<float, 2> uvv = {0.0f, 1.0f};
            Vector<float, 2> unit_240 = {-0.5f, -morph::mathconst<float>::root_3_over_2};
            Vector<float, 2> unit_120 = { -0.5f, morph::mathconst<float>::root_3_over_2};
            Vector<float, 2> unit_30 = { morph::mathconst<float>::root_3_over_2, 0.5f };
            Vector<float, 2> unit_210 = { -morph::mathconst<float>::root_3_over_2, -0.5f };

            // Hex long radius/edge length
            float lr = this->getLR();

            morph::Matrix22<float> rotn = this->setup_hexoverlap_geometry (_rotation);

            // Rotate our basis vector(s)
            uvv = rotn * uvv;
            unit_240 = rotn * unit_240;
            unit_120 = rotn * unit_120;
            unit_30 = rotn * unit_30;
            unit_210 = rotn * unit_210;

            // Variables to hold the areas of rectangles a1 and a2 and triangles t1 and t2.
            float a1 = 0.0f;
            float t1 = 0.0f;
            float t2 = 0.0f;
            float a2 = 0.0f;

            float hex_area = this->hexen.begin()->getArea();

            // Compute relevant intersections
            i5 = this->intersection (p1, q1, p2, q4);
            i1 = this->intersection (p5, q2, p6, q6);

            // Is i5 to the 'left' of i1? (wrt to the 120 degree line?
            bool i5_to_left = true;
            if ((i5-i1).dot(unit_120) < 0.0f) {
                i5_to_left = false;
            }

            // Near Parallelogram defined by i5, q4 and q1.
            float ap1 = std::abs((q4-i5).dot(unit_240)) * (i5-q1).length() / hex_area;
            rtn[0] = ap1;

            // Far parallelogram defined by i1, p5 and q6.
            float ap2 = std::abs((q6-i1).dot(unit_240)) * (i1-p5).length() / hex_area;
            rtn[7+(2*_rotation+2)%12] = ap2;

            // Usually, top left of rectangle a1 is q2
            a1_tl = q2;
            a1_bl = p2; // used in visualisation, but not for area computation

            if (i5_to_left == true) {
                // different set of points. i5 is the new i1
                morph::Vector<float, 2> tmp = i1; i1 = i5; i5 = tmp;
                // uvv/uvh now have to reverse direction
                // uvh = -uvh;  // uvv not reversed
                // q4 is the new p2
                a1_tl = p2;
                a1_bl = q2;
                // Also unit_30/unit_210 have to be swapped
                tmp = unit_30;
                unit_30 = unit_210;
                unit_210 = tmp;
            }

            // Now compute the two triangles+rectangle area
            // The NE one goes in rtn[1+(1+_rotation)%6]

            // Now reason out i2, i3 and i4.
            i2 = i1 - unit_30 * (i1-a1_tl).dot(unit_30);
            i3 = i1 - unit_30 * ((i1-a1_tl).dot(unit_30) + lr);
            // i4 is i1 mirrored about the x axis of the shifted hex, or equivalently:
            i4 = i1 - unit_30 * (2.0f * (i1-a1_tl).dot(unit_30) + lr);
            // i6 for visualization only:
            i6 = i5 + unit_30 * (2.0f * (i1-a1_tl).dot(unit_30) + lr);

            // Rectangle a1 area
            float vside = lr;
            float hside = (i2 - a1_tl).length();
            a1 = vside * hside;

            // Area of top triangle is defined by the points i1, i2 and p2
            vside = (i1-i2).length(); // hside unchanged
            t1 = vside * hside * 0.5f;

            // Area of bottom triangle defined by i3, i4 and (sw_sft), but hside is unchanged
            vside = (i3-i4).length();
            t2 = vside * hside * 0.5f; // always same as t1?

            // Lastly, a2, the middle strip is based on the last intersect, i5
            if (i5.has_nan()) {
                //throw std::runtime_error ("No intersection i5, deal with this...");
                std::cout << "No intersection i5?\n";
            } else {
                a2 = (i1-i4).length() * std::abs((i5-i1).dot(unit_120));
            }

            float ov_area_prop = ((a1 + t1 + t2) * 2.0f + a2) / hex_area;
            rtn[1+(1+_rotation)%6] = ov_area_prop; // [2 3 4 5 6 1]

            // The area remainder goes in [1 2 3 4 5 6]
            float rem_prop = 1.0f - ov_area_prop - ap1 - ap2;
            rtn[1+_rotation] = rem_prop;

            return rtn;
        }

        // Set up wrapping. This works only on parallelogram shaped domains.
        void setParallelogramWrap (bool onR, bool onG)
        {
            if (!(onR && onG)) {
                throw std::runtime_error ("Test single axis wrapping then remove this exception.");
            }

            // Find furthest SW hex
            bool first = true;
            std::array<float, 4> limits = {{0,0,0,0}};
            auto h = this->hexen.begin();
            std::list<Hex>::iterator bl_hex = this->hexen.begin();
            while (h != this->hexen.end()) {
                if (h->testFlags(HEX_IS_BOUNDARY) == true) {
                    if (first) {
                        limits = {{h->x, h->x, h->y, h->y}};
                        first = false;
                    }
                    if (h->x < limits[0] && h->y <= limits[2]) {
                        limits[0] = h->x; limits[2] = h->y;
                        bl_hex = h;
                    }
                }
                ++h;
            }
            // Find hex nearest limits. Really?
            //std::cout << "Bottom left hex is " << bl_hex->outputCart() << std::endl;

            int count = 0;
            std::list<Hex>::iterator row_start = bl_hex;
            if (onR) {
                // go to end of each row and wrap back to the start. This may only work
                // for parallelograms, at least in an initial implementation.
                // First row
                std::list<Hex>::iterator cur_hex = row_start;
                while (cur_hex->has_ne()) { cur_hex = cur_hex->ne; }
                cur_hex->set_ne(bl_hex);
                bl_hex->set_nw(cur_hex);
                //std::cout << "set E hex of " << cur_hex->outputCart() << " to " << bl_hex->outputCart() << std::endl;
                // Rest of the rows
                while (row_start->has_nne()) {
                    row_start = row_start->nne;
                    cur_hex = row_start;
                    count = 0;
                    while (cur_hex->has_ne()) {
                        cur_hex = cur_hex->ne;
                        ++count;
                    }
                    //std::cout << "set E hex of " << cur_hex->outputCart() << " to " << row_start->outputCart() << std::endl;

                    cur_hex->set_ne (row_start);
                    row_start->set_nw (cur_hex);
                }
            }

            std::list<Hex>::iterator col_start = bl_hex;
            int vcount = 0;
            if (onG) { // scan up columns in the 'G' direction
                // First col
                std::list<Hex>::iterator cur_hex = col_start;
                while (cur_hex->has_nne()) { cur_hex = cur_hex->nne; ++vcount; }
                cur_hex->set_nne (bl_hex);
                bl_hex->set_nsw (cur_hex);
                //std::cout << "Firstcol. set NE hex of " << cur_hex->outputRG() << " to " << bl_hex->outputRG() << std::endl;
                //std::cout << "Firstcol. set SW hex of " << bl_hex->outputRG() << " to " << cur_hex->outputRG() << std::endl;

                cur_hex->set_nnw(bl_hex->nw);
                bl_hex->nw->set_nse(cur_hex->ne);
                //std::cout << "Firstcol. set NW hex of " << cur_hex->outputRG() << " to " << bl_hex->nw->outputRG() << std::endl;
                //std::cout << "Firstcol. set SE hex of " << bl_hex->nw->outputRG() << " to " << cur_hex->ne->outputRG() << std::endl;

                // Rest of the rows
                for (int i = 0; i < count; ++i) { // NB: Assumes every row the same length
                    col_start = col_start->ne;
                    cur_hex = col_start;
                    while (cur_hex->has_nne()) { cur_hex = cur_hex->nne; }

                    cur_hex->set_nne(col_start);
                    col_start->set_nsw(cur_hex);
                    //std::cout << "set NE hex of " << cur_hex->outputRG() << " to " << col_start->outputRG() << std::endl;
                    //std::cout << "set SW hex of " << col_start->outputRG() << " to " << cur_hex->outputRG() << std::endl;

                    // Also set the nnw of the current hex to be the nse of the start of the prev col
                    cur_hex->set_nnw(col_start->nw);
                    col_start->nw->set_nse(cur_hex);
                    //std::cout << "set NW hex of " << cur_hex->outputRG() << " to " << col_start->nw->outputRG() << std::endl;
                    //std::cout << "set SE hex of " << col_start->nw->outputRG() << " to " << cur_hex->outputRG() << std::endl;
                }
            }

            // Final scan across to set se neighbours of end rows and nw neighbours of start rows.
            row_start = bl_hex;
            if (onR && onG) {
                std::list<Hex>::iterator cur_hex = row_start;
                // First row
                for (int i = 0; i < count; ++i) { cur_hex = cur_hex->ne; }
                row_start->set_nnw(cur_hex->nne);
                cur_hex->set_nse(row_start->nsw);
                // Rest of the rows
                for (int j = 0; j < vcount; ++j) {
                    row_start = row_start->nne;
                    cur_hex = row_start;
                    for (int i = 0; i < count; ++i) { cur_hex = cur_hex->ne; }
                    row_start->set_nnw(cur_hex->nne);
                    cur_hex->set_nse(row_start->nsw);
                }
            }
        }

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
        std::list<const Hex*> bhexen; // Not better as a separate list<Hex>?

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
        void init()
        {
            // Use span_x to determine how many rings out to traverse.
            float halfX = this->x_span/2.0f;
            unsigned int maxRing = std::abs(std::ceil(halfX/this->d));

            DBG ("Creating hexagonal hex grid with maxRing: " << maxRing);

            // The "vector iterator" - this is an identity iterator that is added to each Hex in the grid.
            unsigned int vi = 0;

            // Vectors of list-iterators to hexes in this->hexen. Used to keep a track of nearest
            // neighbours. I'm using vector, rather than a list as this allows fast random access of
            // elements and I'll not be inserting or erasing in the middle of the arrays.
            std::vector<std::list<morph::Hex>::iterator> prevRingEven;
            std::vector<std::list<morph::Hex>::iterator> prevRingOdd;

            // Swap pointers between rings.
            std::vector<std::list<morph::Hex>::iterator>* prevRing = &prevRingEven;
            std::vector<std::list<morph::Hex>::iterator>* nextPrevRing = &prevRingOdd;

            // Direction iterators used in the loop for creating hexes
            int ri = 0;
            int gi = 0;

            // Create central "ring" first (the single hex)
            this->hexen.emplace_back (vi++, this->d, ri, gi);

            // Put central ring in the prevRing vector:
            {
                std::list<morph::Hex>::iterator h = this->hexen.end(); --h;
                prevRing->push_back (h);
            }

            // Now build up the rings around it, setting neighbours as we go. Each ring has 6 more hexes
            // than the previous one (except for ring 1, which has 6 instead of 1 in the centre).
            unsigned int numInRing = 6;

            // How many hops in the same direction before turning a corner?  Increases for each
            // ring. Increases by 1 in each ring.
            unsigned int ringSideLen = 1;

            // These are used to iterate along the six sides of the hexagonal ring that's inside, but
            // adjacent to the hexagonal ring that's under construction.
            int walkstart = 0;
            int walkinc = 0;
            int walkmin = walkstart-1;
            int walkmax = 1;

            for (unsigned int ring = 1; ring <= maxRing; ++ring) {

                // Set start ri, gi. This moves up a hex and left a hex onto the start hex of the new ring.
                --ri; ++gi;

                nextPrevRing->clear();

                // Now walk around the ring, in 6 walks, that will bring us round to just before we
                // started. walkstart has the starting iterator number for the vertices of the hexagon.

                // Walk in the r direction first:
                for (unsigned int i = 0; i<ringSideLen; ++i) {

                    this->hexen.emplace_back (vi++, this->d, ri++, gi);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexNW = hi; }

                    // 1. Set my W neighbour to be the previous hex in THIS ring, if possible
                    if (i > 0) {
                        hi->set_nw (lasthi);
                        // Set me (hi) as E neighbour to previous hex in the ring (lasthi):
                        lasthi->set_ne (hi);
                    }
                    // else i must be 0 in this case, we would set the SW neighbour now,
                    // but as this won't have been added to the ring, we have to leave it

                    // 2. SW neighbour
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my SW neighbour:
                        hi->set_nsw ((*prevRing)[j]);
                        // Set me as NE neighbour to those in prevRing:
                        (*prevRing)[j]->set_nne (hi);
                    }
                    ++j;

                    // 3. Set my SE neighbour:
                    if (j<=walkmax) {
                        hi->set_nse ((*prevRing)[j]);
                        // Set me as NW neighbour:
                        (*prevRing)[j]->set_nnw (hi);
                    }

                    // Put in me nextPrevRing:
                    nextPrevRing->push_back (hi);
                }
                walkstart += walkinc;
                walkmin   += walkinc;
                walkmax   += walkinc;

                // Walk in -b direction
                for (unsigned int i = 0; i<ringSideLen; ++i) {
                    this->hexen.emplace_back (vi++, this->d, ri++, gi--);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexNE = hi; }

                    // 1. Set my NW neighbour to be the previous hex in THIS ring, if possible
                    if (i > 0) {
                        hi->set_nnw (lasthi);
                        // Set me as SE neighbour to previous hex in the ring:
                        lasthi->set_nse (hi);
                    } else {
                        // Set my W neighbour for the first hex in the row.
                        hi->set_nw (lasthi);
                        // Set me as E neighbour to previous hex in the ring:
                        lasthi->set_ne (hi);
                    }

                    // 2. W neighbour
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my W neighbour:
                        hi->set_nw ((*prevRing)[j]);
                        // Set me as E neighbour to those in prevRing:
                        (*prevRing)[j]->set_ne (hi);
                    }
                    ++j;

                    // 3. Set my SW neighbour:
                    if (j<=walkmax) {
                        hi->set_nsw ((*prevRing)[j]);
                        // Set me as NE neighbour:
                        (*prevRing)[j]->set_nne (hi);
                    }

                    nextPrevRing->push_back (hi);
                }
                walkstart += walkinc;
                walkmin += walkinc;
                walkmax += walkinc;

                // Walk in -g direction
                for (unsigned int i = 0; i<ringSideLen; ++i) {

                    this->hexen.emplace_back (vi++, this->d, ri, gi--);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexE = hi; }

                    // 1. Set my NE neighbour to be the previous hex in THIS ring, if possible
                    if (i > 0) {
                        hi->set_nne (lasthi);
                        // Set me as SW neighbour to previous hex in the ring:
                        lasthi->set_nsw (hi);
                    } else {
                        // Set my NW neighbour for the first hex in the row.
                        hi->set_nnw (lasthi);
                        // Set me as SE neighbour to previous hex in the ring:
                        lasthi->set_nse (hi);
                    }

                    // 2. NW neighbour
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my NW neighbour:
                        hi->set_nnw ((*prevRing)[j]);
                        // Set me as SE neighbour to those in prevRing:
                        (*prevRing)[j]->set_nse (hi);
                    }
                    ++j;

                    // 3. Set my W neighbour:
                    if (j<=walkmax) {
                        hi->set_nw ((*prevRing)[j]);
                        // Set me as E neighbour:
                        (*prevRing)[j]->set_ne (hi);
                    }

                    // Put in me nextPrevRing:
                    nextPrevRing->push_back (hi);
                }
                walkstart += walkinc;
                walkmin += walkinc;
                walkmax += walkinc;

                // Walk in -r direction
                for (unsigned int i = 0; i<ringSideLen; ++i) {

                    this->hexen.emplace_back (vi++, this->d, ri--, gi);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexSE = hi; }

                    // 1. Set my E neighbour to be the previous hex in THIS ring, if possible
                    if (i > 0) {
                        hi->set_ne (lasthi);
                        // Set me as W neighbour to previous hex in the ring:
                        lasthi->set_nw (hi);
                    } else {
                        // Set my NE neighbour for the first hex in the row.
                        hi->set_nne (lasthi);
                        // Set me as SW neighbour to previous hex in the ring:
                        lasthi->set_nsw (hi);
                    }

                    // 2. NE neighbour:
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my NE neighbour:
                        hi->set_nne ((*prevRing)[j]);
                        // Set me as SW neighbour to those in prevRing:
                        (*prevRing)[j]->set_nsw (hi);
                    }
                    ++j;

                    // 3. Set my NW neighbour:
                    if (j<=walkmax) {
                        hi->set_nnw ((*prevRing)[j]);
                        // Set me as SE neighbour:
                        (*prevRing)[j]->set_nse (hi);
                    }

                    nextPrevRing->push_back (hi);
                }
                walkstart += walkinc;
                walkmin += walkinc;
                walkmax += walkinc;

                // Walk in b direction
                for (unsigned int i = 0; i<ringSideLen; ++i) {
                    this->hexen.emplace_back (vi++, this->d, ri--, gi++);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexSW = hi; }

                    // 1. Set my SE neighbour to be the previous hex in THIS ring, if possible
                    if (i > 0) {
                        hi->set_nse (lasthi);
                        // Set me as NW neighbour to previous hex in the ring:
                        lasthi->set_nnw (hi);
                    } else { // i == 0
                        // Set my E neighbour for the first hex in the row.
                        hi->set_ne (lasthi);
                        // Set me as W neighbour to previous hex in the ring:
                        lasthi->set_nw (hi);
                    }

                    // 2. E neighbour:
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my E neighbour:
                        hi->set_ne ((*prevRing)[j]);
                        // Set me as W neighbour to those in prevRing:
                        (*prevRing)[j]->set_nw (hi);
                    }
                    ++j;

                    // 3. Set my NE neighbour:
                    if (j<=walkmax) {
                        hi->set_nne ((*prevRing)[j]);
                        // Set me as SW neighbour:
                        (*prevRing)[j]->set_nsw (hi);
                    }

                    nextPrevRing->push_back (hi);
                }
                walkstart += walkinc;
                walkmin += walkinc;
                walkmax += walkinc;

                // Walk in g direction up to almost the last hex
                for (unsigned int i = 0; i<ringSideLen; ++i) {

                    this->hexen.emplace_back (vi++, this->d, ri, gi++);
                    auto hi = this->hexen.end(); hi--;
                    auto lasthi = hi;
                    --lasthi;

                    // Set vertex
                    if (i==0) { vertexW = hi; }

                    // 1. Set my SW neighbour to be the previous hex in THIS ring, if possible
                    if (i == (ringSideLen-1)) {
                        // Special case at end; on last g walk hex, set the NE neighbour Set my NE neighbour
                        // for the first hex in the row.
                        hi->set_nne ((*nextPrevRing)[0]); // (*nextPrevRing)[0] is an iterator to the first hex
                        // Set me as NW neighbour to previous hex in the ring:
                        (*nextPrevRing)[0]->set_nsw (hi);
                    }
                    if (i > 0) {
                        hi->set_nsw (lasthi);
                        // Set me as NE neighbour to previous hex in the ring:
                        lasthi->set_nne (hi);
                    } else {
                        // Set my SE neighbour for the first hex in the row.
                        hi->set_nse (lasthi);
                        // Set me as NW neighbour to previous hex in the ring:
                        lasthi->set_nnw (hi);
                    }

                    // 2. E neighbour:
                    int j = walkstart + (int)i - 1;
                    if (j>walkmin && j<walkmax) {
                        // Set my SE neighbour:
                        hi->set_nse ((*prevRing)[j]);
                        // Set me as NW neighbour to those in prevRing:
                        (*prevRing)[j]->set_nnw (hi);
                    }
                    ++j;

                    // 3. Set my E neighbour:
                    if (j==walkmax) { // We're on the last square and need to set the East neighbour of the
                        // first hex in the last ring.
                        hi->set_ne ((*prevRing)[0]);
                        // Set me as W neighbour:
                        (*prevRing)[0]->set_nw (hi);

                    } else if (j<walkmax) {
                        hi->set_ne ((*prevRing)[j]);
                        // Set me as W neighbour:
                        (*prevRing)[j]->set_nw (hi);
                    }

                    // Put in me nextPrevRing:
                    nextPrevRing->push_back (hi);
                }
                // Should now be on the last hex.

                // Update the walking increments for finding the vertices of the hexagonal ring. These are
                // for walking around the ring *inside* the ring of hexes being created and hence note that
                // I set walkinc to numInRing/6 BEFORE incrementing numInRing by 6, below.
                walkstart = 0;
                walkinc = numInRing / 6;
                walkmin = walkstart - 1;
                walkmax = walkmin + 1 + walkinc;

                // Always 6 additional hexes in the next ring out
                numInRing += 6;

                // And ring side length goes up by 1
                ringSideLen++;

                // Swap prevRing and nextPrevRing.
                std::vector<std::list<morph::Hex>::iterator>* tmp = prevRing;
                prevRing = nextPrevRing;
                nextPrevRing = tmp;
            }
            DBG ("Finished creating " << this->hexen.size() << " hexes in " << maxRing << " rings.");
        }

        /*!
         * Starting from \a startFrom, and following nearest-neighbour relations, find
         * the closest Hex in hexen to the coordinate point \a point, and set its
         * Hex::onBoundary attribute to true.
         *
         * \return An iterator into HexGrid::hexen which refers to the closest Hex to \a point.
         */
        std::list<morph::Hex>::iterator setBoundary (const morph::BezCoord<float>& point,
                                                     std::list<morph::Hex>::iterator startFrom)
        {
            std::list<morph::Hex>::iterator h = this->findHexNearPoint (point, startFrom);
            h->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
            return h;
        }

        /*!
         * Determine whether the boundary is contiguous. Whilst doing so, populate a
         * list<Hex> containing just the boundary Hexes.
         */
        bool boundaryContiguous()
        {
            this->bhexen.clear();
            std::list<morph::Hex>::const_iterator bhi = this->hexen.begin();
            if (this->findBoundaryHex (bhi) == false) {
                // Found no boundary hex
                return false;
            }
            std::set<unsigned int> seen;
            std::list<morph::Hex>::const_iterator hi = bhi;
            return this->boundaryContiguous (bhi, hi, seen);
        }

        /*!
         * Determine whether the boundary is contiguous, starting from the boundary
         * Hex iterator \a bhi.
         *
         * The overload with bhexes takes a list of Hex pointers and populates it with
         * pointers to the hexes on the boundary.
         */
        bool boundaryContiguous (std::list<Hex>::const_iterator bhi,
                                 std::list<Hex>::const_iterator hi, std::set<unsigned int>& seen)
        {
            bool rtn = false;
            std::list<morph::Hex>::const_iterator hi_next;
            seen.insert (hi->vi);
            // Insert into the std::list of Hex pointers, too
            this->bhexen.push_back (&(*hi));

            if (rtn == false && hi->has_ne() && hi->ne->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->ne->vi) == seen.end()) {
                hi_next = hi->ne;
                rtn = (this->boundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nne() && hi->nne->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nne->vi) == seen.end()) {
                hi_next = hi->nne;
                rtn = (this->boundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nnw() && hi->nnw->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nnw->vi) == seen.end()) {
                hi_next = hi->nnw;
                rtn =  (this->boundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nw() && hi->nw->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nw->vi) == seen.end()) {
                hi_next = hi->nw;
                rtn =  (this->boundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nsw() && hi->nsw->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nsw->vi) == seen.end()) {
                hi_next = hi->nsw;
                rtn =  (this->boundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nse() && hi->nse->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nse->vi) == seen.end()) {
                hi_next = hi->nse;
                rtn =  (this->boundaryContiguous (bhi, hi_next, seen));
            }

            if (rtn == false) {
                // Checked all neighbours
                if (hi == bhi) {
                    // Back at start, nowhere left to go! return true.
                    rtn = true;
                }
            }

            return rtn;
        }

        /*!
         * Set the hex closest to point as being on the region boundary. Region
         * boundaries are supposed to be temporary, so that client code can find a
         * region, extract the pointers to all the Hexes in that region and store that
         * information for later use.
         */
        std::list<Hex>::iterator setRegionBoundary (const BezCoord<float>& point, std::list<Hex>::iterator startFrom)
        {
            std::list<morph::Hex>::iterator h = this->findHexNearPoint (point, startFrom);
            h->setFlag (HEX_IS_REGION_BOUNDARY | HEX_INSIDE_REGION);
            return h;
        }

        /*!
         * Determine whether the region boundary is contiguous, starting from the
         * boundary Hex iterator #bhi.
         */
        bool regionBoundaryContiguous (std::list<Hex>::const_iterator bhi,
                                       std::list<Hex>::const_iterator hi, std::set<unsigned int>& seen)
        {
            bool rtn = false;
            std::list<morph::Hex>::const_iterator hi_next;
            seen.insert (hi->vi);
            // Insert into the list of Hex pointers, too
            this->bhexen.push_back (&(*hi));

            if (rtn == false && hi->has_ne() && hi->ne->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->ne->vi) == seen.end()) {
                hi_next = hi->ne;
                rtn = (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nne() && hi->nne->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->nne->vi) == seen.end()) {
                hi_next = hi->nne;
                rtn = this->regionBoundaryContiguous (bhi, hi_next, seen);
            }
            if (rtn == false && hi->has_nnw() && hi->nnw->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->nnw->vi) == seen.end()) {
                hi_next = hi->nnw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nw() && hi->nw->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->nw->vi) == seen.end()) {
                hi_next = hi->nw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nsw() && hi->nsw->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->nsw->vi) == seen.end()) {
                hi_next = hi->nsw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nse() && hi->nse->testFlags(HEX_IS_REGION_BOUNDARY) == true && seen.find(hi->nse->vi) == seen.end()) {
                hi_next = hi->nse;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }

            if (rtn == false) {
                // Checked all neighbours
                if (hi == bhi) { rtn = true; }
            }

            return rtn;
        }

        /*!
         * Find a hex, any hex, that's on the boundary specified by #boundary. This
         * assumes that setBoundary (const BezCurvePath&) has been called to mark the
         * Hexes that lie on the boundary.
         */
        bool findBoundaryHex (std::list<Hex>::const_iterator& hi) const
        {
            if (hi->testFlags(HEX_IS_BOUNDARY) == true) {
                // No need to change the Hex iterator
                return true;
            }

            if (hi->has_ne()) {
                std::list<morph::Hex>::const_iterator ci(hi->ne);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }
            if (hi->has_nne()) {
                std::list<morph::Hex>::const_iterator ci(hi->nne);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }
            if (hi->has_nnw()) {
                std::list<morph::Hex>::const_iterator ci(hi->nnw);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }
            if (hi->has_nw()) {
                std::list<morph::Hex>::const_iterator ci(hi->nw);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }
            if (hi->has_nsw()) {
                std::list<morph::Hex>::const_iterator ci(hi->nsw);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }
            if (hi->has_nse()) {
                std::list<morph::Hex>::const_iterator ci(hi->nse);
                if (this->findBoundaryHex (ci) == true) {
                    hi = ci;
                    return true;
                }
            }

            return false;
        }

        /*!
         * Find the hex near @point, starting from startFrom, which should be as close
         * as possible to point in order to reduce computation time.
         */
        std::list<Hex>::iterator findHexNearPoint (const BezCoord<float>& point, std::list<Hex>::iterator startFrom)
        {
            bool neighbourNearer = true;

            std::list<morph::Hex>::iterator h = startFrom;
            float dmin = h->distanceFrom (point);
            float dcur = 0.0f;

            while (neighbourNearer == true) {

                neighbourNearer = false;
                if (h->has_ne() && (dcur = h->ne->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->ne;
                    neighbourNearer = true;

                } else if (h->has_nne() && (dcur = h->nne->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->nne;
                    neighbourNearer = true;

                } else if (h->has_nnw() && (dcur = h->nnw->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->nnw;
                    neighbourNearer = true;

                } else if (h->has_nw() && (dcur = h->nw->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->nw;
                    neighbourNearer = true;

                } else if (h->has_nsw() && (dcur = h->nsw->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->nsw;
                    neighbourNearer = true;

                } else if (h->has_nse() && (dcur = h->nse->distanceFrom (point)) < dmin) {
                    dmin = dcur;
                    h = h->nse;
                    neighbourNearer = true;
                }
            }

            return h;
        }

        /*!
         * Mark hexes as being inside the boundary given that \a hi refers to a boundary
         * Hex and at least one adjacent hex to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * hex is the inside)
         *
         * \param hi list iterator to starting Hex.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (std::list<Hex>::iterator hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY)
        {
            this->markFromBoundary (&(*hi), bdryFlag, insideFlag);
        }

        /*!
         * Mark hexes as being inside the boundary given that \a hi refers to a boundary
         * Hex and at least one adjacent hex to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * hex is the inside)
         *
         * \param hi list iterator to a pointer to the starting Hex.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (std::list<Hex*>::iterator hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY)
        {
            this->markFromBoundary ((*hi), bdryFlag, insideFlag);
        }

        /*!
         * Mark hexes as being inside the boundary given that \a hi refers to a boundary
         * Hex and at least one adjacent hex to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * hex is the inside)
         *
         * \param hi pointer to the starting Hex.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (morph::Hex* hi,
                               unsigned int bdryFlag = HEX_IS_BOUNDARY,
                               unsigned int insideFlag = HEX_INSIDE_BOUNDARY)
        {
            // Find a marked-inside Hex next to this boundary hex. This will be the first direction to mark
            // a line of inside hexes in.
            std::list<morph::Hex>::iterator first_inside = this->hexen.begin();
            unsigned short firsti = 0;
            for (unsigned short i = 0; i < 6; ++i) {
                if (hi->has_neighbour(i)
                    && hi->get_neighbour(i)->testFlags(insideFlag) == true
                    && hi->get_neighbour(i)->testFlags(bdryFlag) == false
                    ) {
                    first_inside = hi->get_neighbour(i);
                    firsti = i;
                    break;
                }
            }

            // Mark a line in the first direction
            this->markFromBoundaryCommon (first_inside, firsti, bdryFlag, insideFlag);

            // For each other direction also mark lines. Count direction upwards until we hit a boundary hex:
            short diri = (firsti + 1) % 6;
            // Can debug first *count up* direction with morph::Hex::neighbour_pos(diri)
            while (hi->has_neighbour(diri) && hi->get_neighbour(diri)->testFlags(bdryFlag)==false && diri != firsti) {
                first_inside = hi->get_neighbour(diri);
                this->markFromBoundaryCommon (first_inside, diri, bdryFlag, insideFlag);
                diri = (diri + 1) % 6;
            }

            // Then count downwards until we hit the other boundary hex
            diri = (firsti - 1);
            if (diri < 0) { diri = 5; }
            while (hi->has_neighbour(diri) && hi->get_neighbour(diri)->testFlags(bdryFlag)==false && diri != firsti) {
                first_inside = hi->get_neighbour(diri);
                this->markFromBoundaryCommon (first_inside, diri, bdryFlag, insideFlag);
                diri = (diri - 1);
                if (diri < 0) { diri = 5; }
            }
        }

        /*!
         * Common code used by markFromBoundary()
         */
        void markFromBoundaryCommon (std::list<Hex>::iterator first_inside, unsigned short firsti,
                                     unsigned int bdryFlag = HEX_IS_BOUNDARY,
                                     unsigned int insideFlag = HEX_INSIDE_BOUNDARY)
        {
            // From the "first inside the boundary hex" head in the direction specified by firsti until a
            // boundary hex is reached.
            std::list<morph::Hex>::iterator straight = first_inside;

#ifdef DO_WARNINGS
            bool warning_given = false;
#endif
            while (straight->testFlags(bdryFlag) == false) {
                // Set insideBoundary true
                straight->setFlag (insideFlag);
                if (straight->has_neighbour(firsti)) {
                    straight = straight->get_neighbour (firsti);
                } else {
                    // no further neighbour in this direction
                    if (straight->testFlags(bdryFlag) == false) {
#ifdef DO_WARNINGS
                        if (!warning_given) {
                            std::cerr << "WARNING: Got to edge of region (dirn " << firsti
                                      << ") without encountering a boundary Hex.\n";
                            warning_given = true;
                        }
#endif
                        break;
                    }
                }
            }
        }

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
                                        unsigned int insideFlag = HEX_INSIDE_BOUNDARY) const
        {
            bool gotnextneighbour = false;

            // From each boundary hex, loop round all 6 neighbours until we get to a new neighbour
            for (unsigned short i = 0; i < 6 && gotnextneighbour == false; ++i) {

                // This is "if it's a neighbour and the neighbour is a boundary hex"
                if (bhi->has_neighbour(i) && bhi->get_neighbour(i)->testFlags(bdryFlag)) {

                    // cbhi is "candidate boundary hex iterator", now guaranteed to be a boundary hex
                    std::list<morph::Hex>::iterator cbhi = bhi->get_neighbour(i);

                    // Test if the candidate boundary hex is in the 'recently seen' deque
                    bool hex_already_seen = false;
                    for (auto rs : recently_seen) {
                        if (rs == cbhi) {
                            // This candidate hex has been recently seen. continue to next i
                            hex_already_seen = true;
                        }
                    }
                    if (hex_already_seen) { continue; }

                    unsigned short i_opp = ((i+3)%6);

                    // Go round each of the candidate boundary hex's neighbours (but j!=i)
                    for (unsigned short j = 0; j < 6; ++j) {

                        // Ignore the candidate boundary hex itself. if j==i_opp, then
                        // i's neighbour in dirn morph::Hex::neighbour_pos(j) is the
                        // candidate iself, continue to next i
                        if (j==i_opp) { continue; }

                        // What is this logic. If the candidate boundary hex (which is already
                        // known to be on the boundary) has a neighbour which is inside the
                        // boundary and not itself a boundary hex, then cbhi IS the next
                        // boundary hex.
                        if (cbhi->has_neighbour(j)
                            && cbhi->get_neighbour(j)->testFlags(insideFlag)==true
                            && cbhi->get_neighbour(j)->testFlags(bdryFlag)==false) {
                            recently_seen.push_back (bhi);
                            if (recently_seen.size() > n_recents) { recently_seen.pop_front(); }
                            bhi = cbhi;
                            gotnextneighbour = true;
                            break;
                        }
                    }
                }
            }

            return (gotnextneighbour);
        }

        /*!
         * Mark hexes as insideBoundary if they are inside the boundary. Starts from
         * \a hi which is assumed to already be known to refer to a hex lying inside the
         * boundary.
         */
        void markHexesInside (std::list<Hex>::iterator hi,
                              unsigned int bdryFlag = HEX_IS_BOUNDARY,
                              unsigned int insideFlag = HEX_INSIDE_BOUNDARY)
        {
            // Run to boundary, marking as we go
            std::list<morph::Hex>::iterator bhi(hi);
            while (bhi->testFlags (bdryFlag) == false && bhi->has_nne()) {
                bhi->setFlag (insideFlag);
                bhi = bhi->nne;
            }
            std::list<morph::Hex>::iterator bhi_start = bhi;

            // Mark from first boundary hex and across the region
            this->markFromBoundary (bhi, bdryFlag, insideFlag);

            // a deque to hold the 'n_recents' most recently seen boundary hexes.
            std::deque<std::list<morph::Hex>::iterator> recently_seen;
            size_t n_recents = 16; // 2 should be sufficient for boundaries with double thickness
            // sections. If problems occur, trying increasing this.
            bool gotnext = this->findNextBoundaryNeighbour (bhi, recently_seen, n_recents, bdryFlag, insideFlag);
            // Loop around boundary, marking inwards in all possible directions from each boundary hex
            while (gotnext && bhi != bhi_start) {
                this->markFromBoundary (bhi, bdryFlag, insideFlag);
                gotnext = this->findNextBoundaryNeighbour (bhi, recently_seen, n_recents, bdryFlag, insideFlag);
            }
        }

        /*!
         * Recursively mark hexes to be kept if they are inside the rectangular hex
         * domain.
         */
        void markHexesInsideRectangularDomain (const std::array<int, 6>& extnts)
        {
            // Check ri,gi,bi and reduce to equivalent ri,gi,bi=0.  Use gi to determine whether outside
            // top/bottom region Add gi contribution to ri to determine whether outside left/right region

            // Is the bottom row's gi even or odd?  extnts[2] is gi for the bottom row. If it's even, then
            // we add 0.5 to all rows with even gi. If it's odd then we add 0.5 to all rows with ODD gi.
            float even_addn = 0.5f;
            float odd_addn = 0.0f;
            float addleft = 0;
            if (extnts[2]%2 == 0) {
                // bottom row has EVEN gi (extnts[2])
                even_addn = 0.0f;
                odd_addn = 0.5f;
            } else {
                // bottom row has odd gi (extnts[2])
                addleft += 0.5f;
            }

            if (std::abs(extnts[2]%2) == std::abs(extnts[4]%2)) {
                // Left most hex is on a parity-matching line to bottom line, no need to add left.
            } else {
                // Need to add left.
                if (extnts[2]%2 == 0) {
                    addleft += 1.0f;
                    // For some reason, only in this case do we addleft (and not in the case where BR is
                    // ODD and Left most hex NOT matching, which makes addleft = 0.5 + 0.5). I can't work
                    // it out.
                    this->d_rowlen += addleft;
                    this->d_size = this->d_rowlen * this->d_numrows;
                } else {
                    addleft += 0.5f;
                }
            }

            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {

                // Here, hz is "horizontal index", made up of the ri index, half the gi index.
                //
                // plus a row-varying addition of a half (the row of hexes above is shifted right by 0.5 a
                // hex width).
                float hz = hi->ri + 0.5*(hi->gi); /*+ (hi->gi%2 ? odd_addn : even_addn)*/;
                float parityhalf = (hi->gi%2 ? odd_addn : even_addn);

                if (hz < (extnts[0] - addleft + parityhalf)) {
                    // outside
                } else if (hz > (extnts[1] + parityhalf)) {
                    // outside
                } else if (hi->gi < extnts[2]) {
                    // outside
                } else if (hi->gi > extnts[3]) {
                    // outside
                } else {
                    // inside
                    hi->setInsideDomain();
                }
                ++hi;
            }
        }

        /*!
         * Mark hexes to be kept if they are in a parallelogram domain.
         */
        void markHexesInsideParallelogramDomain (const std::array<int, 6>& extnts)
        {
            // Check ri,gi,bi and reduce to equivalent ri,gi,bi=0.  Use gi to determine whether outside
            // top/bottom region Add gi contribution to ri to determine whether outside left/right region
            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->ri < extnts[0]
                    || hi->ri > extnts[1]
                    || hi->gi < extnts[2]
                    || hi->gi > extnts[3]) {
                    // outside
                } else {
                    // inside
                    hi->setInsideDomain();
                }
                ++hi;
            }
        }

        /*!
         * Mark ALL hexes as inside the domain
         */
        void markAllHexesInsideDomain()
        {
            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                hi->setInsideDomain();
                hi++;
            }
        }

        /*!
         * Discard hexes in this->hexen that are outside the boundary #boundary.
         */
        void discardOutsideBoundary()
        {
            // Mark those hexes inside the boundary
            std::list<morph::Hex>::iterator centroidHex = this->findHexNearest (this->boundaryCentroid);
            this->markHexesInside (centroidHex);
            // Run through and discard those hexes outside the boundary:
            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->testFlags(HEX_INSIDE_BOUNDARY) == false) {
                    // When erasing a Hex, I need to update the neighbours of its neighbours.
                    hi->disconnectNeighbours();
                    // Having disconnected the neighbours, erase the Hex.
                    hi = this->hexen.erase (hi);
                } else {
                    ++hi;
                }
            }
            // The Hex::vi indices need to be re-numbered.
            this->renumberVectorIndices();
            // Finally, do something about the hexagonal grid vertices; set this to true to mark that the
            // iterators to the outermost vertices are no longer valid and shouldn't be used.
            this->gridReduced = true;
        }

        /*!
         * Discard hexes in this->hexen that are outside the rectangular hex domain.
         */
        void discardOutsideDomain()
        {
            // Similar to discardOutsideBoundary:
            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->insideDomain() == false) {
                    hi->disconnectNeighbours();
                    hi = this->hexen.erase (hi);
                } else {
                    ++hi;
                }
            }
            this->renumberVectorIndices();
            this->gridReduced = true;
        }

        /*!
         * Find the extents of the boundary hexes. Find the ri for the left-most hex and
         * the ri for the right-most hex (elements 0 and 1 of the return array). Find
         * the gi for the top most hex and the gi for the bottom most hex. Assumes bi is
         * 0.
         *
         * Return object contains: {ri-left, ri-right, gi-bottom, gi-top, gi at ri-left,
         * gi at ri-right}
         *
         * gi at ri-left, gi at ri-right are returned so that the bottom left hex can be
         * set correctly and the entire boundary is enclosed - it's important to know if
         * the bottom line is parity-matched with the line on which the left and right
         * most boundary hexes are found.
         */
        std::array<int, 6> findBoundaryExtents() const
        {
            // Return object contains {ri-left, ri-right, gi-bottom, gi-top, gi at ri-left, gi at ri-right}
            // i.e. {xmin, xmax, ymin, ymax, gi at xmin, gi at xmax}
            std::array<int, 6> rtn = {{0,0,0,0,0,0}};

            // Check to see if there are any boundary hexes at all.
            unsigned int bhcount = 0;
            for (auto h : this->hexen) { bhcount += h.testFlags(HEX_IS_BOUNDARY) == true ? 1 : 0; }
            if (bhcount == 0) { return rtn; }

            // Find the furthest left and right hexes and the further up and down hexes.
            std::array<float, 4> limits = {{0,0,0,0}};
            bool first = true;
            for (auto h : this->hexen) {
                if (h.testFlags(HEX_IS_BOUNDARY) == true) {
                    if (first) {
                        limits = {{h.x, h.x, h.y, h.y}};
                        first = false;
                    }
                    if (h.x < limits[0]) {
                        limits[0] = h.x;
                        rtn[4] = h.gi;
                    }
                    if (h.x > limits[1]) {
                        limits[1] = h.x;
                        rtn[5] = h.gi;
                    }
                    if (h.y < limits[2]) {
                        limits[2] = h.y;
                    }
                    if (h.y > limits[3]) {
                        limits[3] = h.y;
                    }
                }
            }

            // Now compute the ri and gi values that these xmax/xmin/ymax/ymin correspond to. THIS, if
            // nothing else, should auto-vectorise!  d_ri is the distance moved in ri direction per x, d_gi
            // is distance
            float d_ri = this->hexen.front().getD();
            float d_gi = this->hexen.front().getV();
            rtn[0] = (int)(limits[0] / d_ri);
            rtn[1] = (int)(limits[1] / d_ri);
            rtn[2] = (int)(limits[2] / d_gi);
            rtn[3] = (int)(limits[3] / d_gi);

            // Add 'growth buffer'
            rtn[0] -= this->d_growthbuffer_horz;
            rtn[1] += this->d_growthbuffer_horz;
            rtn[2] -= this->d_growthbuffer_vert;
            rtn[3] += this->d_growthbuffer_vert;

            return rtn;
        }

        /*!
         * Does what it says on the tin. Re-number the Hex::vi vector index in each
         * Hex in the HexGrid, from the start of the list<Hex> hexen until the end.
         */
        void renumberVectorIndices()
        {
            unsigned int vi = 0;
            this->vhexen.clear();
            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                hi->vi = vi++;
                this->vhexen.push_back (&(*hi));
                ++hi;
            }
        }

        /*!
         * The centre to centre hex distance between adjacent members of the hex grid.
         */
        float d = 1.0f;

        /*!
         * The centre to centre hex distance between hexes on adjacent rows - the
         * 'vertical' distance.
         */
        float v = 1.0f * morph::mathconst<float>::root_3_over_2;

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

        /*
         * Hex references to the hexes on the vertices of the hexagonal
         * grid. Configured during init(). These will become invalid when a new
         * boundary is applied to the original hexagonal grid. When this occurs,
         * gridReduced should be set false.
         */
        std::list<Hex>::iterator vertexE;
        std::list<Hex>::iterator vertexNE;
        std::list<Hex>::iterator vertexNW;
        std::list<Hex>::iterator vertexW;
        std::list<Hex>::iterator vertexSW;
        std::list<Hex>::iterator vertexSE;

        /*!
         * Set true when a new boundary has been applied. This means that
         * the #vertexE, #vertexW, and similar iterators are no longer valid.
         */
        bool gridReduced = false;

    };

} // namespace morph
