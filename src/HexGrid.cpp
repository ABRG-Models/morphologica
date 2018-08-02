/*!
 * Implementation of HexGrid
 *
 * Author: Seb James
 *
 * Date: 2018/07
 */

#include "HexGrid.h"
#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <stdexcept>
#include "BezCurvePath.h"
#include "BezCoord.h"

#define DBGSTREAM std::cout
#define DEBUG 1
//#define DEBUG2 1
#include "MorphDbg.h"

using std::ceil;
using std::abs;
using std::endl;
using std::stringstream;
using std::vector;
using std::set;
using std::runtime_error;
using std::numeric_limits;

using morph::BezCurvePath;
using morph::BezCoord;
using morph::Hex;

morph::HexGrid::HexGrid ()
    : d(1.0f)
    , x_span(1.0f)
    , z(0.0f)
{
}

morph::HexGrid::HexGrid (float d_, float x_span_, float z_)
{
    this->d = d_;
    this->x_span = x_span_;
    this->z = z_;

    this->init();
}

void
morph::HexGrid::setBoundary (const BezCurvePath& p)
{
    this->boundary = p;

    if (!this->boundary.isNull()) {
        DBG ("Applying boundary...");

        // Compute the points on the boundary using half of the hex to
        // hex spacing as the step size.
        vector<BezCoord> bpoints = this->boundary.getPoints (this->d/2.0f);

        pair<float,float> c =  BezCurvePath::getCentroid (bpoints);
        DBG ("Boundary centroid: " << c.first << "," << c.second);

        // Offset the boundary by the centroid, which should now be 0,0.
        auto bpi = bpoints.begin();
        while (bpi != bpoints.end()) {
            bpi->subtract(c);
            ++bpi;
        }

        bpi = bpoints.begin();
        list<Hex>::iterator nearbyBoundaryPoint = this->hexen.begin();
        while (bpi != bpoints.end()) {
            nearbyBoundaryPoint = this->setBoundary (*bpi++, nearbyBoundaryPoint);
            DBG2 ("Added boundary point " << nearbyBoundaryPoint->ri << "," << nearbyBoundaryPoint->gi);
        }

        // Check that the boundary is contiguous.
        if (this->boundaryContiguous (nearbyBoundaryPoint) == false) {
            stringstream ee;
            ee << "The boundary which was constructed from "
               << p.name << " is not a contiguous sequence of hexes.";
            throw runtime_error (ee.str());
        }

        // Given that the boundary IS contiguous, can now discard
        // hexes outside the boundary.
        this->discardOutside();
    }
}

list<Hex>::iterator
morph::HexGrid::setBoundary (const BezCoord& point, list<Hex>::iterator startFrom)
{
    // Searching from "startFrom", search out, via neighbours until
    // the hex closest to the boundary point is located. How to know
    // if it's closest? When all neighbours are further from the
    // currently closest point?

    bool neighbourNearer = true;

    list<Hex>::iterator h = startFrom;
    float d = h->distanceFrom (point);
    float d_ = 0.0f;

    while (neighbourNearer == true) {

        neighbourNearer = false;
        if (h->has_ne && (d_ = h->ne->distanceFrom (point)) < d) {
            d = d_;
            h = h->ne;
            neighbourNearer = true;

        } else if (h->has_nne && (d_ = h->nne->distanceFrom (point)) < d) {
            d = d_;
            h = h->nne;
            neighbourNearer = true;

        } else if (h->has_nnw && (d_ = h->nnw->distanceFrom (point)) < d) {
            d = d_;
            h = h->nnw;
            neighbourNearer = true;

        } else if (h->has_nw && (d_ = h->nw->distanceFrom (point)) < d) {
            d = d_;
            h = h->nw;
            neighbourNearer = true;

        } else if (h->has_nsw && (d_ = h->nsw->distanceFrom (point)) < d) {
            d = d_;
            h = h->nsw;
            neighbourNearer = true;

        } else if (h->has_nse && (d_ = h->nse->distanceFrom (point)) < d) {
            d = d_;
            h = h->nse;
            neighbourNearer = true;
        }
    }

    DBG2 ("Nearest hex to point (" << point.x() << "," << point.y() << ") is at (" << h->ri << "," << h->gi << ")");

    // Mark it for being on the boundary
    h->boundaryHex = true;

    return h;
}

