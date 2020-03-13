/*!
 * Implementation of HexGrid
 *
 * Author: Seb James
 *
 * Date: 2018/07
 */

#include "HexGrid.h"
#include "MathConst.h"
#include <string>
#include <cmath>
#include <float.h>
#include <limits>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <stdexcept>
#include "BezCurvePath.h"
#include "BezCoord.h"
#include "HdfData.h"

#define DBGSTREAM std::cout
//#define DEBUG 1
//#define DEBUG2 1
#include "MorphDbg.h"

using std::ceil;
using std::abs;
using std::sin;
using std::cos;
using std::atan2;
using std::sqrt;
using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::vector;
using std::set;
using std::runtime_error;
using std::numeric_limits;
using std::to_string;

using morph::BezCurvePath;
using morph::BezCoord;
using morph::Hex;

morph::HexGrid::HexGrid ()
    : d(1.0f)
    , x_span(1.0f)
    , z(0.0f)
{
}

morph::HexGrid::HexGrid (const string& path)
    : d(1.0f)
    , x_span(1.0f)
    , z(0.0f)
{
    this->load (path);
}

morph::HexGrid::HexGrid (float d_, float x_span_, float z_, morph::HexDomainShape shape)
{
    this->d = d_;
    this->v = this->d * SQRT_OF_3_OVER_2_F;
    this->x_span = x_span_;
    this->z = z_;
    this->domainShape = shape;

    this->init();
}

void
morph::HexGrid::load (const string& path)
{
    HdfData hgdata (path, true);
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
        string h5path = "/hexen/" + to_string(i);
        Hex h (hgdata, h5path);
        this->hexen.push_back (h);
    }

    // After creating hexen list, need to set neighbour relations in each Hex, as loaded in d_ne,
    // etc.
    for (Hex& _h : this->hexen) {
        DBG ("Set neighbours for Hex " << _h.outputRG());
        // For each Hex, six loops through hexen:
        if (_h.has_ne() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_ne[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.ne = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour E relation...");
            }
        }

        if (_h.has_nne() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_nne[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.nne = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour NE relation...");
            }
        }

        if (_h.has_nnw() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_nnw[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.nnw = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour NW relation...");
            }
        }

        if (_h.has_nw() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_nw[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.nw = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour W relation...");
            }
        }

        if (_h.has_nsw() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_nsw[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.nsw = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour SW relation...");
            }
        }

        if (_h.has_nse() == true) {
            bool matched = false;
            unsigned int neighb_it = (unsigned int) this->d_nse[_h.vi];
            list<Hex>::iterator hi = this->hexen.begin();
            while (hi != this->hexen.end()) {
                if (hi->vi == neighb_it) {
                    matched = true;
                    _h.nse = hi;
                    break;
                }
                ++hi;
            }
            if (!matched) {
                throw runtime_error ("Failed to match hexen neighbour SE relation...");
            }
        }
    }
}

