/*
 * Author: Seb James
 *
 * Date: 2018/07
 */
#pragma once

#include <morph/Hex.h>
#include <morph/BezCurvePath.h>
#include <morph/BezCoord.h>
#include <morph/MathConst.h>
#include <morph/HdfData.h>

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
        void populate_d_neighbours (void)
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
        void d_clear (void)
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
        HexGrid (float d_, float x_span_, float z_ = 0.0f,
                 HexDomainShape shape = HexDomainShape::Boundary)
        {
            this->d = d_;
            this->v = this->d * SQRT_OF_3_OVER_2_F;
            this->x_span = x_span_;
            this->z = z_;
            this->domainShape = shape;
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
            this->v = this->d * SQRT_OF_3_OVER_2_F;
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

            if (this->domainShape == morph::HexDomainShape::Boundary) {
                // Boundary IS contiguous, discard hexes outside the boundary.
                this->discardOutsideBoundary();
            } else {
                throw std::runtime_error ("For now, setBoundary (const list<Hex>& pHexes) doesn't know what to "
                                          "do if domain shape is not HexDomainShape::Boundary.");
            }

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
                this->boundaryCentroid = std::make_pair (0.0, 0.0);
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

            if (this->domainShape == morph::HexDomainShape::Boundary) {
                this->discardOutsideBoundary();
                this->populate_d_vectors();
            } else {
                // Given that the boundary IS contiguous, can now set a domain of hexes (rectangular,
                // parallelogram or hexagonal region, such that computations can be efficient) and discard
                // hexes outside the domain.  setDomain() will define a regular domain, then discard those
                // hexes outside the regular domain and populate all the d_ vectors.
                this->setDomain();
            }
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
                this->boundaryCentroid = std::make_pair (0.0, 0.0);
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
        void setBoundaryOnOuterEdge (void)
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

            if (this->domainShape == morph::HexDomainShape::Boundary) {
                // Boundary IS contiguous, discard hexes outside the boundary.
                this->discardOutsideBoundary();
            } else {
                throw std::runtime_error ("For now, setBoundary (const list<Hex>& pHexes) doesn't know what to do if domain shape is not HexDomainShape::Boundary.");
            }

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
        std::list<Hex> getBoundary (void) const
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
                                                       const std::pair<float, float> c = std::make_pair(0.0, 0.0))
        {
            std::vector<morph::BezCoord<float>> bpoints;
            throw std::runtime_error ("HexGrid::rectangleCompute: Implement me");
            return bpoints;
        }

        /*!
         * Compute a set of coordinates arranged as a parallelogram
         * \param r Number of hexes to the E (and to the W)
         * \param g Number of hexes to the NE (and SW)
         * \param c centre argument so that the parallelogram centre is offset from the coordinate origin
         * \return A vector of the coordinates of points on the generated pgram
         */
        std::vector<BezCoord<float>> parallelogramCompute (const int r, const int g,
                                                           const std::pair<float, float> c = std::make_pair(0.0, 0.0))
        {
            std::vector<morph::BezCoord<float>> bpoints;
            throw std::runtime_error ("HexGrid::parallelogramCompute: Implement me");
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
                                                     const std::pair<float, float> c = std::make_pair(0.0, 0.0))
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
            for (double phi = 0.0; phi < morph::TWO_PI_D; phi+=delta_phi) {
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
            double p = M_PI * apb * sum;

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
                                    const std::pair<float, float> c = std::make_pair(0.0, 0.0), bool offset=true)
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
                                  const std::pair<float, float> c = std::make_pair(0.0, 0.0), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = ellipseCompute (a, a, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * Set up a rectangular boundary of width x and height y
         */
        void setRectangularBoundary (const float x, const float y,
                                     const std::pair<float, float> c = std::make_pair(0.0, 0.0), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = rectangleCompute (x, y, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * Set up a parallelogram boundary extending r hexes to the E and g hexes to the NE
         */
        void setParallelogramBoundary (const int r, const int g,
                                       const std::pair<float, float> c = std::make_pair(0.0, 0.0), bool offset=true)
        {
            std::vector<morph::BezCoord<float>> bpoints = parallelogramCompute (r, g, c);
            this->setBoundary (bpoints, offset);
        }

        /*!
         * To use the originally generated hexagonal domain as a simple HexGrid,k call
         * this to ensure vector indices and the domain are all set up as they should
         * be.
         */
        void leaveAsHexagon()
        {
            this->renumberVectorIndices();
            this->setDomain();
        }

        /*!
         * \brief Accessor for the size of hexen.
         *
         * return The number of hexes in the grid.
         */
        unsigned int num (void) const { return this->hexen.size(); }

        /*!
         * \brief Obtain the vector index of the last Hex in hexen.
         *
         * return Hex::vi from the last Hex in the grid.
         */
        unsigned int lastVectorIndex (void) const { return this->hexen.rbegin()->vi; }

        /*!
         * Output some text information about the hexgrid.
         */
        std::string output (void) const
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
        std::string extent (void) const
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
        float width (void) const
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
        float depth (void) const
        {
            std::array<int, 6> extents = this->findBoundaryExtents();
            float ymin = this->v * float(extents[2]);
            float ymax = this->v * float(extents[3]);
            return (ymax - ymin);
        }

        /*!
         * Getter for d.
         */
        float getd (void) const { return this->d; }

        /*!
         * Getter for v - vertical hex spacing.
         */
        float getv (void) const { return this->v; }

        /*!
         * Get the shortest distance from the centre to the perimeter. This is the
         * "short radius".
         */
        float getSR (void) const { return this->d/2; }

        /*!
         * The distance from the centre of the Hex to any of the vertices. This is the
         * "long radius".
         */
        float getLR (void) const { return (this->d/morph::SQRT_OF_3_F); }

        /*!
         * The vertical distance from the centre of the hex to the "north east" vertex
         * of the hex.
         */
        float getVtoNE (void) const { return (this->d/(2.0f*morph::SQRT_OF_3_F)); }

        /*!
         * Compute and return the area of one hex in the grid. The area is that of 6
         * triangles: (1/2 LR * d/2) * 6 // or (d*d*3)/(2*sqrt(3)) = d * d * sqrt(3)/2
         */
        float getHexArea (void) const { return (this->d * this->d * morph::SQRT_OF_3_OVER_2_F); }

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
        void computeDistanceToBoundary (void)
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
         * Populate d_ vectors. simple version. (Finds extents, then calls
         * populate_d_vectors(const array<int, 6>&)
         */
        void populate_d_vectors (void)
        {
            std::array<int, 6> extnts = this->findBoundaryExtents();
            this->populate_d_vectors (extnts);
        }

        /*!
         * Populate d_ vectors, paying attention to domainShape.
         */
        void populate_d_vectors (const std::array<int, 6>& extnts)
        {
            // First, find the starting hex. For Rectangular and parallelogram domains,
            // that's the bottom left hex.
            std::list<morph::Hex>::iterator hi = this->hexen.begin();
            // bottom left hex.
            std::list<morph::Hex>::iterator blh = this->hexen.end();

            if (this->domainShape == morph::HexDomainShape::Rectangle
                || this->domainShape == morph::HexDomainShape::Parallelogram) {

                // Use neighbour relations to go from bottom left to top right.  Find hex on bottom row.
                while (hi != this->hexen.end()) {
                    if (hi->gi == extnts[2]) {
                        // We're on the bottom row
                        break;
                    }
                    ++hi;
                }
                // hi now on bottom row; so travel west
                while (hi->has_nw() == true) { hi = hi->nw; }

                // hi should now be the bottom left hex.
                blh = hi;

                // Sanity check
                if (blh->has_nne() == false || blh->has_ne() == false || blh->has_nnw() == true) {
                    std::stringstream ee;
                    ee << "We expect the bottom left hex to have an east and a "
                       << "north east neighbour, but no north west neighbour. This has: "
                       << (blh->has_nne() == true ? "Neighbour NE ":"NO Neighbour NE ")
                       << (blh->has_ne() == true ? "Neighbour E ":"NO Neighbour E ")
                       << (blh->has_nnw() == true ? "Neighbour NW ":"NO Neighbour NW ");
                    throw std::runtime_error (ee.str());
                }
            } // else Hexagon or Boundary starts from 0, hi already set to hexen.begin();

            // Clear the d_ vectors.
            this->d_clear();

            // Now raster through the hexes, building the d_ vectors.
            if (this->domainShape == morph::HexDomainShape::Rectangle) {
                bool next_row_ne = true;
                this->d_push_back (hi);
                do {
                    hi = hi->ne;

                    this->d_push_back (hi);

                    if (hi->has_ne() == false) {
                        if (hi->gi == extnts[3]) {
                            // last (i.e. top) row and no neighbour east, so finished.
                            break;
                        } else {
                            if (next_row_ne == true) {
                                hi = blh->nne;
                                next_row_ne = false;
                                blh = hi;
                            } else {
                                hi = blh->nnw;
                                next_row_ne = true;
                                blh = hi;
                            }
                            this->d_push_back (hi);
                        }
                    }
                } while (hi->has_ne() == true);

            } else if (this->domainShape == morph::HexDomainShape::Parallelogram) {

                this->d_push_back (hi); // Push back the first one, which is guaranteed to have a NE
                while (hi->has_ne() == true) {

                    // Step to new hex to the E
                    hi = hi->ne;

                    if (hi->has_ne() == false) {
                        // New hex has no NE, so it is on end of row.
                        if (hi->gi == extnts[3]) {
                            // on end of top row and no neighbour east, so finished; push back and break
                            this->d_push_back (hi);
                            break;
                        } else {
                            // On end of non-top row, so push back...
                            this->d_push_back (hi);
                            // do the 'carriage return'...
                            hi = blh->nne;
                            // And push that back...
                            this->d_push_back (hi);
                            // Update the new 'start of last row' iterator
                            blh = hi;
                        }
                    } else {
                        // New hex does have neighbour east, so just push it back.
                        this->d_push_back (hi);
                    }
                }

            } else { // Hexagon or Boundary

                while (hi != this->hexen.end()) {
                    this->d_push_back (hi);
                    hi++;
                }
            }

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

        /*!
         * For every hex in hexen, unset the flags HEX_IS_REGION_BOUNDARY and
         * HEX_INSIDE_REGION
         */
        void clearRegionBoundaryFlags (void)
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
        void init (void)
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
        bool boundaryContiguous (void)
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
            this->bhexen.push_back ((morph::Hex*)&(*hi));

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
            this->bhexen.push_back ((morph::Hex*)&(*hi));

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
            float d = h->distanceFrom (point);
            float d_ = 0.0f;

            while (neighbourNearer == true) {

                neighbourNearer = false;
                if (h->has_ne() && (d_ = h->ne->distanceFrom (point)) < d) {
                    d = d_;
                    h = h->ne;
                    neighbourNearer = true;

                } else if (h->has_nne() && (d_ = h->nne->distanceFrom (point)) < d) {
                    d = d_;
                    h = h->nne;
                    neighbourNearer = true;

                } else if (h->has_nnw() && (d_ = h->nnw->distanceFrom (point)) < d) {
                    d = d_;
                    h = h->nnw;
                    neighbourNearer = true;

                } else if (h->has_nw() && (d_ = h->nw->distanceFrom (point)) < d) {
                    d = d_;
                    h = h->nw;
                    neighbourNearer = true;

                } else if (h->has_nsw() && (d_ = h->nsw->distanceFrom (point)) < d) {
                    d = d_;
                    h = h->nsw;
                    neighbourNearer = true;

                } else if (h->has_nse() && (d_ = h->nse->distanceFrom (point)) < d) {
                    d = d_;
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
        void markAllHexesInsideDomain (void)
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
        void discardOutsideBoundary (void)
        {
            // Mark those hexes inside the boundary
            std::list<morph::Hex>::iterator centroidHex = this->findHexNearest (this->boundaryCentroid);
            this->markHexesInside (centroidHex);
            // Run through and discard those hexes outside the boundary:
            auto hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->testFlags(HEX_INSIDE_BOUNDARY) == false) {
                    // When erasing a Hex, I need to update the neighbours of its
                    // neighbours.
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
        void discardOutsideDomain (void)
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
        std::array<int, 6> findBoundaryExtents (void) const
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
         * setDomain() will define a regular domain, then discard those hexes outside
         * the regular domain and populate all the d_ vectors.
         *
         * setDomain() ASSUMES that a boundary has already been set.
         */
        void setDomain (void)
        {
            // 1. Find extent of boundary, both left/right and up/down, with 'buffer region' already added.
            std::array<int, 6> extnts = this->findBoundaryExtents();
            for (auto e : extnts) { std::cout << e << std::endl; }
            // 1.5 set rowlen and numrows
            this->d_rowlen = extnts[1]-extnts[0]+1;
            this->d_numrows = extnts[3]-extnts[2]+1;
            this->d_size = this->d_rowlen * this->d_numrows;
            // 2. Mark Hexes inside whichever domain
            if (this->domainShape == morph::HexDomainShape::Rectangle) {
                this->markHexesInsideRectangularDomain (extnts);
            } else if (this->domainShape == morph::HexDomainShape::Parallelogram) {
                this->markHexesInsideParallelogramDomain (extnts);
            } else if (this->domainShape == morph::HexDomainShape::Hexagon) {
                // The original domain was hexagonal, so just mark ALL of them as being in the domain.
                this->markAllHexesInsideDomain();
            } else {
                throw std::runtime_error ("Unknown HexDomainShape");
            }
            // 3. Discard hexes outside domain
            this->discardOutsideDomain();
            // 3.5 Mark hexes inside boundary
            std::list<morph::Hex>::iterator centroidHex = this->findHexNearest (this->boundaryCentroid);
            this->markHexesInside (centroidHex);
            // Before populating d_ vectors, also compute the distance to boundary
            this->computeDistanceToBoundary();
            // 4. Populate d_ vectors
            this->populate_d_vectors (extnts);
        }

        /*!
         * Does what it says on the tin. Re-number the Hex::vi vector index in each
         * Hex in the HexGrid, from the start of the list<Hex> hexen until the end.
         */
        void renumberVectorIndices (void)
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
         * Set true when a new boundary or domain has been applied. This means that
         * the #vertexE, #vertexW, and similar iterators are no longer valid.
         */
        bool gridReduced = false;

    };

} // namespace morph