bool
morph::HexGrid::findBoundaryHex (list<Hex>::const_iterator& hi) const
{
    if (hi->boundaryHex == true) {
        // No need to change the Hex iterator
        return true;
    }

    if (hi->has_ne) {
        list<Hex>::const_iterator ci(hi->ne);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nne) {
        list<Hex>::const_iterator ci(hi->nne);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nnw) {
        list<Hex>::const_iterator ci(hi->nnw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nw) {
        list<Hex>::const_iterator ci(hi->nw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nsw) {
        list<Hex>::const_iterator ci(hi->nsw);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }
    if (hi->has_nse) {
        list<Hex>::const_iterator ci(hi->nse);
        if (this->findBoundaryHex (ci) == true) {
            hi = ci;
            return true;
        }
    }

    return false;
}

bool
morph::HexGrid::boundaryContiguous (void) const
{
    list<Hex>::const_iterator hi = this->hexen.begin();
    if (this->findBoundaryHex (hi) == false) {
        // Found no boundary hex
        return false;
    }
    return this->boundaryContiguous (hi);
}

bool
morph::HexGrid::boundaryContiguous (list<Hex>::const_iterator bhi) const
{
    bool rtn = true;

    list<Hex>::const_iterator hi = bhi;
    list<Hex>::const_iterator hi_next = bhi;
    set<unsigned int> seen;

    do {
        hi = hi_next;
        if (hi != bhi) {
            DBG2 ("Inserting " << hi->vi << " into seen");
            seen.insert (hi->vi);
        }
        DBG2 (hi->output());

        if (hi->has_ne && hi->ne->boundaryHex == true && seen.find(hi->ne->vi) == seen.end()) {
            DBG2 ("hi_next is neighbour EAST");
            hi_next = hi->ne;
        } else {
            if (hi->has_nne && hi->nne->boundaryHex == true && seen.find(hi->nne->vi) == seen.end()) {
                DBG2 ("hi_next is neighbour NORTHEAST");
                hi_next = hi->nne;
            } else {
                if (hi->has_nnw && hi->nnw->boundaryHex == true && seen.find(hi->nnw->vi) == seen.end()) {
                    DBG2 ("hi_next is neighbour NORTHWEST");
                    hi_next = hi->nnw;
                } else {
                    if (hi->has_nw && hi->nw->boundaryHex == true && seen.find(hi->nw->vi) == seen.end()) {
                        DBG2 ("hi_next is neighbour WEST");
                        hi_next = hi->nw;
                    } else {
                        if (hi->has_nsw && hi->nsw->boundaryHex == true && seen.find(hi->nsw->vi) == seen.end()) {
                            DBG2 ("hi_next is neighbour SOUTHWEST");
                            hi_next = hi->nsw;
                        } else {
                            if (hi->has_nse && hi->nse->boundaryHex == true && seen.find(hi->nse->vi) == seen.end()) {
                                DBG2 ("hi_next is neighbour SOUTHEAST");
                                hi_next = hi->nse;
                            } else {
                                // No neighbours left.
                                DBG2 ("No neighbours left");
                                if (hi == bhi) {
                                    DBG2 ("Back at start, nowhere left to go!");
                                    rtn = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        DBG2  ("Current hex: " << hi->ri << "," << hi->gi
              << ". Next boundary hex: " << hi_next->ri << "," << hi_next->gi);

        // if not progressed here, return false.
        if (hi_next == hi) {
            DBG ("hi_next == hi!");
            rtn = false;
            break;
        }

    } while (hi_next != bhi);

    DBG ("Boundary " << (rtn ? "IS" : "isn't") << " contiguous");

    return rtn;
}

void
morph::HexGrid::markHexesInside (list<Hex>::iterator hi)
{
    if (hi->boundaryHex == true) {
        hi->insideBoundary = true;
        return;

    } else if (hi->insideBoundary == true) {
        return;

    } else {

        hi->insideBoundary = true;

        if (hi->has_ne) {
            this->markHexesInside (hi->ne);
        }
        if (hi->has_nne) {
            this->markHexesInside (hi->nne);
        }
        if (hi->has_nnw) {
            this->markHexesInside (hi->nnw);
        }
        if (hi->has_nw) {
            this->markHexesInside (hi->nw);
        }
        if (hi->has_nsw) {
            this->markHexesInside (hi->nsw);
        }
        if (hi->has_nse) {
            this->markHexesInside (hi->nse);
        }
    }
}

void
morph::HexGrid::computeDistanceToBoundary (void)
{
    list<Hex>::iterator h = this->hexen.begin();
    while (h != this->hexen.end()) {
        if (h->boundaryHex == true) {
            h->distToBoundary = 0.0f;
        } else {
            // Not a boundary hex
            list<Hex>::iterator bh = this->hexen.begin();
            while (bh != this->hexen.end()) {
                if (bh->boundaryHex == true) {
                    float delta = h->distanceFrom (*bh);
                    if (delta < h->distToBoundary || h->distToBoundary < 0.0f) {
                        h->distToBoundary = delta;
                    }
                }
                ++bh;
            }
        }
        DBG2 ("Hex: " << h->vi <<"  d to bndry: " << h->distToBoundary
              << " on bndry? " << (h->boundaryHex?"Y":"N"));
        ++h;
    }
}

void
morph::HexGrid::discardOutside (void)
{
    // Mark those hexes inside the boundary
    this->markHexesInside (this->hexen.begin());

#ifdef DEBUG
    // Do a little count of them:
    unsigned int numInside = 0;
    unsigned int numOutside = 0;
    for (auto hi : this->hexen) {
        if (hi.insideBoundary == true) {
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
        if (hi->insideBoundary == false) {
            // Here's the problem I think. When erasing a Hex, I need
            // to update the neighbours of its neighbours.
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

    // Finally, do something about the hexagonal grid vertices; set
    // this to true to mark that the iterators to the outermost
    // vertices are no longer valid and shouldn't be used.
    this->gridReducedToBoundary = true;
}

void
morph::HexGrid::renumberVectorIndices (void)
{
    unsigned int vi = 0;
    this->vhexen.clear();
    auto hi = this->hexen.begin();
    while (hi != this->hexen.end()) {
        DBG2 ("Old vi: " << hi->vi << " new vi: " << vi);
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
    if (gridReducedToBoundary == false) {
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

    // The "vector iterator" - this is an identity iterator that is
    // added to each Hex in the grid.
    unsigned int vi = 0;

    // Vectors of list-iterators to hexes in this->hexen. Used to keep a
    // track of nearest neighbours. I'm using vector, rather than a
    // list as this allows fast random access of elements and I'll not
    // be inserting or erasing in the middle of the arrays.
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

    // Now build up the rings around it, setting neighbours as we
    // go. Each ring has 6 more hexes than the previous one (except
    // for ring 1, which has 6 instead of 1 in the centre).
    unsigned int numInRing = 6;

    // How many hops in the same direction before turning a corner?
    // Increases for each ring. Increases by 1 in each ring.
    unsigned int ringSideLen = 1;

    // These are used to iterate along the six sides of the hexagonal
    // ring that's inside, but adjacent to the hexagonal ring that's
    // under construction.
    int walkstart = 0;
    int walkinc = 0;
    int walkmin = walkstart-1;
    int walkmax = 1;

    for (unsigned int ring = 1; ring <= maxRing; ++ring) {

        DBG2 ("\n\n************** numInRing: " << numInRing << " ******************");

        // Set start ri, gi. This moves up a hex and left a hex onto
        // the start hex of the new ring.
        --ri; ++gi;

        nextPrevRing->clear();

        // Now walk around the ring, in 6 walks, that will bring us
        // round to just before we started. walkstart has the starting
        // iterator number for the vertices of the hexagon.
        DBG2 ("Before r; walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);

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
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi << ") as E neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as E neighbour to previous hex in the ring:
                lasthi->set_ne (hi);
            } else {
                // i must be 0 in this case, we would set the SW
                // neighbour now, but as this won't have been added to
                // the ring, we have to leave it.
                DBG2 (" r walk: I am (" << hi->ri << "," << hi->gi << "). Omitting SW neighbour of first hex in ring.");
            }

            // 2. SW neighbour
            int j = walkstart + (int)i - 1;
            DBG2 ("i is " << i << ", j is " << j << ", walk min/max: " << walkmin << " " << walkmax);
            if (j>walkmin && j<walkmax) {
                // Set my SW neighbour:
                hi->set_nsw ((*prevRing)[j]);
                // Set me as NE neighbour to those in prevRing:
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi << ") as NE neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nne (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my SE neighbour:
            if (j<=walkmax) {
                hi->set_nse ((*prevRing)[j]);
                // Set me as NW neighbour:
                DBG2 (" r walk: Set me (" << hi->ri << "," << hi->gi << ") as NW neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nnw (hi);
            }

            // Put in me nextPrevRing:
            nextPrevRing->push_back (hi);
        }
        walkstart += walkinc;
        walkmin   += walkinc;
        walkmax   += walkinc;

        // Walk in -b direction
        DBG2 ("Before -b; walkinc: " << walkinc << ", walkmin: " << walkmin << ", walkmax: " << walkmax);
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
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi << ") as SE neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as SE neighbour to previous hex in the ring:
                lasthi->set_nse (hi);
            } else {
                // Set my W neighbour for the first hex in the row.
                hi->set_nw (lasthi);
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi << ") as E neighbour for last walk's hex at (" << lasthi->ri << "," << lasthi->gi << ")");
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
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi << ") as E neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_ne (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my SW neighbour:
            DBG2 ("i is " << i << ", j is " << j);
            if (j<=walkmax) {
                hi->set_nsw ((*prevRing)[j]);
                // Set me as NE neighbour:
                DBG2 ("-b walk: Set me (" << hi->ri << "," << hi->gi << ") as NE neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
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
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi << ") as SW neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as SW neighbour to previous hex in the ring:
                lasthi->set_nsw (hi);
            } else {
                // Set my NW neighbour for the first hex in the row.
                hi->set_nnw (lasthi);
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi << ") as SE neighbour for last walk's hex at (" << lasthi->ri << "," << lasthi->gi << ")");
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
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi << ") as SE neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nse (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my W neighbour:
            if (j<=walkmax) {
                hi->set_nw ((*prevRing)[j]);
                // Set me as E neighbour:
                DBG2 ("-g walk: Set me (" << hi->ri << "," << hi->gi << ") as E neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
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
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi << ") as W neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as W neighbour to previous hex in the ring:
                lasthi->set_nw (hi);
            } else {
                // Set my NE neighbour for the first hex in the row.
                hi->set_nne (lasthi);
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi << ") as SW neighbour for last walk's hex at (" << lasthi->ri << "," << lasthi->gi << ")");
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
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi << ") as SW neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nsw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my NW neighbour:
            if (j<=walkmax) {
                hi->set_nnw ((*prevRing)[j]);
                // Set me as SE neighbour:
                DBG2 ("-r walk: Set me (" << hi->ri << "," << hi->gi << ") as SE neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
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
                DBG2 (" b in-ring: Set me (" << hi->ri << "," << hi->gi << ") as NW neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as NW neighbour to previous hex in the ring:
                lasthi->set_nnw (hi);
            } else { // i == 0
                // Set my E neighbour for the first hex in the row.
                hi->set_ne (lasthi);
                DBG2 (" b in-ring: Set me (" << hi->ri << "," << hi->gi << ") as W neighbour for last walk's hex at (" << lasthi->ri << "," << lasthi->gi << ")");
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
                DBG2 (" b walk: Set me (" << hi->ri << "," << hi->gi << ") as W neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my NE neighbour:
            if (j<=walkmax) {
                hi->set_nne ((*prevRing)[j]);
                // Set me as SW neighbour:
                DBG2 (" b walk: Set me (" << hi->ri << "," << hi->gi << ") as SW neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
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
                // Special case at end; on last g walk hex, set the NE neighbour
                // Set my NE neighbour for the first hex in the row.
                hi->set_nne ((*nextPrevRing)[0]); // (*nextPrevRing)[0] is an iterator to the first hex

                DBG2 (" g in-ring: Set me (" << hi->ri << "," << hi->gi << ") as SW neighbour for this ring's first hex at (" << (*nextPrevRing)[0]->ri << "," << (*nextPrevRing)[0]->gi << ")");
                // Set me as NW neighbour to previous hex in the ring:
                (*nextPrevRing)[0]->set_nsw (hi);

            }
            if (i > 0) {
                hi->set_nsw (lasthi);
                DBG2 (" g in-ring: Set me (" << hi->ri << "," << hi->gi << ") as NE neighbour for hex at (" << lasthi->ri << "," << lasthi->gi << ")");
                // Set me as NE neighbour to previous hex in the ring:
                lasthi->set_nne (hi);
            } else {
                // Set my SE neighbour for the first hex in the row.
                hi->set_nse (lasthi);
                DBG2 (" g in-ring: Set me (" << hi->ri << "," << hi->gi << ") as NW neighbour for last walk's hex at (" << lasthi->ri << "," << lasthi->gi << ")");
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
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi << ") as NW neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nnw (hi);
            }
            ++j;
            DBG2 ("i is " << i << ", j is " << j);

            // 3. Set my E neighbour:
            if (j==walkmax) { // We're on the last square and need to
                              // set the East neighbour of the first
                              // hex in the last ring.
                hi->set_ne ((*prevRing)[0]);
                // Set me as W neighbour:
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi << ") as W neighbour for hex at (" << (*prevRing)[0]->ri << "," << (*prevRing)[0]->gi << ")");
                (*prevRing)[0]->set_nw (hi);

            } else if (j<walkmax) {
                hi->set_ne ((*prevRing)[j]);
                // Set me as W neighbour:
                DBG2 (" g walk: Set me (" << hi->ri << "," << hi->gi << ") as W neighbour for hex at (" << (*prevRing)[j]->ri << "," << (*prevRing)[j]->gi << ")");
                (*prevRing)[j]->set_nw (hi);
            }

            // Put in me nextPrevRing:
            nextPrevRing->push_back (hi);
        }
        // Should now be on the last hex.

        // Update the walking increments for finding the vertices of
        // the hexagonal ring. These are for walking around the ring
        // *inside* the ring of hexes being created and hence note
        // that I set walkinc to numInRing/6 BEFORE incrementing
        // numInRing by 6, below.
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