void
morph::HexGrid::save (const string& path)
{
    HdfData hgdata (path);
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
    list<Hex>::const_iterator h = this->hexen.begin();
    unsigned int hcount = 0;
    while (h != this->hexen.end()) {
        // Make up a path
        string h5path = "/hexen/" + to_string(hcount);
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

pair<float, float>
morph::HexGrid::computeCentroid (const list<Hex>& pHexes)
{
    pair<float, float> centroid;
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

void
morph::HexGrid::setBoundaryOnOuterEdge (void)
{
    // From centre head to boundary, then mark boundary and walk
    // around the edge.
    list<Hex>::iterator bpi = this->hexen.begin();
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
    set<unsigned int> seen;
    list<Hex>::iterator hi = bpi;
    if (this->boundaryContiguous (bpi, hi, seen) == false) {
        stringstream ee;
        ee << "The boundary is not a contiguous sequence of hexes.";
        throw runtime_error (ee.str());
    }

    if (this->domainShape == morph::HexDomainShape::Boundary) {
        // Boundary IS contiguous, discard hexes outside the boundary.
        this->discardOutsideBoundary();
    } else {
        throw runtime_error ("For now, setBoundary (const list<Hex>& pHexes) doesn't know what to do if domain shape is not HexDomainShape::Boundary.");
    }

    this->populate_d_vectors();

}

void
morph::HexGrid::setBoundary (const list<Hex>& pHexes)
{
    this->boundaryCentroid = this->computeCentroid (pHexes);

    list<Hex>::iterator bpoint = this->hexen.begin();
    list<Hex>::iterator bpi = this->hexen.begin();
    while (bpi != this->hexen.end()) {
        list<Hex>::const_iterator ppi = pHexes.begin();
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
    set<unsigned int> seen;
    list<Hex>::iterator hi = bpoint;
    if (this->boundaryContiguous (bpoint, hi, seen) == false) {
        stringstream ee;
        ee << "The boundary is not a contiguous sequence of hexes.";
        throw runtime_error (ee.str());
    }

    if (this->domainShape == morph::HexDomainShape::Boundary) {
        // Boundary IS contiguous, discard hexes outside the boundary.
        this->discardOutsideBoundary();
    } else {
        throw runtime_error ("For now, setBoundary (const list<Hex>& pHexes) doesn't know what to "
                             "do if domain shape is not HexDomainShape::Boundary.");
    }

    this->populate_d_vectors();
}

float
morph::HexGrid::ellipsePerimeter (const float a, const float b)
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

vector<BezCoord<float>>
morph::HexGrid::ellipseCompute (const float a, const float b)
{
    // Compute the points on the boundary using the parametric elliptical formula and
    // half of the hex to hex spacing as the angular step size. Return as bpoints.
    vector<BezCoord<float>> bpoints;

    // Estimate a good delta_phi based on the larger of a and b. Compute the delta_phi
    // required to travel a fraction of one hex-to-hex distance.
    double delta_phi = 0.0;
    double dfraction = this->d / 2.0;
    if (a > b) {
        delta_phi = atan2 (dfraction, a);
    } else {
        delta_phi = atan2 (dfraction, b);
    }

    // Loop around phi, computing x and y of the elliptical boundary and filling up bpoints
    for (double phi = 0.0; phi < morph::TWO_PI_D; phi+=delta_phi) {
        float x_pt = static_cast<float>(a * cos (phi));
        float y_pt = static_cast<float>(b * sin (phi));
        BezCoord<float> b(make_pair(x_pt, y_pt));
        bpoints.push_back (b);
    }

    return bpoints;
}

void
morph::HexGrid::setEllipticalBoundary (const float a, const float b)
{
    vector<BezCoord<float>> bpoints = ellipseCompute (a, b);
    this->setBoundary (bpoints);
}

void
morph::HexGrid::setCircularBoundary (const float a)
{
    vector<BezCoord<float>> bpoints = ellipseCompute (a, a);
    this->setBoundary (bpoints);
}

void
morph::HexGrid::clearRegionBoundaryFlags (void)
{
    for (auto& hh : this->hexen) {
        hh.unsetFlag (HEX_IS_REGION_BOUNDARY | HEX_INSIDE_REGION);
    }
}

vector<list<Hex>::iterator>
morph::HexGrid::getRegion (BezCurvePath<float>& p, pair<float, float>& regionCentroid, bool applyOriginalBoundaryCentroid)
{
    p.computePoints (this->d/2.0f, true);
    vector<BezCoord<float>> bpoints = p.getPoints();
    return this->getRegion (bpoints, regionCentroid, applyOriginalBoundaryCentroid);
}

vector<list<Hex>::iterator>
morph::HexGrid::getRegion (vector<BezCoord<float>>& bpoints, pair<float, float>& regionCentroid, bool applyOriginalBoundaryCentroid)
{
    // First clear all region boundary flags, as we'll be defining a new region boundary
    this->clearRegionBoundaryFlags();

    // Compute region centroid from bpoints
    regionCentroid = BezCurvePath<float>::getCentroid (bpoints);

    // A return object
    vector<list<Hex>::iterator> theRegion;

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
    list<Hex>::iterator nearbyRegionBoundaryPoint = this->hexen.begin(); // i.e the Hex at 0,0
    typename vector<BezCoord<float>>::iterator bpi = bpoints.begin();
    while (bpi != bpoints.end()) {
        nearbyRegionBoundaryPoint = this->setRegionBoundary (*bpi++, nearbyRegionBoundaryPoint);
        DBG2 ("Added region boundary point " << nearbyRegionBoundaryPoint->ri << "," << nearbyRegionBoundaryPoint->gi);
    }

    // Check that the region boundary is contiguous.
    {
        set<unsigned int> seen;
        list<Hex>::iterator hi = nearbyRegionBoundaryPoint;
        if (this->regionBoundaryContiguous (nearbyRegionBoundaryPoint, hi, seen) == false) {
            stringstream ee;
            ee << "The constructed region boundary is not a contiguous sequence of hexes.";
            return theRegion;
        }
    }

    // Mark hexes inside region. Use centroid of the region.
    list<Hex>::iterator insideRegionHex = this->findHexNearest (regionCentroid);
    this->markHexesInside (insideRegionHex, HEX_IS_REGION_BOUNDARY, HEX_INSIDE_REGION);

    // Populate theRegion, then return it
    list<Hex>::iterator hi = this->hexen.begin();
    while (hi != this->hexen.end()) {
        if (hi->testFlags (HEX_INSIDE_REGION) == true) {
            theRegion.push_back (hi);
        }
        ++hi;
    }

    return theRegion;
}

list<Hex>::iterator
morph::HexGrid::setRegionBoundary (const BezCoord<float>& point, list<Hex>::iterator startFrom)
{
    list<Hex>::iterator h = this->findHexNearPoint (point, startFrom);
    h->setFlag (HEX_IS_REGION_BOUNDARY | HEX_INSIDE_REGION);
    return h;
}

bool
morph::HexGrid::regionBoundaryContiguous (list<Hex>::const_iterator bhi, list<Hex>::const_iterator hi, set<unsigned int>& seen)
{
    bool rtn = false;
    list<Hex>::const_iterator hi_next;

    DBG2 ("Inserting " << hi->vi << " into seen (and bhexen) which is Hex (" << hi->ri << "," << hi->gi<<")");
    seen.insert (hi->vi);
    // Insert into the list of Hex pointers, too
    this->bhexen.push_back ((Hex*)&(*hi));

    DBG2 (hi->output());

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
        if (hi == bhi) {
            DBG2 ("Back at start, nowhere left to go! return true.");
            rtn = true;
        } else {
            DBG2 ("Back at hi=(" << hi->ri << "," << hi->gi << "), while bhi=(" <<  bhi->ri << "," << bhi->gi << "), return false");
            rtn = false;
        }
    }

    DBG2 ("Region boundary " << (rtn ? "IS" : "isn't") << " contiguous so far...");

    return rtn;
}

void
morph::HexGrid::setBoundary (const BezCurvePath<float>& p)
{
    this->boundary = p;

    if (!this->boundary.isNull()) {
        DBG ("Applying boundary...");
        // Compute the points on the boundary using half of the hex to hex spacing as the step
        // size. The 'true' argument inverts the y axis.
        this->boundary.computePoints (this->d/2.0f, true);
        vector<BezCoord<float>> bpoints = this->boundary.getPoints();
        this->setBoundary (bpoints);
    }
}

void
morph::HexGrid::setBoundary (vector<BezCoord<float>>& bpoints)
{
    this->boundaryCentroid = BezCurvePath<float>::getCentroid (bpoints);
    DBG ("Boundary centroid: " << boundaryCentroid.first << "," << boundaryCentroid.second);
    auto bpi = bpoints.begin();
    while (bpi != bpoints.end()) {
        bpi->subtract (this->boundaryCentroid);
        ++bpi;
    }
    // Copy the centroid
    this->originalBoundaryCentroid = this->boundaryCentroid;
    // Zero out the centroid, as the boundary is now centred on 0,0
    this->boundaryCentroid = make_pair (0.0, 0.0);

    list<Hex>::iterator nearbyBoundaryPoint = this->hexen.begin(); // i.e the Hex at 0,0
    bpi = bpoints.begin();
    while (bpi != bpoints.end()) {
        nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
        DBG2 ("Added boundary point " << nearbyBoundaryPoint->ri << "," << nearbyBoundaryPoint->gi);
    }

    // Check that the boundary is contiguous.
    {
        set<unsigned int> seen;
        list<Hex>::iterator hi = nearbyBoundaryPoint;
        if (this->boundaryContiguous (nearbyBoundaryPoint, hi, seen) == false) {
            stringstream ee;
            ee << "The constructed boundary is not a contiguous sequence of hexes.";
            throw runtime_error (ee.str());
        }
    }

    if (this->domainShape == morph::HexDomainShape::Boundary) {

        this->discardOutsideBoundary();
        // Now populate the d_ vectors
        this->populate_d_vectors();

    } else {
        // Given that the boundary IS contiguous, can now set a domain of hexes (rectangular,
        // parallelogram or hexagonal region, such that computations can be efficient) and discard
        // hexes outside the domain.  setDomain() will define a regular domain, then discard those
        // hexes outside the regular domain and populate all the d_ vectors.
        this->setDomain();
    }
}

list<Hex>::iterator
morph::HexGrid::setBoundary (const BezCoord<float>& point, list<Hex>::iterator startFrom)
{
    // Searching from "startFrom", search out, via neighbours until the hex closest to the boundary
    // point is located. How to know if it's closest? When all neighbours are further from the
    // currently closest point?
    list<Hex>::iterator h = this->findHexNearPoint (point, startFrom);

    // Mark it for being on the boundary
    h->setFlag (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);

    return h;
}

list<Hex>::iterator
morph::HexGrid::findHexNearPoint (const BezCoord<float>& point, list<Hex>::iterator startFrom)
{
    bool neighbourNearer = true;

    list<Hex>::iterator h = startFrom;
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

    DBG2 ("Nearest hex to point (" << point.x() << "," << point.y() << ") is at (" << h->ri << "," << h->gi << ")");

    return h;
}

list<Hex>
morph::HexGrid::getBoundary (void) const
{
    list<Hex> bhexen_concrete;
    auto hh = this->bhexen.begin();
    while (hh != this->bhexen.end()) {
        bhexen_concrete.push_back (*(*hh));
        ++hh;
    }
    return bhexen_concrete;
}

bool
morph::HexGrid::findBoundaryHex (list<Hex>::const_iterator& hi) const
{
    DBG ("Testing Hex ri,gi = " << hi->ri << "," << hi->gi << " x,y = " << hi->x << "," << hi->y);
    if (hi->testFlags(HEX_IS_BOUNDARY) == true) {
        // No need to change the Hex iterator
        return true;
    }

    if (hi->has_ne()) {
        list<Hex>::const_iterator ci(hi->ne);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nne()) {
        list<Hex>::const_iterator ci(hi->nne);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nnw()) {
        list<Hex>::const_iterator ci(hi->nnw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nw()) {
        list<Hex>::const_iterator ci(hi->nw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nsw()) {
        list<Hex>::const_iterator ci(hi->nsw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nse()) {
        list<Hex>::const_iterator ci(hi->nse);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }

    return false;
}

bool
morph::HexGrid::boundaryContiguous (void)
{
    this->bhexen.clear();

    list<Hex>::const_iterator bhi = this->hexen.begin();
    if (this->findBoundaryHex (bhi) == false) {
        // Found no boundary hex
        return false;
    }
    set<unsigned int> seen;
    list<Hex>::const_iterator hi = bhi;
    return this->boundaryContiguous (bhi, hi, seen);
}

bool
morph::HexGrid::boundaryContiguous (list<Hex>::const_iterator bhi, list<Hex>::const_iterator hi, set<unsigned int>& seen)
{
    DBG2 ("Called for hi=" << hi->vi);
    bool rtn = false;
    list<Hex>::const_iterator hi_next;

    DBG2 ("Inserting " << hi->vi
          << " into seen (and bhexen) which is Hex (" << hi->ri << "," << hi->gi<<")");
    seen.insert (hi->vi);
    // Insert into the list of Hex pointers, too
    this->bhexen.push_back ((Hex*)&(*hi));

    DBG2 (hi->output());

    if (rtn == false && hi->has_ne() && hi->ne->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->ne->vi) == seen.end()) {
        hi_next = hi->ne;
        rtn = (this->boundaryContiguous (bhi, hi_next, seen));
    }
    if (rtn == false && hi->has_nne() && hi->nne->testFlags(HEX_IS_BOUNDARY) == true && seen.find(hi->nne->vi) == seen.end()) {
        hi_next = hi->nne;
        rtn = this->boundaryContiguous (bhi, hi_next, seen);
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
            DBG2 ("Back at start, nowhere left to go! return true.");
            rtn = true;
        } else {
            DBG2 ("Back at hi=(" << hi->ri << "," << hi->gi << "), while bhi=(" <<  bhi->ri << "," << bhi->gi << "), return false");
            rtn = false;
        }
    }

    DBG2 ("Boundary " << (rtn ? "IS" : "isn't") << " contiguous so far...");

    return rtn;
}

void
morph::HexGrid::markFromBoundary (list<Hex>::iterator hi,
                                  unsigned int bdryFlag, unsigned int insideFlag)
{
    this->markFromBoundary (&(*hi), bdryFlag, insideFlag);
}

void
morph::HexGrid::markFromBoundary (list<Hex*>::iterator hi,
                                  unsigned int bdryFlag, unsigned int insideFlag)
{
    this->markFromBoundary ((*hi), bdryFlag, insideFlag);
}

void
morph::HexGrid::markFromBoundary (Hex* hi, unsigned int bdryFlag, unsigned int insideFlag)
{
    // Find a marked-inside Hex next to this boundary hex. This will be the first direction to mark
    // a line of inside hexes in.
    list<Hex>::iterator first_inside = this->hexen.begin();
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
    //DBG ("Mark line in direction " << Hex::neighbour_pos(firsti));
    this->markFromBoundaryCommon (first_inside, firsti, bdryFlag, insideFlag);

    // For each other direction also mark lines. Count direction upwards until we hit a boundary hex:
    short diri = (firsti + 1) % 6;
    //DBG ("First count up direction: " << Hex::neighbour_pos(diri) << " (" << diri << ")");
    while (hi->has_neighbour(diri) && hi->get_neighbour(diri)->testFlags(bdryFlag)==false && diri != firsti) {
        first_inside = hi->get_neighbour(diri);
        //DBG ("Counting up: Mark line in direction " << Hex::neighbour_pos(diri));
        this->markFromBoundaryCommon (first_inside, diri, bdryFlag, insideFlag);
        diri = (diri + 1) % 6;
    }
    // Then count downwards until we hit the other boundary hex
    diri = (firsti - 1);
    if (diri < 0) { diri = 5; }
    //DBG ("First count down direction: " << Hex::neighbour_pos(diri) << " (" << diri << ")");
    while (hi->has_neighbour(diri) && hi->get_neighbour(diri)->testFlags(bdryFlag)==false && diri != firsti) {
        first_inside = hi->get_neighbour(diri);
        //DBG ("Counting down: Mark line in direction " << Hex::neighbour_pos(diri));
        this->markFromBoundaryCommon (first_inside, diri, bdryFlag, insideFlag);
        diri = (diri - 1);
        if (diri < 0) { diri = 5; }
    }
}

void
morph::HexGrid::markFromBoundaryCommon (list<Hex>::iterator first_inside, unsigned short firsti,
                                        unsigned int bdryFlag, unsigned int insideFlag)
{
    // From the "first inside the boundary hex" head in the direction specified by firsti until a
    // boundary hex is reached.
    list<Hex>::iterator straight = first_inside;

#ifdef DO_WARNINGS
    bool warning_given = false;
#endif
    while (straight->testFlags(bdryFlag) == false) {
        DBG2 ("Set insideBoundary true");
        straight->setFlag (insideFlag);
        if (straight->has_neighbour(firsti)) {
            DBG2 ("has neighbour in " << firsti << " dirn");
            straight = straight->get_neighbour (firsti);
        } else {
            // no further neighbour in this direction
            if (straight->testFlags(bdryFlag) == false) {
#ifdef DO_WARNINGS
                if (!warning_given) {
                    cerr << "WARNING: Got to edge of region (dirn " << firsti
                         << ") without encountering a boundary Hex.\n";
                    warning_given = true;
                }
#endif
                break;
            }
        }
    }
}

bool
morph::HexGrid::findNextBoundaryNeighbour (list<Hex>::iterator& bhi, list<Hex>::iterator& bhi_last,
                                           unsigned int bdryFlag, unsigned int insideFlag) const
{
    bool gotnextneighbour = false;
    // From each boundary hex, loop round all 6 neighbours until we get to a new neighbour
    for (unsigned short i = 0; i < 6 && gotnextneighbour == false; ++i) {

        //DBG ("Looking at boundary neighbour in dirn " << Hex::neighbour_pos(i));

        // This is "if it's a neighbour and the neighbour is a boundary hex"
        if (bhi->has_neighbour(i) && bhi->get_neighbour(i)->testFlags(bdryFlag)) {

            //DBG (Hex::neighbour_pos(i) << " is a candidate boundary hex");
            // cbhi is "candidate boundary hex iterator", now guaranteed to be a boundary hex
            list<Hex>::iterator cbhi = bhi->get_neighbour(i);

            if (bhi_last != this->hexen.end() && cbhi == bhi_last) {
                //DBG ("candidate was the last used boundary hex, continue: " << cbhi->outputCart());
                continue;
            }

            unsigned short i_opp = ((i+3)%6);
            //DBG2 ("Opp. dirn: " << Hex::neighbour_pos(i_opp));

            // Go round each of the candidate boundary hex's neighbours (but j!=i)
            for (unsigned short j = 0; j < 6; ++j) {

                // Ignore the candidate boundary hex itself
                if (j==i_opp) {
                    //DBG ("Neighbour to " << Hex::neighbour_pos (j) << " is the candidate iself");
                    continue;
                }
#ifdef DEBUG2
                if (cbhi->has_neighbour(j)) {
                    DBG ("Candidate neighbour " << Hex::neighbour_pos (j)
                         << " Inside:" << (cbhi->get_neighbour(j)->testFlags(insideFlag)?"Y":"N")
                         << " Boundary:" << (cbhi->get_neighbour(j)->testFlags(bdryFlag)?"Y":"N"));
                } else {
                    DBG ("No neighbour to " << Hex::neighbour_pos (j));
                }
#endif
                if (cbhi->has_neighbour(j)
                    && cbhi->get_neighbour(j)->testFlags(insideFlag)==true
                    && cbhi->get_neighbour(j)->testFlags(bdryFlag)==false) {
                    bhi_last = bhi;
                    bhi = cbhi;
                    gotnextneighbour = true;
                    break;
                }
            }
        }
    }

    return (gotnextneighbour);
}

void
morph::HexGrid::markHexesInside (list<Hex>::iterator centre_hi,
                                 unsigned int bdryFlag, unsigned int insideFlag)
{
    // Run to boundary, marking as we go
    list<Hex>::iterator bhi(centre_hi);
    while (bhi->testFlags (bdryFlag) == false && bhi->has_nne()) {
        bhi->setFlag (insideFlag);
        bhi = bhi->nne;
    }
    list<Hex>::iterator bhi_start = bhi;

    // Mark from first boundary hex and across the region
    //DBG ("markFromBoundary with hex " << bhi->outputCart());
    this->markFromBoundary (bhi, bdryFlag, insideFlag);

    // Find the first next neighbour:
    list<Hex>::iterator bhi_last = this->hexen.end();
    bool gotnext = this->findNextBoundaryNeighbour (bhi, bhi_last, bdryFlag, insideFlag);
    // Loop around boundary, marking inwards in all possible directions from each boundary hex
    while (gotnext && bhi != bhi_start) {
        //DBG ("0 markFromBoundary with hex " << bhi->outputCart());
        this->markFromBoundary (bhi, bdryFlag, insideFlag);
        gotnext = this->findNextBoundaryNeighbour (bhi, bhi_last, bdryFlag, insideFlag);
    }
}

void
morph::HexGrid::markHexesInsideRectangularDomain (const array<int, 6>& extnts)
{
    // Check ri,gi,bi and reduce to equivalent ri,gi,bi=0.  Use gi to determine whether outside
    // top/bottom region Add gi contribution to ri to determine whether outside left/right region

    // Is the bottom row's gi even or odd?  extnts[2] is gi for the bottom row. If it's even, then
    // we add 0.5 to all rows with even gi. If it's odd then we add 0.5 to all rows with ODD gi.
    float even_addn = 0.5f;
    float odd_addn = 0.0f;
    float addleft = 0;
    if (extnts[2]%2 == 0) {
        DBG ("bottom row has EVEN gi (" << extnts[2] << ")");
        even_addn = 0.0f;
        odd_addn = 0.5f;
    } else {
        DBG ("bottom row has ODD gi (" << extnts[2] << ")");
        addleft += 0.5f;
    }

    if (abs(extnts[2]%2) == abs(extnts[4]%2)) {
        // Left most hex is on a parity-matching line to bottom line, no need to add left.
        DBG("Left most hex on line matching parity of bottom line")
    } else {
        // Need to add left.
        DBG("Left most hex NOT on line matching parity of bottom line add left to BL hex");
        if (extnts[2]%2 == 0) {
            addleft += 1.0f;
            // For some reason, only in this case do we addleft (and not in the case where BR is
            // ODD and Left most hex NOT matching, which makes addleft = 0.5 + 0.5). I can't work
            // it out.
            DBG ("Before: d_rowlen " << d_rowlen << " d_size " << d_size);
            this->d_rowlen += addleft;
            this->d_size = this->d_rowlen * this->d_numrows;
            DBG ("after: d_rowlen " << d_rowlen << " d_size " << d_size);
        } else {
            addleft += 0.5f;
        }
    }

    DBG ("FINAL addleft is: " << addleft);

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
            DBG2 ("Outside. gi:" << hi->gi << ". Horz idx: " << hz
                  << " < extnts[0]-addleft+parityhalf: " << extnts[0] << "-" << addleft
                  << "+" << parityhalf);
        } else if (hz > (extnts[1] + parityhalf)) {
            // outside
            DBG2 ("Outside. gi:" << hi->gi << ". Horz idx: " << hz
                  << " > extnts[1]+parityhalf: " << extnts[0] << "+" << parityhalf);
        } else if (hi->gi < extnts[2]) {
            // outside
            DBG2 ("Outside. Vert idx: " << hi->gi << " < extnts[2]: " << extnts[2]);
        } else if (hi->gi > extnts[3]) {
            // outside
            DBG2 ("Outside. Vert idx: " << hi->gi << " > extnts[3]: " << extnts[3]);
        } else {
            // inside
            DBG2 ("INSIDE. Horz,vert index: " << hz << "," << hi->gi);
            hi->setInsideDomain();
        }
        ++hi;
    }
}

void
morph::HexGrid::markHexesInsideParallelogramDomain (const array<int, 6>& extnts)
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

void
morph::HexGrid::markAllHexesInsideDomain (void)
{
    list<Hex>::iterator hi = this->hexen.begin();
    while (hi != this->hexen.end()) {
        hi->setInsideDomain();
        hi++;
    }
}

void
morph::HexGrid::computeDistanceToBoundary (void)
{
    list<Hex>::iterator h = this->hexen.begin();
    while (h != this->hexen.end()) {
        if (h->testFlags(HEX_IS_BOUNDARY) == true) {
            h->distToBoundary = 0.0f;
        } else {
            if (h->testFlags(HEX_INSIDE_BOUNDARY) == false) {
                // Set to a dummy, negative value
                h->distToBoundary = -100.0;
            } else {
                // Not a boundary hex, but inside boundary
                list<Hex>::iterator bh = this->hexen.begin();
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
        DBG2 ("Hex: " << h->vi <<"  d to bndry: " << h->distToBoundary
              << " on bndry? " << (h->testFlags(HEX_IS_BOUNDARY)?"Y":"N"));
        ++h;
    }
}

array<int, 6>
morph::HexGrid::findBoundaryExtents (void)
{
    // Return object contains {ri-left, ri-right, gi-bottom, gi-top, gi at ri-left, gi at ri-right}
    // i.e. {xmin, xmax, ymin, ymax, gi at xmin, gi at xmax}
    array<int, 6> rtn = {{0,0,0,0,0,0}};

    // Find the furthest left and right hexes and the further up and down hexes.
    array<float, 4> limits = {{0,0,0,0}};
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

    DBG ("ll,lr,lb,lt:     {" << limits[0] << "," << limits[1] << "," << limits[2]
         << "," << limits[3] << "}");
    DBG ("d_ri: " << d_ri << ", d_gi: " << d_gi);
    DBG ("ril,rir,gib,git: {" << rtn[0] << "," << rtn[1] << "," << rtn[2] << "," << rtn[3] << "}");

    // Add 'growth buffer'
    rtn[0] -= this->d_growthbuffer_horz;
    rtn[1] += this->d_growthbuffer_horz;
    rtn[2] -= this->d_growthbuffer_vert;
    rtn[3] += this->d_growthbuffer_vert;

    return rtn;
}

float
morph::HexGrid::width (void)
{
    // {xmin, xmax, ymin, ymax, gi at xmin, gi at xmax}
    array<int, 6> extents = this->findBoundaryExtents();
    float xmin = this->d * float(extents[0]);
    float xmax = this->d * float(extents[1]);
    return (xmax - xmin);
}

float
morph::HexGrid::depth (void)
{
    array<int, 6> extents = this->findBoundaryExtents();
    float ymin = this->v * float(extents[2]);
    float ymax = this->v * float(extents[3]);
    return (ymax - ymin);
}

void
morph::HexGrid::d_clear (void)
{
    this->d_x.clear();
    this->d_y.clear();
    this->d_ri.clear();
    this->d_gi.clear();
    this->d_bi.clear();
    this->d_flags.clear();
}

void
morph::HexGrid::d_push_back (list<Hex>::iterator hi)
{
    d_x.push_back (hi->x);
    d_y.push_back (hi->y);
    d_ri.push_back (hi->ri);
    d_gi.push_back (hi->gi);
    d_bi.push_back (hi->bi);
    d_flags.push_back (hi->getFlags());
    d_distToBoundary.push_back (hi->distToBoundary);

    // record in the Hex the iterator in the d_ vectors so that d_nne and friends can be set up
    // later.
    hi->di = d_x.size()-1;
}

void
morph::HexGrid::populate_d_neighbours (void)
{
    // Resize d_nne and friends
    this->d_nne.resize (this->d_x.size(), 0);
    this->d_ne.resize (this->d_x.size(), 0);
    this->d_nnw.resize (this->d_x.size(), 0);
    this->d_nw.resize (this->d_x.size(), 0);
    this->d_nsw.resize (this->d_x.size(), 0);
    this->d_nse.resize (this->d_x.size(), 0);

    list<Hex>::iterator hi = this->hexen.begin();
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

#ifdef DEBUG__
        //if (hi->di == 1075 || hi->di == 1076) {
        DBG("d_[" << hi->di << "] has NNE: " << this->d_nne[hi->di]
            << ", NNW: " << this->d_nnw[hi->di]
            << ", NW: " << this->d_nw[hi->di]
            << ", NSW: " << this->d_nsw[hi->di]
            << ", NSE: " << this->d_nse[hi->di]
            << ", NE: " << this->d_ne[hi->di]);
        //}
#endif
        ++hi;
    }
}

void
morph::HexGrid::setDomain (void)
{
    // 1. Find extent of boundary, both left/right and up/down, with 'buffer region' already added.
    array<int, 6> extnts = this->findBoundaryExtents();

    // 1.5 set rowlen and numrows
    this->d_rowlen = extnts[1]-extnts[0]+1;
    this->d_numrows = extnts[3]-extnts[2]+1;
    this->d_size = this->d_rowlen * this->d_numrows;
    DBG("Initially, d_rowlen=" << d_rowlen << ", d_numrows=" << d_numrows << ", d_size=" << d_size);

    if (this->domainShape == morph::HexDomainShape::Rectangle) {
        // 2. Mark Hexes inside and outside the domain.  Mark those hexes inside the boundary
        this->markHexesInsideRectangularDomain (extnts);
    } else if (this->domainShape == morph::HexDomainShape::Parallelogram) {
        this->markHexesInsideParallelogramDomain (extnts);
    } else if (this->domainShape == morph::HexDomainShape::Hexagon) {
        // The original domain was hexagonal, so just mark ALL of them as being in the domain.
        this->markAllHexesInsideDomain();
    } else {
        throw runtime_error ("Unknown HexDomainShape");
    }

    // 3. Discard hexes outside domain
    this->discardOutsideDomain();

    // 3.5 Mark hexes inside boundary
    list<Hex>::iterator centroidHex = this->findHexNearest (this->boundaryCentroid);
    this->markHexesInside (centroidHex);
#ifdef DEBUG
    {
        // Do a little count of them:
        unsigned int numInside = 0;
        unsigned int numOutside = 0;
        for (auto hi : this->hexen) {
            if (hi.testFlags(HEX_INSIDE_BOUNDARY) == true) {
                ++numInside;
            } else {
                ++numOutside;
            }
        }
        DBG ("Num inside: " << numInside << "; num outside: " << numOutside);
    }
#endif

    // Before populating d_ vectors, also compute the distance to boundary
    this->computeDistanceToBoundary();

    // 4. Populate d_ vectors
    this->populate_d_vectors (extnts);
}

void
morph::HexGrid::populate_d_vectors (void)
{
    array<int, 6> extnts = this->findBoundaryExtents();
    this->populate_d_vectors (extnts);
}

void
morph::HexGrid::populate_d_vectors (const array<int, 6>& extnts)
{
    // First, find the starting hex. For Rectangular and parallelogram domains, that's the bottom
    // left hex.
    list<Hex>::iterator hi = this->hexen.begin();
    // bottom left hex.
    list<Hex>::iterator blh = this->hexen.end();

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
        DBG ("hi is on bottom row, posn xy:" << hi->x << "," << hi->y
             << " or rg:" << hi->ri << "," << hi->gi);
        while (hi->has_nw() == true) {
            hi = hi->nw;
        }
        DBG ("hi is at bottom left posn xy:" << hi->x << "," << hi->y
             << " or rg:" << hi->ri << "," << hi->gi);

        // hi should now be the bottom left hex.
        blh = hi;

        // Sanity check
        if (blh->has_nne() == false || blh->has_ne() == false || blh->has_nnw() == true) {
            stringstream ee;
            ee << "We expect the bottom left hex to have an east and a "
               << "north east neighbour, but no north west neighbour. This has: "
               << (blh->has_nne() == true ? "Neighbour NE ":"NO Neighbour NE ")
               << (blh->has_ne() == true ? "Neighbour E ":"NO Neighbour E ")
               << (blh->has_nnw() == true ? "Neighbour NW ":"NO Neighbour NW ");
            throw runtime_error (ee.str());
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

            DBG2 ("Pushed back flags: " << hi->getFlags()
                  << " for r/g: " << hi->ri << "," << hi->gi);

            if (hi->has_ne() == false) {
                if (hi->gi == extnts[3]) {
                    // last (i.e. top) row and no neighbour east, so finished.
                    DBG ("Fin. (top row)");
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
                    // on end of top row and no neighbour east, so finished.
                    DBG ("Fin. (top row)");
                    // push back and break
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
    DBG ("Size of d_x: " << this->d_x.size() << " and d_size=" << this->d_size);
    this->populate_d_neighbours();
}

void
morph::HexGrid::discardOutsideDomain (void)
{
    // Run through and discard those hexes outside the boundary:
    auto hi = this->hexen.begin();
    while (hi != this->hexen.end()) {
        if (hi->insideDomain() == false) {
            // When erasing a Hex, I need to update the neighbours of its neighbours.
            hi->disconnectNeighbours();
            // Having disconnected the neighbours, erase the Hex.
            hi = this->hexen.erase (hi);
        } else {
            ++hi;
        }
    }
    DBG("Number of hexes in this->hexen is now: " << this->hexen.size());

    // The Hex::vi indices need to be re-numbered.
    this->renumberVectorIndices();

    // Finally, do something about the hexagonal grid vertices; set this to true to mark that the
    // iterators to the outermost vertices are no longer valid and shouldn't be used.
    this->gridReduced = true;
}

void
morph::HexGrid::discardOutsideBoundary (void)
{
    // Mark those hexes inside the boundary
    list<Hex>::iterator centroidHex = this->findHexNearest (this->boundaryCentroid);
    this->markHexesInside (centroidHex);

#ifdef DEBUG
    // Do a little count of them:
    unsigned int numInside = 0;
    unsigned int numOutside = 0;
    for (auto hi : this->hexen) {
        if (hi.testFlags(HEX_INSIDE_BOUNDARY) == true) {
            ++numInside;
        } else {
            ++numOutside;
        }
    }
    DBG("Num inside: " << numInside << "; num outside: " << numOutside);
#endif

    // Run through and discard those hexes outside the boundary:
    auto hi = this->hexen.begin();
    while (hi != this->hexen.end()) {
        if (hi->testFlags(HEX_INSIDE_BOUNDARY) == false) {
            // Here's the problem I think. When erasing a Hex, I need to update the neighbours of
            // its neighbours.
            hi->disconnectNeighbours();
            // Having disconnected the neighbours, erase the Hex.
            hi = this->hexen.erase (hi);
        } else {
            ++hi;
        }
    }
    DBG("Number of hexes in this->hexen is now: " << this->hexen.size());

    // The Hex::vi indices need to be re-numbered.
    this->renumberVectorIndices();

    // Finally, do something about the hexagonal grid vertices; set this to true to mark that the
    // iterators to the outermost vertices are no longer valid and shouldn't be used.
    this->gridReduced = true;
}

list<Hex>::iterator
morph::HexGrid::findHexNearest (const pair<float, float>& pos)
{
    list<Hex>::iterator nearest = this->hexen.end();
    list<Hex>::iterator hi = this->hexen.begin();
    float dist = FLT_MAX;
    while (hi != this->hexen.end()) {
        float dx = pos.first - hi->x;
        float dy = pos.second - hi->y;
        float dl = sqrt (dx*dx + dy*dy);
        if (dl < dist) {
            dist = dl;
            nearest = hi;
        }
        ++hi;
    }
    DBG ("Nearest Hex to " << pos.first << "," << pos.second
         << " is (r,g):" << nearest->ri << "," << nearest->gi
         << " (x,y):" << nearest->x << "," << nearest->y);
    return nearest;
}

void
morph::HexGrid::renumberVectorIndices (void)
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

unsigned int
morph::HexGrid::num (void) const
{
    return this->hexen.size();
}

unsigned int
morph::HexGrid::lastVectorIndex (void) const
{
    return this->hexen.rbegin()->vi;
}

string
morph::HexGrid::output (void) const
{
    stringstream ss;
    ss << "Hex grid with " << this->hexen.size() << " hexes." << endl;
    auto i = this->hexen.begin();
    float lasty = this->hexen.front().y;
    unsigned int rownum = 0;
    ss << endl << "Row/Ring " << rownum++ << ":" << endl;
    while (i != this->hexen.end()) {
        if (i->y > lasty) {
            ss << endl << "Row/Ring " << rownum++ << ":" << endl;
            lasty = i->y;
        }
        ss << i->output() << endl;
        ++i;
    }
    return ss.str();
}

string
morph::HexGrid::extent (void) const
{
    stringstream ss;
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

float
morph::HexGrid::getd (void) const
{
    return this->d;
}

float
morph::HexGrid::getv (void) const
{
    return this->v;
}

float
morph::HexGrid::getSR (void) const
{
    return this->d/2;
}

float
morph::HexGrid::getLR (void) const
{
    float lr = this->d/morph::SQRT_OF_3_F;
    return lr;
}

float
morph::HexGrid::getVtoNE (void) const
{
    float vne = this->d/(2.0f*morph::SQRT_OF_3_F);
    return vne;
}

float
morph::HexGrid::getHexArea (void) const
{
    // Area is the area of 6 triangles: (1/2 LR * d/2) * 6
    // or (d*d*3)/(2*sqrt(3)) = d * d * sqrt(3)/2
    float ha = this->d * this->d * morph::SQRT_OF_3_OVER_2_F;
    return ha;
}

float
morph::HexGrid::getXmin (float phi) const
{
    float xmin = 0.0f;
    float x_ = 0.0f;
    bool first = true;
    for (auto h : this->hexen) {
        x_ = h.x * cos (phi) + h.y * sin (phi);
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

float
morph::HexGrid::getXmax (float phi) const
{
    float xmax = 0.0f;
    float x_ = 0.0f;
    bool first = true;
    for (auto h : this->hexen) {
        x_ = h.x * cos (phi) + h.y * sin (phi);
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

void
morph::HexGrid::init (float d_, float x_span_, float z_)
{
    this->d = d_;
    this->v = this->d * SQRT_OF_3_OVER_2_F;
    this->x_span = x_span_;
    this->z = z_;
    this->init();
}

void
morph::HexGrid::init (void)
{
    // Use span_x to determine how many rings out to traverse.
    float halfX = this->x_span/2.0f;
    DBG ("halfX:" << halfX);
    DBG ("d:" << d);
    unsigned int maxRing = abs(ceil(halfX/this->d));
    DBG ("ceil(halfX/d):" << ceil(halfX/d));

    DBG ("Creating hexagonal hex grid with maxRing: " << maxRing);

    // The "vector iterator" - this is an identity iterator that is added to each Hex in the grid.
    unsigned int vi = 0;

    // Vectors of list-iterators to hexes in this->hexen. Used to keep a track of nearest
    // neighbours. I'm using vector, rather than a list as this allows fast random access of
    // elements and I'll not be inserting or erasing in the middle of the arrays.
    vector<list<Hex>::iterator> prevRingEven;
    vector<list<Hex>::iterator> prevRingOdd;

    // Swap pointers between rings.
    vector<list<Hex>::iterator>* prevRing = &prevRingEven;
    vector<list<Hex>::iterator>* nextPrevRing = &prevRingOdd;

    // Direction iterators used in the loop for creating hexes
    int ri = 0;
    int gi = 0;

    // Create central "ring" first (the single hex)
    this->hexen.emplace_back (vi++, this->d, ri, gi);

    // Put central ring in the prevRing vector:
    {
        list<Hex>::iterator h = this->hexen.end(); --h;
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

        DBG2 ("\n\n************** numInRing: " << numInRing << " ******************");

        // Set start ri, gi. This moves up a hex and left a hex onto the start hex of the new ring.
        --ri; ++gi;

        nextPrevRing->clear();

        // Now walk around the ring, in 6 walks, that will bring us round to just before we
        // started. walkstart has the starting iterator number for the vertices of the hexagon.
        DBG2 ("Before r; walkinc: " << walkinc << ", walkmin: " << walkmin
              << ", walkmax: " << walkmax);

        // Walk in the r direction first:
        DBG2 ("============ r walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {

            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri++, gi);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexNW = hi; }

            // 1. Set my W neighbour to be the previous hex in THIS ring, if possible
            if (i > 0) {
                hi->set_nw (lasthi);
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as E neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as E neighbour to previous hex in the ring:
                lasthi->set_ne (hi);
            } else {
                // i must be 0 in this case, we would set the SW neighbour now, but as this won't
                // have been added to the ring, we have to leave it.
                DBG2 (" r walk: I am (" << hi->ri << "," << hi->gi
                      << "). Omitting SW neighbour of first hex in ring.");
            }

            // 2. SW neighbour
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j << ", walk min/max: " << walkmin << " " << walkmax);
            if (j>walkmin && j<walkmax) {
                // Set my SW neighbour:
                hi->set_nsw ((*prevRing)[j]);
                // Set me as NE neighbour to those in prevRing:
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as NE neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nne (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my SE neighbour:
            if (j<=walkmax) {
                hi->set_nse ((*prevRing)[j]);
                // Set me as NW neighbour:
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as NW neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nnw (hi);
            }

            // Put in me nextPrevRing:
            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin   += walkinc;
        walkmax   += walkinc;

        // Walk in -b direction
        DBG2 ("Before -b; walkinc: " << walkinc << ", walkmin: " << walkmin
              << ", walkmax: " << walkmax);
        DBG2 ("=========== -b walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {
            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri++, gi--);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexNE = hi; }

            // 1. Set my NW neighbour to be the previous hex in THIS ring, if possible
            if (i > 0) {
                hi->set_nnw (lasthi);
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SE neighbour for hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as SE neighbour to previous hex in the ring:
                lasthi->set_nse (hi);
            } else {
                // Set my W neighbour for the first hex in the row.
                hi->set_nw (lasthi);
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as E neighbour for last walk's hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as E neighbour to previous hex in the ring:
                lasthi->set_ne (hi);
            }

            // 2. W neighbour
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j << " prevRing->size(): " <<prevRing->size() );
            if (j>walkmin && j<walkmax) {
                // Set my W neighbour:
                hi->set_nw ((*prevRing)[j]);
                // Set me as E neighbour to those in prevRing:
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as E neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_ne (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my SW neighbour:
            DBG2 ("i is " << i << ", j is " << j);
            if (j<=walkmax) {
                hi->set_nsw ((*prevRing)[j]);
                // Set me as NE neighbour:
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as NE neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nne (hi);
            }

            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin += walkinc;
        walkmax += walkinc;
        DBG2 ("walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);

        // Walk in -g direction
        DBG2 ("=========== -g walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {

            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri, gi--);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexE = hi; }

            // 1. Set my NE neighbour to be the previous hex in THIS ring, if possible
            if (i > 0) {
                hi->set_nne (lasthi);
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SW neighbour for hex at (" << lasthi->ri << ","
                      << lasthi->gi << ")");
                // Set me as SW neighbour to previous hex in the ring:
                lasthi->set_nsw (hi);
            } else {
                // Set my NW neighbour for the first hex in the row.
                hi->set_nnw (lasthi);
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SE neighbour for last walk's hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as SE neighbour to previous hex in the ring:
                lasthi->set_nse (hi);
            }

            // 2. NW neighbour
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j);
            if (j>walkmin && j<walkmax) {
                // Set my NW neighbour:
                hi->set_nnw ((*prevRing)[j]);
                // Set me as SE neighbour to those in prevRing:
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SE neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nse (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my W neighbour:
            if (j<=walkmax) {
                hi->set_nw ((*prevRing)[j]);
                // Set me as E neighbour:
                DBG2 ("-g walk: Set me (" << hi->ri << ","
                      << hi->gi << ") as E neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_ne (hi);
            }

            // Put in me nextPrevRing:
            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin += walkinc;
        walkmax += walkinc;
        DBG2 ("walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);

        // Walk in -r direction
        DBG2 ("=========== -r walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {

            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri--, gi);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexSE = hi; }

            // 1. Set my E neighbour to be the previous hex in THIS ring, if possible
            if (i > 0) {
                hi->set_ne (lasthi);
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as W neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as W neighbour to previous hex in the ring:
                lasthi->set_nw (hi);
            } else {
                // Set my NE neighbour for the first hex in the row.
                hi->set_nne (lasthi);
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SW neighbour for last walk's hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as SW neighbour to previous hex in the ring:
                lasthi->set_nsw (hi);
            }

            // 2. NE neighbour:
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j);
            if (j>walkmin && j<walkmax) {
                // Set my NE neighbour:
                hi->set_nne ((*prevRing)[j]);
                // Set me as SW neighbour to those in prevRing:
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SW neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nsw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my NW neighbour:
            if (j<=walkmax) {
                hi->set_nnw ((*prevRing)[j]);
                // Set me as SE neighbour:
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SE neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nse (hi);
            }

            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin += walkinc;
        walkmax += walkinc;
        DBG2 ("walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);

        // Walk in b direction
        DBG2 ("============ b walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {
            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri--, gi++);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexSW = hi; }

            // 1. Set my SE neighbour to be the previous hex in THIS ring, if possible
            if (i > 0) {
                hi->set_nse (lasthi);
                DBG2 (" b in-ring: Set me (" << hi->ri << "," << hi->gi
                      << ") as NW neighbour for hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as NW neighbour to previous hex in the ring:
                lasthi->set_nnw (hi);
            } else { // i == 0
                // Set my E neighbour for the first hex in the row.
                hi->set_ne (lasthi);
                DBG2 (" b in-ring: Set me (" << hi->ri << ","
                      << hi->gi << ") as W neighbour for last walk's hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as W neighbour to previous hex in the ring:
                lasthi->set_nw (hi);
            }

            // 2. E neighbour:
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j);
            if (j>walkmin && j<walkmax) {
                // Set my E neighbour:
                hi->set_ne ((*prevRing)[j]);
                // Set me as W neighbour to those in prevRing:
                DBG2 (" b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as W neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my NE neighbour:
            if (j<=walkmax) {
                hi->set_nne ((*prevRing)[j]);
                // Set me as SW neighbour:
                DBG2 (" b walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as SW neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nsw (hi);
            }

            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin += walkinc;
        walkmax += walkinc;
        DBG2 ("walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);

        // Walk in g direction up to almost the last hex
        DBG2 ("============ g walk =================");
        for (unsigned int i = 0; i<ringSideLen; ++i) {

            DBG2 ("Adding hex at " << ri << "," << gi);
            this->hexen.emplace_back (vi++, this->d, ri, gi++);
            auto hi = this->hexen.end(); hi--;
            auto lasthi = hi;
            --lasthi;

            // Set vertex
            if (i==0) { vertexW = hi; }

            // 1. Set my SW neighbour to be the previous hex in THIS ring, if possible
            DBG2(" g walk: i is " << i << " and ringSideLen-1 is " << (ringSideLen-1));
            if (i == (ringSideLen-1)) {
                // Special case at end; on last g walk hex, set the NE neighbour Set my NE neighbour
                // for the first hex in the row.
                hi->set_nne ((*nextPrevRing)[0]); // (*nextPrevRing)[0] is an iterator to the first
                                                  // hex

                DBG2 (" g in-ring: Set me (" << hi->ri << ","
                      << hi->gi << ") as SW neighbour for this ring's first hex at ("
                      << (*nextPrevRing)[0]->ri << "," << (*nextPrevRing)[0]->gi << ")");
                // Set me as NW neighbour to previous hex in the ring:
                (*nextPrevRing)[0]->set_nsw (hi);

            }
            if (i > 0) {
                hi->set_nsw (lasthi);
                DBG2 (" g in-ring: Set me (" << hi->ri << "," << hi->gi
                      << ") as NE neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as NE neighbour to previous hex in the ring:
                lasthi->set_nne (hi);
            } else {
                // Set my SE neighbour for the first hex in the row.
                hi->set_nse (lasthi);
                DBG2 (" g in-ring: Set me (" << hi->ri << "," << hi->gi
                      << ") as NW neighbour for last walk's hex at ("
                      << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as NW neighbour to previous hex in the ring:
                lasthi->set_nnw (hi);
            }

            // 2. E neighbour:
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j);
            if (j>walkmin && j<walkmax) {
                // Set my SE neighbour:
                hi->set_nse ((*prevRing)[j]);
                // Set me as NW neighbour to those in prevRing:
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as NW neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nnw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my E neighbour:
            if (j==walkmax) { // We're on the last square and need to set the East neighbour of the
                              // first hex in the last ring.
                hi->set_ne ((*prevRing)[0]);
                // Set me as W neighbour:
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as W neighbour for hex at ("
                      << (*prevRing)[0]->ri << "," << (*prevRing)[0]->gi << ")");
                (*prevRing)[0]->set_nw (hi);

            } else if (j<walkmax) {
                hi->set_ne ((*prevRing)[j]);
                // Set me as W neighbour:
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi
                      << ") as W neighbour for hex at ("
                      << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
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
        vector<list<Hex>::iterator>* tmp = prevRing;
        prevRing = nextPrevRing;
        DBG2 ("New prevRing contains " << prevRing->size() << " elements");
        nextPrevRing = tmp;
    }

    DBG ("Finished creating " << this->hexen.size() << " hexes in " << maxRing << " rings.");
}
