/*
 * Author: Seb James
 *
 * Date: 2021/02
 */
#pragma once

#include <morph/Rect.h>
#include <morph/BezCurvePath.h>
#include <morph/BezCoord.h>
#include <morph/MathConst.h>
#include <morph/HdfData.h>
#include <morph/Vector.h>

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

    enum class CartDomainShape {
        Rectangle,
        Boundary // The shape of the arbitrary boundary set with CartGrid::setBoundary
    };

    /*!
     * This class is used to build an cartesian grid of rectangular elements.
     *
     * It has been developed from HexGrid.h. It looks byzantine in complexity, given
     * than it's 'only' supposed to provide a way to track a rectangular grid. This is
     * because, as with HexGrid.h, the initial grid is intended to provide a region from
     * which an arbitrary boundary region can be 'cut out' AND it maintains all the
     * neighbour relationships correctly.
     *
     * Optionally, a boundary may be set by calling setBoundary (const
     * BezCurvePath&). If this is done, then the boundary is converted to a set of
     * elements, then those elements in the grid lying outside the boundary are removed.
     */
    class alignas(8) CartGrid
    {
    public:
        /*
         * Domain attributes
         * -----------------
         *
         * Vectors containing the "domain" info extracted from the list of elements. The
         * "domain" is the set of elements left over after the boundary has been applied
         *
         * Each of these is prefixed d_ and is carefully aligned.
         *
         * The order in which these are populated is raster-style, from top left to
         * bottom right.
         */
        alignas(alignof(std::vector<float>)) std::vector<float> d_x;
        alignas(alignof(std::vector<float>)) std::vector<float> d_y;

        /*
         * Neighbour iterators. For use when the stride to the neighbour ne or nw is not
         * constant. On a Cartesian grid, these are necessary if an arbitrary boundary
         * has been applied.
         */
        alignas(8) std::vector<int> d_ne;
        alignas(8) std::vector<int> d_nne;
        alignas(8) std::vector<int> d_nn;
        alignas(8) std::vector<int> d_nnw;
        alignas(8) std::vector<int> d_nw;
        alignas(8) std::vector<int> d_nsw;
        alignas(8) std::vector<int> d_ns;
        alignas(8) std::vector<int> d_nse;

        alignas(8) std::vector<int> d_xi;
        alignas(8) std::vector<int> d_yi;

        /*!
         * Flags, such as "on boundary", "inside boundary", "outside boundary", "has
         * neighbour east", etc.
         */
        alignas(8) std::vector<unsigned int> d_flags;

        //! Distance to boundary for any element.
        alignas(8) std::vector<float> d_distToBoundary;

        /*!
         * How many additional rects to grow out to the left and right; top and
         * bottom? Set this to a larger number if the boundary is expected to grow
         * during a simulation.
         */
        unsigned int d_growthbuffer_horz = 0;
        unsigned int d_growthbuffer_vert = 0;

        //! Add entries to all the d_ vectors for the Rect pointed to by ri.
        void d_push_back (std::list<Rect>::iterator ri)
        {
            d_x.push_back (ri->x);
            d_y.push_back (ri->y);
            d_xi.push_back (ri->xi);
            d_yi.push_back (ri->yi);
            d_flags.push_back (ri->getFlags());
            d_distToBoundary.push_back (ri->distToBoundary);

            // record in the Rect the iterator in the d_ vectors so that d_nne and friends can be set up later.
            ri->di = d_x.size()-1;
        }

        //! Once Rect::di attributes have been set, populate d_nne and friends.
        void populate_d_neighbours()
        {
            // Resize d_nne and friends
            this->d_nne.resize (this->d_x.size(), 0);
            this->d_nn.resize (this->d_x.size(), 0);
            this->d_ne.resize (this->d_x.size(), 0);
            this->d_nnw.resize (this->d_x.size(), 0);
            this->d_nw.resize (this->d_x.size(), 0);
            this->d_nsw.resize (this->d_x.size(), 0);
            this->d_ns.resize (this->d_x.size(), 0);
            this->d_nse.resize (this->d_x.size(), 0);

            std::list<morph::Rect>::iterator ri = this->rects.begin();
            while (ri != this->rects.end()) {

                if (ri->has_ne() == true) {
                    this->d_ne[ri->di] = ri->ne->di;
                } else {
                    this->d_ne[ri->di] = -1;
                }

                if (ri->has_nne() == true) {
                    this->d_nne[ri->di] = ri->nne->di;
                } else {
                    this->d_nne[ri->di] = -1;
                }

                if (ri->has_nn() == true) {
                    this->d_nn[ri->di] = ri->nn->di;
                } else {
                    this->d_nn[ri->di] = -1;
                }

                if (ri->has_nnw() == true) {
                    this->d_nnw[ri->di] = ri->nnw->di;
                } else {
                    this->d_nnw[ri->di] = -1;
                }

                if (ri->has_nw() == true) {
                    this->d_nw[ri->di] = ri->nw->di;
                } else {
                    this->d_nw[ri->di] = -1;
                }

                if (ri->has_nsw() == true) {
                    this->d_nsw[ri->di] = ri->nsw->di;
                } else {
                    this->d_nsw[ri->di] = -1;
                }

                if (ri->has_ns() == true) {
                    this->d_ns[ri->di] = ri->ns->di;
                } else {
                    this->d_ns[ri->di] = -1;
                }

                if (ri->has_nse() == true) {
                    this->d_nse[ri->di] = ri->nse->di;
                } else {
                    this->d_nse[ri->di] = -1;
                }

                ++ri;
            }
        }

        //! Clear out all the d_ vectors
        void d_clear()
        {
            this->d_x.clear();
            this->d_y.clear();
            this->d_xi.clear();
            this->d_yi.clear();
            this->d_flags.clear();
        }

        /*!
         * Save this CartGrid (and all the rects in it) into the HDF5 file at the
         * location \a path.
         */
        void save (const std::string& path)
        {
            morph::HdfData cgdata (path);
            cgdata.add_val ("/d", d);
            cgdata.add_val ("/v", v);
            cgdata.add_val ("/x_span", x_span);
            cgdata.add_val ("/y_span", y_span);
            cgdata.add_val ("/z", z);
            cgdata.add_val ("/d_growthbuffer_horz", d_growthbuffer_horz);
            cgdata.add_val ("/d_growthbuffer_vert", d_growthbuffer_vert);

            // pair<float,float>
            cgdata.add_contained_vals ("/boundaryCentroid", boundaryCentroid);

            // Don't save BezCurvePath boundary - limit this to the ability to
            // save which elements are boundary elements and which aren't

            // Don't save vertexE, vertexNE etc. Make sure to set gridReduced
            // = true when calling load()

            // vector<float>
            cgdata.add_contained_vals ("/d_x", d_x);
            cgdata.add_contained_vals ("/d_y", d_y);
            cgdata.add_contained_vals ("/d_distToBoundary", d_distToBoundary);
            // vector<int>
            cgdata.add_contained_vals ("/d_xi", d_xi);
            cgdata.add_contained_vals ("/d_yi", d_yi);

            cgdata.add_contained_vals ("/d_ne", d_ne);
            cgdata.add_contained_vals ("/d_nne", d_nne);
            cgdata.add_contained_vals ("/d_nn", d_nn);
            cgdata.add_contained_vals ("/d_nnw", d_nnw);
            cgdata.add_contained_vals ("/d_nw", d_nw);
            cgdata.add_contained_vals ("/d_nsw", d_nsw);
            cgdata.add_contained_vals ("/d_ns", d_ns);
            cgdata.add_contained_vals ("/d_nse", d_nse);

            // vector<unsigned int>
            cgdata.add_contained_vals ("/d_flags", d_flags);

            // list<Rect> rects
            // for i in list, save Rect
            std::list<morph::Rect>::const_iterator r = this->rects.begin();
            unsigned int rcount = 0;
            while (r != this->rects.end()) {
                // Make up a path
                std::string h5path = "/rects/" + std::to_string(rcount);
                r->save (cgdata, h5path);
                ++r;
                ++rcount;
            }
            cgdata.add_val ("/rcount", rcount);

            // What about vrects? Probably don't save and re-call method to populate.
            this->renumberVectorIndices();

            // What about brects? Probably re-run/test this->boundaryContiguous() on load.
            this->boundaryContiguous();
        }

        /*!
         * Populate this CartGrid from the HDF5 file at the location \a path.
         */
        void load (const std::string& path)
        {
            morph::HdfData cgdata (path, true);
            cgdata.read_val ("/d", this->d);
            cgdata.read_val ("/v", this->v);
            cgdata.read_val ("/x_span", this->x_span);
            cgdata.read_val ("/y_span", this->y_span);
            cgdata.read_val ("/z", this->z);
            cgdata.read_val ("/d_growthbuffer_horz", this->d_growthbuffer_horz);
            cgdata.read_val ("/d_growthbuffer_vert", this->d_growthbuffer_vert);

            cgdata.read_contained_vals ("/boundaryCentroid", this->boundaryCentroid);
            cgdata.read_contained_vals ("/d_x", this->d_x);
            cgdata.read_contained_vals ("/d_y", this->d_y);
            cgdata.read_contained_vals ("/d_distToBoundary", this->d_distToBoundary);
            cgdata.read_contained_vals ("/d_xi", this->d_xi);
            cgdata.read_contained_vals ("/d_yi", this->d_yi);
            cgdata.read_contained_vals ("/d_ne", this->d_ne);
            cgdata.read_contained_vals ("/d_nne", this->d_nne);
            cgdata.read_contained_vals ("/d_nnw", this->d_nnw);
            cgdata.read_contained_vals ("/d_nw", this->d_nw);
            cgdata.read_contained_vals ("/d_nsw", this->d_nsw);
            cgdata.read_contained_vals ("/d_nse", this->d_nse);

            // Assume a boundary has been applied so set this true. Also, the CartGrid::save method doesn't
            // save CartGrid::vertexE, etc
            this->gridReduced = true;

            unsigned int rcount = 0;
            cgdata.read_val ("/rcount", rcount);
            for (unsigned int i = 0; i < rcount; ++i) {
                std::string h5path = "/rects/" + std::to_string(i);
                morph::Rect r(cgdata, h5path);
                this->rects.push_back (r);
            }

            // After creating rects list, need to set neighbour relations in each Rect, as loaded in d_ne,
            // etc.
            for (morph::Rect& _r : this->rects) {
                DBG ("Set neighbours for Rect " << _r.outputRG());
                // For each Rect, six loops through rects:
                if (_r.has_ne() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_ne[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.ne = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour E relation...");
                    }
                }

                if (_r.has_nne() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nne[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nne = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour NE relation...");
                    }
                }

                if (_r.has_nn() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nn[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nn = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour N relation...");
                    }
                }

                if (_r.has_nnw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nnw[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nnw = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour NW relation...");
                    }
                }

                if (_r.has_nw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nw[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nw = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour W relation...");
                    }
                }

                if (_r.has_nsw() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nsw[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nsw = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour SW relation...");
                    }
                }

                if (_r.has_ns() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_ns[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.ns = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour S relation...");
                    }
                }

                if (_r.has_nse() == true) {
                    bool matched = false;
                    unsigned int neighb_it = (unsigned int) this->d_nse[_r.vi];
                    std::list<morph::Rect>::iterator ri = this->rects.begin();
                    while (ri != this->rects.end()) {
                        if (ri->vi == neighb_it) {
                            matched = true;
                            _r.nse = ri;
                            break;
                        }
                        ++ri;
                    }
                    if (!matched) {
                        throw std::runtime_error ("Failed to match rects neighbour SE relation...");
                    }
                }
            }
        }

        //! Default constructor creates symmetric grid centered about 0,0.
        CartGrid(): d(1.0f), v(1.0f), x_span(1.0f), y_span(1.0f), z(0.0f) {}

        //! Construct then load from file.
        CartGrid (const std::string& path) : d(1.0f), v(1.0f), x_span(1.0f), z(0.0f) { this->load (path); }

        //! Construct the initial grid with a square element distance of \a d_ and square size length x_span.
        CartGrid (float d_, float x_span_, float z_ = 0.0f,
                  CartDomainShape shape = CartDomainShape::Rectangle)
        : CartGrid (d_, d_, x_span_, x_span_, z_, shape) {}

        //! Construct with rectangular element width d_, height v_
        CartGrid (float d_, float v_, float x_span_, float y_span_, float z_ = 0.0f,
                  CartDomainShape shape = CartDomainShape::Rectangle)
        {
            this->d = d_;
            this->v = v_;
            this->x_span = x_span_;
            this->y_span = y_span_;
            this->z = z_;
            this->domainShape = shape;

            this->init();
        }

        //! Construct with rectangular element width d_, height v_ starting at location x1,y1 and creating to x2,y2.
        CartGrid (float d_, float v_, float x1, float y1, float x2, float y2, float z_ = 0.0f,
                  CartDomainShape shape = CartDomainShape::Rectangle)
        {
            this->d = d_;
            this->v = v_;
            this->x_span = x2 - x1;
            this->y_span = y2 - y1;
            this->z = z_;
            this->domainShape = shape;

            // init2 is the non-symmetic initialisation for making arbitrary rectangular grids.
            std::cout << "call init2("<<x1<<", etc)\n";
            this->init2 (x1, y1, x2, y2);
        }

        //! Initialisation common code
        void init (float d_, float v_, float x_span_, float y_span_, float z_ = 0.0f)
        {
            this->d = d_;
            this->v = v_;
            this->x_span = x_span_;
            this->y_span = y_span_;
            this->z = z_;
            this->init();
        }
        void init (float d_, float x_span_, float z_ = 0.0f)
        {
            this->init (d_, d_, x_span_, x_span_, z_);
        }

        //! Compute the centroid of the passed in list of Rectes.
        std::pair<float, float> computeCentroid (const std::list<Rect>& pRects)
        {
            std::pair<float, float> centroid;
            centroid.first = 0;
            centroid.second = 0;
            for (auto r : pRects) { // fixme replace with MathAlgo centroiding code
                centroid.first += r.x;
                centroid.second += r.y;
            }
            centroid.first /= pRects.size();
            centroid.second /= pRects.size();
            return centroid;
        }

        /*!
         * Sets boundary to match the list of rects passed in as \a pRects. Note, that
         * unlike void setBoundary (const BezCurvePath& p), this method does not apply
         * any offset to the positions of the rects in \a pRects.
         */
        void setBoundary (const std::list<Rect>& pRects)
        {
            this->boundaryCentroid = this->computeCentroid (pRects);

            std::list<morph::Rect>::iterator bpoint = this->rects.begin();
            std::list<morph::Rect>::iterator bpi = this->rects.begin();
            while (bpi != this->rects.end()) {
                std::list<morph::Rect>::const_iterator ppi = pRects.begin();
                while (ppi != pRects.end()) {
                    // NB: The assumption right now is that the pRects are from the same dimension grid
                    // as this->rects.
                    if (bpi->xi == ppi->xi && bpi->yi == ppi->yi) {
                        // Set h as boundary rect.
                        bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
                        bpoint = bpi;
                        break;
                    }
                    ++ppi;
                }
                ++bpi;
            }

            // Check that the boundary is contiguous.
            std::set<unsigned int> seen;
            std::list<morph::Rect>::iterator ri = bpoint;
            if (this->boundaryContiguous (bpoint, ri, seen, RECT_NEIGHBOUR_POS_E) == false) {
                std::cout << "Uh oh\n";
                throw std::runtime_error ("The boundary is not a contiguous sequence of rects.");
            }

            if (this->domainShape == morph::CartDomainShape::Boundary) {
                // Boundary IS contiguous, discard rects outside the boundary.
                this->discardOutsideBoundary();
            } else {
                throw std::runtime_error ("For now, setBoundary (const list<Rect>& pRects) doesn't know what to "
                                          "do if domain shape is not CartDomainShape::Boundary.");
            }

            this->populate_d_vectors();
        }

        /*!
         * Sets boundary to \a p, then runs the code to discard rects lying outside
         * this boundary. Finishes up by calling morph::CartGrid::discardOutside.
         * The BezCurvePath's centroid may not be 0,0. If loffset has its default value
         * of true, then this method offsets the boundary so that when it is applied to
         * the CartGrid, the centroid IS (0,0). If \a loffset is false, then \a p is not
         * translated in this way.
         */
        void setBoundary (const BezCurvePath<float>& p, bool loffset = true)
        {
            this->boundary = p;
            if (!this->boundary.isNull()) {
                // Compute the points on the boundary using half of the rect to rect
                // spacing as the step size. The 'true' argument inverts the y axis.
                this->boundary.computePoints (this->d/2.0f, true);
                std::vector<morph::BezCoord<float>> bpoints = this->boundary.getPoints();
                this->setBoundary (bpoints, loffset);
            }
        }

        /*!
         * This sets a boundary, just as morph::CartGrid::setBoundary(const
         * morph::BezCurvePath<float> p, bool offset) does but WITHOUT discarding rects
         * outside the boundary. Also, it first clears the previous boundary flags so
         * the new ones are the only ones marked on the boundary. It does this because
         * it does not discard rects outside the boundary or repopulate the CartGrid but
         * it draws a new boundary that can be used by client code
         */
        void setBoundaryOnly (const BezCurvePath<float>& p, bool loffset = true)
        {
            this->boundary = p;
            if (!this->boundary.isNull()) {
                this->boundary.computePoints (this->d/2.0f, true); // FIXME PROBABLY NEEDS TO BE DIFFERENT
                std::vector<morph::BezCoord<float>> bpoints = this->boundary.getPoints();
                this->setBoundaryOnly (bpoints, loffset);
            }
        }

        /*!
         * Sets the boundary of the hexgrid to \a bpoints, then runs the code to discard
         * rects lying outside this boundary. Finishes up by calling
         * CartGrid::discardOutside. By default, this method translates \a bpoints so
         * that when the boundary is applied to the CartGrid, its centroid is (0,0). If
         * the default value of \a loffset is changed to false, \a bpoints is NOT
         * translated.
         */
        void setBoundary (std::vector<BezCoord<float>>& bpoints, bool loffset = true)
        {
            this->boundaryCentroid = morph::BezCurvePath<float>::getCentroid (bpoints);

            auto bpi = bpoints.begin();
            // conditionally executed if we reset the centre
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
            std::list<morph::Rect>::iterator nearbyBoundaryPoint = this->rects.begin(); // i.e the Rect at 0,0
            bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
            }

            // Check that the boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Rect>::iterator hi = nearbyBoundaryPoint;
                if (this->boundaryContiguous (nearbyBoundaryPoint, hi, seen, RECT_NEIGHBOUR_POS_E) == false) {
                    throw std::runtime_error ("The constructed boundary is not a contiguous sequence of rectangular elements.");
                }
            }

            if (this->domainShape == morph::CartDomainShape::Boundary) {
                this->discardOutsideBoundary();
                this->populate_d_vectors();
            } else {
                throw std::runtime_error ("Use CartDomainShape::Boundary when setting a boundary");
            }
        }

        /*!
         * This sets a boundary, just as
         * morph::CartGrid::setBoundary(vector<morph::BezCoord<float>& bpoints, bool offset)
         * does but WITHOUT discarding rects outside the boundary. Also, it first clears
         * the previous boundary flags so the new ones are the only ones marked on the
         * boundary. It does this because it does not discard rects outside the boundary
         * or repopulate the CartGrid but it draws a new boundary that can be used by
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
            for (auto r : this->rects) { r.unsetUserFlag (RECT_IS_BOUNDARY); }

            std::list<morph::Rect>::iterator nearbyBoundaryPoint = this->rects.begin(); // i.e the Rect at 0,0
            bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
            }

            // Check that the boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Rect>::iterator ri = nearbyBoundaryPoint;
                if (this->boundaryContiguous (nearbyBoundaryPoint, ri, seen, RECT_NEIGHBOUR_POS_E) == false) {
                    throw std::runtime_error ("The constructed boundary is not a contiguous sequence of rects.");
                }
            }
        }

        /*!
         * Set all the outer rects as being "boundary" rects. This makes it possible to
         * create the default/original rectangle of rects, then mark the outer rects as
         * being the boundary.
         *
         * Works only on the initial layout of rects.
         */
        void setBoundaryOnOuterEdge()
        {
            // From centre head to boundary, then mark boundary and walk
            // around the edge.
            std::list<morph::Rect>::iterator bpi = this->rects.begin();
            // Head to the south west corner
            while (bpi->has_nw()) { bpi = bpi->nw; }
            while (bpi->has_ns()) { bpi = bpi->ns; }
            bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
            //std::cout << "set flag at start on rect " << bpi->outputCart() << std::endl;

            while (bpi->has_ne()) {
                bpi = bpi->ne;
                bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
                //std::cout << "set flag going E on rect " << bpi->outputCart() << std::endl;
            }
            while (bpi->has_nn()) {
                bpi = bpi->nn;
                bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
                //std::cout << "set flag going N on rect " << bpi->outputCart() << std::endl;
            }
            while (bpi->has_nw()) {
                bpi = bpi->nw;
                bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
                //std::cout << "set flag going W on rect " << bpi->outputCart() << std::endl;
            }
            while (bpi->has_ns() && bpi->ns->testFlags(RECT_IS_BOUNDARY) == false) {
                bpi = bpi->ns;
                bpi->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
                //std::cout << "set flag going S on rect " << bpi->outputCart() << std::endl;
            }

            // Check that the boundary is contiguous, starting from SW corner and
            // heading E (to go anticlockwise)
            std::set<unsigned int> seen;
            std::list<morph::Rect>::iterator ri = bpi;
            if (this->boundaryContiguous (bpi, ri, seen, RECT_NEIGHBOUR_POS_E) == false) {
                throw std::runtime_error ("The boundary is not a contiguous sequence of rects.");
            }

            if (this->domainShape == morph::CartDomainShape::Boundary) {
                // Boundary IS contiguous, discard rects outside the boundary.
                this->discardOutsideBoundary();
            }

            this->populate_d_vectors();
        }

        /*!
         * Get all the boundary rects in a list. This assumes that a boundary has
         * already been set with one of the setBoundary() methods and so there is
         * therefore a set of Rects which are already marked as being on the boundary
         * (with the attribute Rect::boundaryRect == true) Do this by going around the
         * boundary neighbour to neighbour?
         *
         * Now a getter for this->brects.
         */
        std::list<Rect> getBoundary() const
        {
            std::list<morph::Rect> brects_concrete;
            auto rr = this->brects.begin();
            while (rr != this->brects.end()) {
                brects_concrete.push_back (*(*rr));
                ++rr;
            }
            return brects_concrete;
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
            // half of the rect to rect spacing as the angular step size. Return as bpoints.
            std::vector<morph::BezCoord<float>> bpoints;

            // Estimate a good delta_phi based on the larger of a and b. Compute the delta_phi
            // required to travel a fraction of one rect-to-rect distance.
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

        //! calculate perimeter of ellipse with radii \a a and \a b
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
         * \brief Accessor for the size of rects.
         *
         * return The number of rects in the grid.
         */
        unsigned int num() const { return this->rects.size(); }

        /*!
         * \brief Obtain the vector index of the last Rect in rects.
         *
         * return Rect::vi from the last Rect in the grid.
         */
        unsigned int lastVectorIndex() const { return this->rects.rbegin()->vi; }

        /*!
         * Output some text information about the hexgrid.
         */
        std::string output() const
        {
            std::stringstream ss;
            ss << "Rect grid with " << this->rects.size() << " rects:\n";
            auto i = this->rects.begin();
            while (i != this->rects.end()) {
                ss << i->output() << std::endl;
                ++i;
            }
            return ss.str();
        }

        /*!
         * Show the coordinates of the vertices of the overall rect grid generated.
         */
        std::string extent() const
        {
            std::stringstream ss;
            if (gridReduced == false) {
                ss << "Grid vertices: \n"
                   << "      NW: (" << this->vertexNW->x << "," << this->vertexNW->y << ") "
                   << "      NE: (" << this->vertexNE->x << "," << this->vertexNE->y << ")\n"
                   << "      SW: (" << this->vertexSW->x << "," << this->vertexSW->y << ") "
                   << "      SE: (" << this->vertexSE->x << "," << this->vertexSE->y << ")";
            } else {
                ss << "Initial grid vertices are no longer valid.";
            }
            return ss.str();
        }

        /*!
         * Returns the width of the CartGrid (from -x to +x)
         */
        float width() const
        {
            // {ximin, ximax, yimin, yimax}
            std::array<int, 4> extents = this->findBoundaryExtents();
            float xmin = this->d * float(extents[0]);
            float xmax = this->d * float(extents[1]);
            return (xmax - xmin);
        }

        //! Return the number of elements that the CartGrid is wide
        int widthnum() const
        {
            // {ximin, ximax, yimin, yimax}
            std::array<int, 4> extents = this->findBoundaryExtents();
            int wn = std::abs(extents[0]) + std::abs(extents[1]) + 1;
            return wn;
        }

        /*!
         * Returns the 'depth' of the CartGrid (from -y to +y)
         */
        float depth() const
        {
            std::array<int, 4> extents = this->findBoundaryExtents();
            float ymin = this->v * float(extents[2]);
            float ymax = this->v * float(extents[3]);
            return (ymax - ymin);
        }

        //! Return the number of elements that the CartGrid is deep (or high) - y
        int depthnum() const
        {
            // {ximin, ximax, yimin, yimax}
            std::array<int, 4> extents = this->findBoundaryExtents();
            int dn = std::abs(extents[2]) + std::abs(extents[3]) + 1;
            return dn;
        }

        /*!
         * Getter for d.
         */
        float getd() const { return this->d; }

        /*!
         * Getter for v - vertical rect spacing.
         */
        float getv() const { return this->v; }

        /*!
         * Get the shortest distance from the centre to the perimeter. This is the
         * "short radius".
         */
        float getSR() const { return this->d/2; }

        /*!
         * The distance from the centre of the Rect to any of the vertices. This is the
         * "long radius".
         */
        float getLR() const { return 0.5f * std::sqrt (this->d*this->d + this->v*this->v); }

        /*!
         * The vertical distance from the centre of the rect to the "north east" vertex
         * of the rect.
         */
        float getVtoNE() const { return (0.5f * this->v); }

        /*!
         * Compute and return the area of one rect in the grid.
         */
        float getRectArea() const { return (this->d * this->v); }

        /*!
         * Find the minimum value of x' on the CartGrid, where x' is the x axis rotated
         * by phi degrees.
         */
        float getXmin (float phi = 0.0f) const
        {
            float xmin = 0.0f;
            float x_ = 0.0f;
            bool first = true;
            for (auto r : this->rects) {
                x_ = r.x * std::cos (phi) + r.y * std::sin (phi);
                if (first) {
                    xmin = x_;
                    first = false;
                }
                xmin = x_ < xmin ? x_ : xmin;
            }
            return xmin;
        }

        /*!
         * Find the maximum value of x' on the CartGrid, where x' is the x axis rotated
         * by phi degrees.
         */
        float getXmax (float phi = 0.0f) const
        {
            float xmax = 0.0f;
            float x_ = 0.0f;
            bool first = true;
            for (auto r : this->rects) {
                x_ = r.x * std::cos (phi) + r.y * std::sin (phi);
                if (first) {
                    xmax = x_;
                    first = false;
                }
                xmax = x_ > xmax ? x_ : xmax;
            }
            return xmax;
        }

        /*!
         * Run through all the rects and compute the distance to the nearest boundary
         * rect.
         */
        void computeDistanceToBoundary()
        {
            std::list<morph::Rect>::iterator r = this->rects.begin();
            while (r != this->rects.end()) {
                if (r->testFlags(RECT_IS_BOUNDARY) == true) {
                    r->distToBoundary = 0.0f;
                } else {
                    if (r->testFlags(RECT_INSIDE_BOUNDARY) == false) {
                        // Set to a dummy, negative value
                        r->distToBoundary = -100.0;
                    } else {
                        // Not a boundary rect, but inside boundary
                        std::list<morph::Rect>::iterator br = this->rects.begin();
                        while (br != this->rects.end()) {
                            if (br->testFlags(RECT_IS_BOUNDARY) == true) {
                                float delta = r->distanceFrom (*br);
                                if (delta < r->distToBoundary || r->distToBoundary < 0.0f) {
                                    r->distToBoundary = delta;
                                }
                            }
                            ++br;
                        }
                    }
                }
                ++r;
            }
        }

        /*!
         * Populate d_ vectors. simple version. (Finds extents, then calls
         * populate_d_vectors(const array<int, 4>&)
         */
        void populate_d_vectors()
        {
            std::array<int, 4> extnts = this->findBoundaryExtents();
            this->populate_d_vectors (extnts);
        }

        /*!
         * Populate d_ vectors, paying attention to domainShape.
         */
        void populate_d_vectors (const std::array<int, 4>& extnts)
        {
            // A rectangle iterator
            std::list<morph::Rect>::iterator ri = this->rects.begin();
            // Bottom left rectangle
            std::list<morph::Rect>::iterator blr = this->rects.end();

            if (this->domainShape == morph::CartDomainShape::Rectangle) {

                // Use neighbour relations to go from bottom left to top right.  Find rect on bottom row.
                while (ri != this->rects.end()) {
                    if (ri->yi == extnts[2]) {
                        // std::cout << "We're on the bottom row\n";
                        break;
                    }
                    ++ri;
                }
                // ri now on bottom row; so travel west
                while (ri->has_nw() == true) { ri = ri->nw; }

                // ri should now be the bottom left rect.
                blr = ri;

            } // else Hexagon or Boundary starts from 0, hi already set to rects.begin();

            // Clear the d_ vectors.
            this->d_clear();

            // Now raster through the rects, building the d_ vectors.
            if (this->domainShape == morph::CartDomainShape::Rectangle) {

                this->d_push_back (ri);

                do {
                    if (ri->has_ne() == false) {
                        if (ri->yi == extnts[3]) {
                            // last (i.e. top) row and no neighbour east, so finished.
                            break;
                        } else {
                            // Carriage return
                            ri = blr;
                            // Line feed (Move to the next row up)
                            blr = ri->nn;
                            ri = blr;
                            this->d_push_back (ri);
                        }
                    } else {
                        ri = ri->ne;
                        this->d_push_back (ri);
                    }
                } while (ri->has_ne() == true || ri->has_nn() == true);

            } else { // Boundary

                while (ri != this->rects.end()) {
                    this->d_push_back (ri);
                    ri++;
                }
            }

            this->populate_d_neighbours();
        }

        /*!
         * Get a vector of Rect pointers for all rects that are inside/on the path
         * defined by the BezCurvePath \a p, thus this gets a 'region of rects'. The Rect
         * flags "region" and "regionBoundary" are used, temporarily to mark out the
         * region. The idea is that client code will then use the vector of morph::Rect* to work
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
         * \return a vector of iterators to the Rects that make up the region.
         */
        std::vector<std::list<Rect>::iterator> getRegion (BezCurvePath<float>& p,
                                                          std::pair<float, float>& regionCentroid,
                                                          bool applyOriginalBoundaryCentroid = true)
        {
            p.computePoints (this->d/2.0f, true);
            std::vector<morph::BezCoord<float>> bpoints = p.getPoints();
            return this->getRegion (bpoints, regionCentroid, applyOriginalBoundaryCentroid);
        }

        /*!
         * The overload of getRegion that does all the work on a vector of coordinates
         */
        std::vector<std::list<Rect>::iterator> getRegion (std::vector<BezCoord<float>>& bpoints,
                                                          std::pair<float, float>& regionCentroid,
                                                          bool applyOriginalBoundaryCentroid = true)
        {
            // First clear all region boundary flags, as we'll be defining a new region boundary
            this->clearRegionBoundaryFlags();

            // Compute region centroid from bpoints
            regionCentroid = morph::BezCurvePath<float>::getCentroid (bpoints);

            // A return object
            std::vector<std::list<morph::Rect>::iterator> theRegion;

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

            // Now find the rects on the boundary of the region
            std::list<morph::Rect>::iterator nearbyRegionBoundaryPoint = this->rects.begin(); // i.e the Rect at 0,0
            typename std::vector<morph::BezCoord<float>>::iterator bpi = bpoints.begin();
            while (bpi != bpoints.end()) {
                nearbyRegionBoundaryPoint = this->setRegionBoundary (*bpi++, nearbyRegionBoundaryPoint);
            }

            // Check that the region boundary is contiguous.
            {
                std::set<unsigned int> seen;
                std::list<morph::Rect>::iterator hi = nearbyRegionBoundaryPoint;
                if (this->regionBoundaryContiguous (nearbyRegionBoundaryPoint, hi, seen) == false) {
                    std::stringstream ee;
                    ee << "The constructed region boundary is not a contiguous sequence of rects.";
                    return theRegion;
                }
            }

            // Mark rects inside region. Use centroid of the region.
            std::list<morph::Rect>::iterator insideRegionRect = this->findRectNearest (regionCentroid);
            this->markRectsInside (insideRegionRect, RECT_IS_REGION_BOUNDARY, RECT_INSIDE_REGION);

            // Populate theRegion, then return it
            std::list<morph::Rect>::iterator hi = this->rects.begin();
            while (hi != this->rects.end()) {
                if (hi->testFlags (RECT_INSIDE_REGION) == true) {
                    theRegion.push_back (hi);
                }
                ++hi;
            }

            return theRegion;
        }

        //! Get all the (x,y,z) coordinates from the grid and return as vector of Vectors
        std::vector<morph::Vector<float, 3>> getCoordinates3()
        {
            std::vector<morph::Vector<float, 3>> coords (this->num());
            for (unsigned int i = 0; i < this->num(); ++i) {
                coords[i] = { this->d_x[i], this->d_y[i], this->z };
            }
            return coords;
        }

        //! Get all the (x,y) coordinates from the grid and return as vector of Vectors
        std::vector<morph::Vector<float, 2>> getCoordinates2()
        {
            std::vector<morph::Vector<float, 2>> coords (this->num());
            for (unsigned int i = 0; i < this->num(); ++i) {
                coords[i] = { this->d_x[i], this->d_y[i] };
            }
            return coords;
        }

        /*!
         * For every rect in rects, unset the flags RECT_IS_REGION_BOUNDARY and
         * RECT_INSIDE_REGION
         */
        void clearRegionBoundaryFlags()
        {
            for (auto& rr : this->rects) {
                rr.unsetFlag (RECT_IS_REGION_BOUNDARY | RECT_INSIDE_REGION);
            }
        }

        /*!
         * Using this CartGrid as the domain, convolve the domain data \a data with the
         * kernel data \a kerneldata, which exists on another CartGrid, \a
         * kernelgrid. Return the result in \a result.
         */
        template<typename T>
        void convolve (const CartGrid& kernelgrid, const std::vector<T>& kerneldata, const std::vector<T>& data, std::vector<T>& result)
        {
            if (result.size() != this->rects.size()) {
                throw std::runtime_error ("The result vector is not the same size as the CartGrid.");
            }
            if (result.size() != data.size()) {
                throw std::runtime_error ("The data vector is not the same size as the CartGrid.");
            }
            // Could relax this test...
            if (kernelgrid.getd() != this->d) {
                throw std::runtime_error ("The kernel CartGrid must have same d as this CartGrid to carry out convolution.");
            }
            if (&data == &result) {
                throw std::runtime_error ("Pass in separate memory for the result.");
            }

            // For each rect in this CartGrid, compute the convolution kernel
            std::list<Rect>::iterator ri = this->rects.begin();

            for (; ri != this->rects.end(); ++ri) {
                T sum = T{0};
                // For each kernel rect, sum up.
                for (auto kr : kernelgrid.rects) {
                    std::list<Rect>::iterator dri = ri;
                    int xx = kr.xi;
                    int yy = kr.yi;
                    bool failed = false;
                    bool finished = false;

                    while (!finished) {
                        bool moved = false;
                        // Try to move in x direction
                        if (xx > 0) { // Then kernel rect is to right of 0, so relevant rect on Cartgrid is to east
                            if (dri->has_ne()) {
                                dri = dri->ne;
                                --xx;
                                moved = true;
                            } // Didn't move in +x direction
                        } else if (xx < 0) {
                            if (dri->has_nw()) {
                                dri = dri->nw;
                                ++xx;
                                moved = true;
                            } // Didn't move in -x direction
                        }
                        // Try to move in y direction
                        if (yy > 0) {
                            if (dri->has_nn()) {
                                dri = dri->nn;
                                --yy;
                                moved = true;
                            } // Didn't move in +y direction
                        } else if (yy < 0) {
                            if (dri->has_ns()) {
                                dri = dri->ns;
                                ++yy;
                                moved = true;
                            } // Didn't move in -y direction
                        }

                        if (xx == 0 && yy == 0) {
                            finished = true;
                            break;
                        }

                        if (!moved) {
                            // We're stuck; Can't move in x or y direction, so can't add a contribution
                            failed = true;
                            break;
                        }
                    }

                    if (!failed) {
                        // Can do the sum
                        sum +=  data[dri->vi] * kerneldata[kr.vi];
                    }
                }

                result[ri->vi] = sum;
            }
        }

        /*!
         * What shape domain to set? Set this to the non-default BEFORE calling
         * CartGrid::setBoundary (const BezCurvePath& p) - that's where the domainShape
         * is applied.
         */
        CartDomainShape domainShape = CartDomainShape::Rectangle;

        /*!
         * The list of rects that make up this CartGrid.
         */
        std::list<Rect> rects;

        /*!
         * Once boundary secured, fill this vector. Experimental - can I do parallel
         * loops with vectors of rects? Ans: Not very well.
         */
        std::vector<Rect*> vrects;

        /*!
         * While determining if boundary is continuous, fill this maps container of
         * rects.
         */
        std::list<const Rect*> brects; // Not better as a separate list<Rect>?

        /*!
         * Store the centroid of the boundary path. The centroid of a read-in
         * BezCurvePath [see void setBoundary (const BezCurvePath& p)] is subtracted
         * from each generated point on the boundary path so that the boundary once it
         * is expressed in the CartGrid will have a (2D) centroid of roughly
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
         * Initialise a grid of rects in a raster fashion, setting neighbours as we
         * go. This method populates rects based on the grid parameters set in d, v and
         * x_span.
         */
        void init()
        {
            // Use x_span to determine how many cols
            float halfX = this->x_span/2.0f;
            int halfCols = std::abs(std::ceil(halfX/this->d));
            // Use y_span to determine how many rows
            float halfY = this->y_span/2.0f;
            int halfRows = std::abs(std::ceil(halfY/this->v));

            // The "vector iterator" - this is an identity iterator that is added to each Rect in the grid.
            unsigned int vi = 0;

            std::vector<std::list<morph::Rect>::iterator> prevRowEven;
            std::vector<std::list<morph::Rect>::iterator> prevRowOdd;

            // Swap pointers between rows.
            std::vector<std::list<morph::Rect>::iterator>* prevRow = &prevRowEven;
            std::vector<std::list<morph::Rect>::iterator>* nextPrevRow = &prevRowOdd;

            // Build grid, raster style.
            for (int yi = -halfCols; yi <= halfCols; ++yi) {
                size_t pri = 0;
                for (int xi = -halfRows; xi <= halfRows; ++xi) {
                    this->rects.emplace_back (vi++, this->d, this->v, xi, yi);

                    auto ri = this->rects.end(); ri--;
                    this->vrects.push_back (&(*ri));

                    //std::cout << "emplaced Rect " << ri->outputCart() << std::endl;
                    if (xi > -halfRows) {
                        auto ri_w = ri; ri_w--;
                        ri_w->set_ne (ri);
                        ri->set_nw (ri_w);
                    }
                    if (yi > -halfCols) {
                        //std::cout << "For (xi,yi) = (" << xi << "," << yi << ") set Rect (*prevRow)[" << pri << "]"
                        //          << (*prevRow)[pri]->outputCart() << " as S of Rect ri = " << ri->outputCart() << std::endl;
                        (*prevRow)[pri]->set_nn (ri);
                        ri->set_ns ((*prevRow)[pri]);
                        if (xi > -halfRows) {
                            //std::cout << "For (xi,yi) = (" << xi << "," << yi << ") set Rect (*prevRow)[" << (pri-1) << "]"
                            //          << (*prevRow)[pri-1]->outputCart() << " as SW of Rect ri = " << ri->outputCart() << std::endl;
                            (*prevRow)[pri-1]->set_nne (ri);
                            ri->set_nsw ((*prevRow)[pri-1]);
                        }
                        if (xi < halfRows) {
                            (*prevRow)[pri+1]->set_nnw (ri);
                            ri->set_nse ((*prevRow)[pri+1]);
                        }
                    }
                    ++pri;
                    nextPrevRow->push_back (ri);
                }
                // Swap prevRow and nextPrevRow.
                std::vector<std::list<morph::Rect>::iterator>* tmp = prevRow;
                prevRow = nextPrevRow;
                nextPrevRow = tmp;
                nextPrevRow->clear();
            }
        }

        //! Initialize a non-symmetric rectangular grid.
        void init2 (float x1, float y1, float x2, float y2)
        {
            int _xi = std::round(x1/this->d);
            int _xf = std::round(x2/this->d);
            int _yi = std::round(y1/this->v);
            int _yf = std::round(y2/this->v);

            std::cout << "xi to xf: "<< _xi << " to " << _xf << std::endl;

            // The "vector iterator" - this is an identity iterator that is added to each Rect in the grid.
            unsigned int vi = 0;

            std::vector<std::list<morph::Rect>::iterator> prevRowEven;
            std::vector<std::list<morph::Rect>::iterator> prevRowOdd;

            // Swap pointers between rows.
            std::vector<std::list<morph::Rect>::iterator>* prevRow = &prevRowEven;
            std::vector<std::list<morph::Rect>::iterator>* nextPrevRow = &prevRowOdd;

            // Build grid, raster style.
            for (int yi = _yi; yi <= _yf; ++yi) {
                size_t pri = 0;
                for (int xi = _xi; xi <= _xf; ++xi) {
                    this->rects.emplace_back (vi++, this->d, this->v, xi, yi);

                    auto ri = this->rects.end(); ri--;
                    this->vrects.push_back (&(*ri));

                    if (xi > _xi) {
                        auto ri_w = ri; ri_w--;
                        ri_w->set_ne (ri);
                        ri->set_nw (ri_w);
                    }
                    if (yi > _yi) {
                        (*prevRow)[pri]->set_nn (ri);
                        ri->set_ns ((*prevRow)[pri]);
                        if (xi > _xi) {
                            (*prevRow)[pri-1]->set_nne (ri);
                            ri->set_nsw ((*prevRow)[pri-1]);
                        }
                        if (xi < _xf) {
                            (*prevRow)[pri+1]->set_nnw (ri);
                            ri->set_nse ((*prevRow)[pri+1]);
                        }
                    }
                    ++pri;
                    nextPrevRow->push_back (ri);
                }
                // Swap prevRow and nextPrevRow.
                std::vector<std::list<morph::Rect>::iterator>* tmp = prevRow;
                prevRow = nextPrevRow;
                nextPrevRow = tmp;
                nextPrevRow->clear();
            }
        }

        /*!
         * Starting from \a startFrom, and following nearest-neighbour relations, find
         * the closest Rect in rects to the coordinate point \a point, and set its
         * Rect::onBoundary attribute to true.
         *
         * \return An iterator into CartGrid::rects which refers to the closest Rect to \a point.
         */
        std::list<morph::Rect>::iterator setBoundary (const morph::BezCoord<float>& point,
                                                     std::list<morph::Rect>::iterator startFrom)
        {
            std::list<morph::Rect>::iterator h = this->findRectNearPoint (point, startFrom);
            h->setFlag (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY);
            return h;
        }

        /*!
         * Determine whether the boundary is contiguous. Whilst doing so, populate a
         * list<Rect> containing just the boundary Rectes.
         */
        bool boundaryContiguous()
        {
            this->brects.clear();
            std::list<morph::Rect>::const_iterator bhi = this->rects.begin();
            if (this->findBoundaryRect (bhi) == false) {
                // Found no boundary rect
                return false;
            }
            std::set<unsigned int> seen;
            std::list<morph::Rect>::const_iterator hi = bhi;
            return this->boundaryContiguous (bhi, hi, seen, RECT_NEIGHBOUR_POS_E);
        }

        /*!
         * Determine whether the boundary is contiguous, starting from the boundary
         * Rect iterator \a bhi.
         *
         * If following the boundary clockwise, then need to search neighbours "starting
         * from the one that's clockwise around from the last one that was on the
         * boundary"
         *
         * The overload with brects takes a list of Rect pointers and populates it with
         * pointers to the rects on the boundary.
         */
        bool boundaryContiguous (std::list<Rect>::const_iterator bri,
                                 std::list<Rect>::const_iterator ri, std::set<unsigned int>& seen, int dirn)
        {
            bool rtn = false;
            std::list<morph::Rect>::const_iterator ri_next;
            seen.insert (ri->vi);
            // Insert into the std::list of Rect pointers, too
            this->brects.push_back (&(*ri));

            // increasing direction is an anticlockwise sense
            for (int i = 0; i < 8; ++i) {
                if (rtn == false
                    && ri->has_neighbour(dirn+i)
                    && ri->get_neighbour(dirn+i)->testFlags(RECT_IS_BOUNDARY) == true
                    && seen.find(ri->get_neighbour(dirn+i)->vi) == seen.end()) {
                    //std::cout << ri->neighbour_pos(dirn+i) << std::endl;
                    ri_next = ri->get_neighbour(dirn+i);
                    rtn = (this->boundaryContiguous (bri, ri_next, seen, dirn+i));
                }
            }

            if (rtn == false) {
                // Checked all neighbours
                if (ri == bri) {
                    // Back at start, nowhere left to go! return true.
                    rtn = true;
                }
            }

            return rtn;
        }

        /*!
         * Set the rect closest to point as being on the region boundary. Region
         * boundaries are supposed to be temporary, so that client code can find a
         * region, extract the pointers to all the Rects in that region and store that
         * information for later use.
         */
        std::list<Rect>::iterator setRegionBoundary (const BezCoord<float>& point, std::list<Rect>::iterator startFrom)
        {
            std::list<morph::Rect>::iterator h = this->findRectNearPoint (point, startFrom);
            h->setFlag (RECT_IS_REGION_BOUNDARY | RECT_INSIDE_REGION);
            return h;
        }

        /*!
         * Determine whether the region boundary is contiguous, starting from the
         * boundary Rect iterator #bhi.
         */
        bool regionBoundaryContiguous (std::list<Rect>::const_iterator bhi,
                                       std::list<Rect>::const_iterator hi, std::set<unsigned int>& seen)
        {
            bool rtn = false;
            std::list<morph::Rect>::const_iterator hi_next;
            seen.insert (hi->vi);
            // Insert into the list of Rect pointers, too
            this->brects.push_back (&(*hi));

            if (rtn == false && hi->has_ne() && hi->ne->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->ne->vi) == seen.end()) {
                hi_next = hi->ne;
                rtn = (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nne() && hi->nne->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->nne->vi) == seen.end()) {
                hi_next = hi->nne;
                rtn = this->regionBoundaryContiguous (bhi, hi_next, seen);
            }
            if (rtn == false && hi->has_nnw() && hi->nnw->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->nnw->vi) == seen.end()) {
                hi_next = hi->nnw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nw() && hi->nw->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->nw->vi) == seen.end()) {
                hi_next = hi->nw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nsw() && hi->nsw->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->nsw->vi) == seen.end()) {
                hi_next = hi->nsw;
                rtn =  (this->regionBoundaryContiguous (bhi, hi_next, seen));
            }
            if (rtn == false && hi->has_nse() && hi->nse->testFlags(RECT_IS_REGION_BOUNDARY) == true && seen.find(hi->nse->vi) == seen.end()) {
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
         * Find a rect, any rect, that's on the boundary specified by #boundary. This
         * assumes that setBoundary (const BezCurvePath&) has been called to mark the
         * Rects that lie on the boundary.
         */
        bool findBoundaryRect (std::list<Rect>::const_iterator& ri) const
        {
            if (ri->testFlags(RECT_IS_BOUNDARY) == true) {
                // No need to change the Rect iterator
                return true;
            }

            // On a Cartesian grid should be able to simply go south until we hit a boundary rect
            if (ri->has_ns()) {
                std::list<morph::Rect>::const_iterator ci(ri->ns);
                if (this->findBoundaryRect (ci) == true) {
                    ri = ci;
                    return true;
                }
            }

            return false;
        }

        /*!
         * Find the rect near @point, starting from startFrom, which should be as close
         * as possible to point in order to reduce computation time.
         */
        std::list<Rect>::iterator findRectNearPoint (const BezCoord<float>& point, std::list<Rect>::iterator startFrom)
        {
            bool neighbourNearer = true;

            std::list<morph::Rect>::iterator h = startFrom;
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
         * Mark rects as being inside the boundary given that \a hi refers to a boundary
         * Rect and at least one adjacent rect to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * rect is the inside)
         *
         * \param hi list iterator to starting Rect.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (std::list<Rect>::iterator hi,
                               unsigned int bdryFlag = RECT_IS_BOUNDARY,
                               unsigned int insideFlag = RECT_INSIDE_BOUNDARY)
        {
            this->markFromBoundary (&(*hi), bdryFlag, insideFlag);
        }

        /*!
         * Mark rects as being inside the boundary given that \a hi refers to a boundary
         * Rect and at least one adjacent rect to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * rect is the inside)
         *
         * \param hi list iterator to a pointer to the starting Rect.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (std::list<Rect*>::iterator hi,
                               unsigned int bdryFlag = RECT_IS_BOUNDARY,
                               unsigned int insideFlag = RECT_INSIDE_BOUNDARY)
        {
            this->markFromBoundary ((*hi), bdryFlag, insideFlag);
        }

        /*!
         * Mark rects as being inside the boundary given that \a hi refers to a boundary
         * Rect and at least one adjacent rect to \a hi has already been marked as inside
         * the boundary (thus allowing the algorithm to know which side of the boundary
         * rect is the inside)
         *
         * \param hi pointer to the starting Rect.
         *
         * By changing \a bdryFlag and \a insideFlag, it's possible to use this method
         * with region boundaries.
         */
        void markFromBoundary (morph::Rect* hi,
                               unsigned int bdryFlag = RECT_IS_BOUNDARY,
                               unsigned int insideFlag = RECT_INSIDE_BOUNDARY)
        {
            // Find a marked-inside Rect next to this boundary rect. This will be the first direction to mark
            // a line of inside rects in.
            std::list<morph::Rect>::iterator first_inside = this->rects.begin();
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

            // For each other direction also mark lines. Count direction upwards until we hit a boundary rect:
            short diri = (firsti + 1) % 6;
            // Can debug first *count up* direction with morph::Rect::neighbour_pos(diri)
            while (hi->has_neighbour(diri) && hi->get_neighbour(diri)->testFlags(bdryFlag)==false && diri != firsti) {
                first_inside = hi->get_neighbour(diri);
                this->markFromBoundaryCommon (first_inside, diri, bdryFlag, insideFlag);
                diri = (diri + 1) % 6;
            }

            // Then count downwards until we hit the other boundary rect
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
        void markFromBoundaryCommon (std::list<Rect>::iterator first_inside, unsigned short firsti,
                                     unsigned int bdryFlag = RECT_IS_BOUNDARY,
                                     unsigned int insideFlag = RECT_INSIDE_BOUNDARY)
        {
            // From the "first inside the boundary rect" head in the direction specified by firsti until a
            // boundary rect is reached.
            std::list<morph::Rect>::iterator straight = first_inside;

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
                                      << ") without encountering a boundary Rect.\n";
                            warning_given = true;
                        }
#endif
                        break;
                    }
                }
            }
        }

        /*!
         * Given the current boundary rect iterator, bhi and the n_recents last boundary
         * rects in recently_seen, and assuming that bhi has had all its adjacent inside
         * rects marked as insideBoundary, find the next boundary rect.
         *
         * \param bhi The boundary rect iterator. From this rect, find the next boundary
         * rect.
         *
         * \param recently_seen a deque containing the recently processed boundary
         * rects. for a boundary which is always exactly one rect thick, you only need a
         * memory of the last boundary rect to keep you going in the right direction
         * around the boundary BUT if your boundary has some "double thickness"
         * sections, then you need to know a few more recent rects to avoid looping
         * around and returning to the start!
         *
         * \param n_recents The number of rects to record in \a recently_seen. The
         * actual number you will need depends on the "thickness" of your boundary -
         * does it have sections that are two rects thick, or sections that are six
         * rects thick? It also depends on the length along which the boundary may be
         * two rects thick. In theory, if you have a boundary section two rects thick
         * for 5 pairs, then you need to store 10 previous rects. However, due to the
         * way that this algorithm tests rects (always testing direction '0' which is
         * East first, then going anti-clockwise to the next direction; North-East and
         * so on), n_recents=2 appears to be sufficient for a thickness 2 boundary,
         * which is what can occur when setting a boundary using the method
         * CartGrid::setEllipticalBoundary. Boundaries that are more than thickness 2
         * shouldn't really occur, whereas a boundary with a short section of thickness
         * 2 can quite easily occur, as in setEllipticalBoundary, where insisting that
         * the boundary was strictly always only 1 rect thick would make that algorithm
         * more complex.
         *
         * \param bdryFlag The flag used to recognise a boundary rect.
         *
         * \param insideFlag The flag used to recognise a rect that is inside the
         * boundary.
         *
         * \return true if a next boundary neighbour was found, false otherwise.
         */
        bool findNextBoundaryNeighbour (std::list<Rect>::iterator& bhi,
                                        std::deque<std::list<Rect>::iterator>& recently_seen,
                                        size_t n_recents = 2,
                                        unsigned int bdryFlag = RECT_IS_BOUNDARY,
                                        unsigned int insideFlag = RECT_INSIDE_BOUNDARY) const
        {
            bool gotnextneighbour = false;

            // From each boundary rect, loop round all 6 neighbours until we get to a new neighbour
            for (unsigned short i = 0; i < 6 && gotnextneighbour == false; ++i) {

                // This is "if it's a neighbour and the neighbour is a boundary rect"
                if (bhi->has_neighbour(i) && bhi->get_neighbour(i)->testFlags(bdryFlag)) {

                    // cbhi is "candidate boundary rect iterator", now guaranteed to be a boundary rect
                    std::list<morph::Rect>::iterator cbhi = bhi->get_neighbour(i);

                    // Test if the candidate boundary rect is in the 'recently seen' deque
                    bool rect_already_seen = false;
                    for (auto rs : recently_seen) {
                        if (rs == cbhi) {
                            // This candidate rect has been recently seen. continue to next i
                            rect_already_seen = true;
                        }
                    }
                    if (rect_already_seen) { continue; }

                    unsigned short i_opp = ((i+3)%6);

                    // Go round each of the candidate boundary rect's neighbours (but j!=i)
                    for (unsigned short j = 0; j < 6; ++j) {

                        // Ignore the candidate boundary rect itself. if j==i_opp, then
                        // i's neighbour in dirn morph::Rect::neighbour_pos(j) is the
                        // candidate iself, continue to next i
                        if (j==i_opp) { continue; }

                        // What is this logic. If the candidate boundary rect (which is already
                        // known to be on the boundary) has a neighbour which is inside the
                        // boundary and not itself a boundary rect, then cbhi IS the next
                        // boundary rect.
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
         * Mark rects as insideBoundary if they are inside the boundary. Starts from
         * \a hi which is assumed to already be known to refer to a rect lying inside the
         * boundary.
         */
        void markRectsInside (std::list<Rect>::iterator hi,
                              unsigned int bdryFlag = RECT_IS_BOUNDARY,
                              unsigned int insideFlag = RECT_INSIDE_BOUNDARY)
        {
            // Run to boundary, marking as we go
            std::list<morph::Rect>::iterator bhi(hi);
            while (bhi->testFlags (bdryFlag) == false && bhi->has_nne()) {
                bhi->setFlag (insideFlag);
                bhi = bhi->nne;
            }
            std::list<morph::Rect>::iterator bhi_start = bhi;

            // Mark from first boundary rect and across the region
            this->markFromBoundary (bhi, bdryFlag, insideFlag);

            // a deque to hold the 'n_recents' most recently seen boundary rects.
            std::deque<std::list<morph::Rect>::iterator> recently_seen;
            size_t n_recents = 16; // 2 should be sufficient for boundaries with double thickness
            // sections. If problems occur, trying increasing this.
            bool gotnext = this->findNextBoundaryNeighbour (bhi, recently_seen, n_recents, bdryFlag, insideFlag);
            // Loop around boundary, marking inwards in all possible directions from each boundary rect
            while (gotnext && bhi != bhi_start) {
                this->markFromBoundary (bhi, bdryFlag, insideFlag);
                gotnext = this->findNextBoundaryNeighbour (bhi, recently_seen, n_recents, bdryFlag, insideFlag);
            }
        }

        /*!
         * Mark ALL rects as inside the domain
         */
        void markAllRectsInsideDomain()
        {
            std::list<morph::Rect>::iterator hi = this->rects.begin();
            while (hi != this->rects.end()) {
                hi->setInsideDomain();
                hi++;
            }
        }

        /*!
         * Discard rects in this->rects that are outside the boundary #boundary.
         */
        void discardOutsideBoundary()
        {
            // Mark those rects inside the boundary
            std::list<morph::Rect>::iterator centroidRect = this->findRectNearest (this->boundaryCentroid);
            this->markRectsInside (centroidRect);
            // Run through and discard those rects outside the boundary:
            auto hi = this->rects.begin();
            while (hi != this->rects.end()) {
                if (hi->testFlags(RECT_INSIDE_BOUNDARY) == false) {
                    // When erasing a Rect, I need to update the neighbours of its
                    // neighbours.
                    hi->disconnectNeighbours();
                    // Having disconnected the neighbours, erase the Rect.
                    hi = this->rects.erase (hi);
                } else {
                    ++hi;
                }
            }
            // The Rect::vi indices need to be re-numbered.
            this->renumberVectorIndices();
            // Finally, do something about the rectagonal grid vertices; set this to true to mark that the
            // iterators to the outermost vertices are no longer valid and shouldn't be used.
            this->gridReduced = true;
        }

        /*!
         * Discard rects in this->rects that are outside the rectangular rect domain.
         */
        void discardOutsideDomain()
        {
            // Similar to discardOutsideBoundary:
            auto hi = this->rects.begin();
            while (hi != this->rects.end()) {
                if (hi->insideDomain() == false) {
                    hi->disconnectNeighbours();
                    hi = this->rects.erase (hi);
                } else {
                    ++hi;
                }
            }
            this->renumberVectorIndices();
            this->gridReduced = true;
        }

        /*!
         * Find the extents of the boundary rects. Find the xi for the left-most rect and
         * the xi for the right-most rect (elements 0 and 1 of the return array). Find
         * the yi for the top most rect and the yi for the bottom most rect.
         *
         * Return object contains: {xi-left, xi-right, yi-bottom, yi-top}
         */
        std::array<int, 4> findBoundaryExtents() const
        {
            // Return object contains {ri-left, ri-right, gi-bottom, gi-top, gi at ri-left, gi at ri-right}
            // i.e. {xmin, xmax, ymin, ymax, gi at xmin, gi at xmax}
            std::array<int, 4> extents = {{0,0,0,0}};

            // Find the furthest left and right rects and the furthest up and down rects.
            std::array<float, 4> limits = {{0,0,0,0}};
            bool first = true;
            for (auto r : this->rects) {
                if (r.testFlags(RECT_IS_BOUNDARY) == true) {
                    if (first) {
                        limits = {r.x, r.x, r.y, r.y};
                        extents = {r.xi, r.xi, r.yi, r.yi};
                        first = false;
                    }
                    if (r.x < limits[0]) {
                        limits[0] = r.x;
                        extents[0] = r.xi;
                    }
                    if (r.x > limits[1]) {
                        limits[1] = r.x;
                        extents[1] = r.xi;
                    }
                    if (r.y < limits[2]) {
                        limits[2] = r.y;
                        extents[2] = r.yi;
                    }
                    if (r.y > limits[3]) {
                        limits[3] = r.y;
                        extents[3] = r.yi;
                    }
                }
            }

            // Add 'growth buffer'
            extents[0] -= this->d_growthbuffer_horz;
            extents[1] += this->d_growthbuffer_horz;
            extents[2] -= this->d_growthbuffer_vert;
            extents[3] += this->d_growthbuffer_vert;

            return extents;
        }

        /*!
         * Find the Rect in the Rect grid which is closest to the x,y position given by
         * pos.
         */
        std::list<Rect>::iterator findRectNearest (const std::pair<float, float>& pos)
        {
            std::list<morph::Rect>::iterator nearest = this->rects.end();
            std::list<morph::Rect>::iterator ri = this->rects.begin();
            float dist = std::numeric_limits<float>::max();
            while (ri != this->rects.end()) {
                float dx = pos.first - ri->x;
                float dy = pos.second - ri->y;
                float dl = std::sqrt (dx*dx + dy*dy);
                if (dl < dist) {
                    dist = dl;
                    nearest = ri;
                }
                ++ri;
            }
            return nearest;
        }

        //! Assuming a rectangular CartGrid, find bottom left element
        std::list<Rect>::iterator findBottomLeft()
        {
            std::list<morph::Rect>::iterator bottomleft = this->rects.begin();
            while (bottomleft->has_ns()) { bottomleft = bottomleft->ns; }
            while (bottomleft->has_nw()) { bottomleft = bottomleft->nw; }
            return bottomleft;
        }

        /*!
         * Does what it says on the tin. Re-number the Rect::vi vector index in each
         * Rect in the CartGrid, from the start of the list<Rect> rects until the end.
         */
        void renumberVectorIndices()
        {
            unsigned int vi = 0;
            this->vrects.clear();
            auto ri = this->rects.begin();
            while (ri != this->rects.end()) {
                ri->vi = vi++;
                this->vrects.push_back (&(*ri));
                ++ri;
            }
        }

        //! The centre to centre horizontal distance.
        float d = 1.0f; // refactor dx?

        //! The centre to centre rect vertical distance
        float v = 1.0f; // refactor dy?

        //! Give the initial rectangular grid a size x_span in the horizontal direction.
        float x_span = 10.0f;

        //! Give the initial rectangular grid a size y_span in the vertical direction.
        float y_span = 10.0f;

        //! The z coordinate of this rect grid layer
        float z;

        //! A boundary to apply to the initial, rectangular grid.
        BezCurvePath<float> boundary;

        /*
         * Rect references to the rects on the vertices of the rectagonal
         * grid. Configured during init(). These will become invalid when a new
         * boundary is applied to the original rectagonal grid. When this occurs,
         * gridReduced should be set false.
         */
        std::list<Rect>::iterator vertexNE;
        std::list<Rect>::iterator vertexNW;
        std::list<Rect>::iterator vertexSW;
        std::list<Rect>::iterator vertexSE;

        /*!
         * Set true when a new boundary or domain has been applied. This means that
         * the #vertexNE, #vertexSW, and similar iterators are no longer valid.
         */
        bool gridReduced = false;
    };

} // namespace morph
