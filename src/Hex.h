/*
 * Author: Seb James
 *
 * Date: 2018/07
 */

#ifndef _HEX_H_
#define _HEX_H_

#include <string>
#include <list>
#include <array>
#include <utility>
#include <cmath>
#include "BezCoord.h"
#include "HdfData.h"
#include "MathConst.h"

using std::string;
using std::to_string;
using std::list;
using std::array;
using std::abs;
using std::sqrt;
using std::pair;
using morph::BezCoord;
using morph::HdfData;

//#define DEBUG_WITH_COUT 1
#ifdef DEBUG_WITH_COUT
#include <iostream>
using std::cout;
using std::endl;
#endif

/*!
 * Flags
 */
//@{
/*!
 * Set true when ne has been set. Use of iterators (Hex::ne etc) rather than pointers for
 * neighbouring hexes means we can't do any kind of check to see if the iterator is valid, so we
 * have to keep separate boolean flags for whether or not each Hex has a neighbour. Those flags are
 * kept in Hex::flags.
 */

#define HEX_HAS_NE                0x1
#define HEX_HAS_NNE               0x2
#define HEX_HAS_NNW               0x4
#define HEX_HAS_NW                0x8
#define HEX_HAS_NSW              0x10
#define HEX_HAS_NSE              0x20
#define HEX_HAS_NEIGHB_ALL       0x3f // HEX_HAS_NE | HEX_HAS_NNE | ...etc

//! All hexes marked as boundary hexes, including some that are additional to requirements:
#define HEX_IS_BOUNDARY          0x40
//! All hexes inside boundary plus as much of the boundary as needed to make a contiguous boundary:
#define HEX_INSIDE_BOUNDARY      0x80
//! All hexes inside the domain of computation:
#define HEX_INSIDE_DOMAIN       0x100
//! Hex is a 'region boundary hex'. Regions are intended to be temporary to aid client code.
#define HEX_IS_REGION_BOUNDARY  0x200
//! Hex is inside the region
#define HEX_INSIDE_REGION       0x400

//! Four flags for client code to use for its own devices. For an example of use, see DirichDom.h
//@{
#define HEX_USER_FLAG_0    0x10000000
#define HEX_USER_FLAG_1    0x20000000
#define HEX_USER_FLAG_2    0x40000000
#define HEX_USER_FLAG_3    0x80000000
//@}
//! Four bits high: all user flags set
#define HEX_ALL_USER       0xf0000000
//! Bitmask for all the flags that aren't the 4 user flags.
#define HEX_NON_USER       0x0fffffff
//@} // Flags

//! Neighbour (or edge, or side) positions
//@{
#define HEX_NEIGHBOUR_POS_E       0x0
#define HEX_NEIGHBOUR_POS_NE      0x1
#define HEX_NEIGHBOUR_POS_NW      0x2
#define HEX_NEIGHBOUR_POS_W       0x3
#define HEX_NEIGHBOUR_POS_SW      0x4
#define HEX_NEIGHBOUR_POS_SE      0x5
//@}

//! Vertex positions
//@{
#define HEX_VERTEX_POS_NE     0x0
#define HEX_VERTEX_POS_N      0x1
#define HEX_VERTEX_POS_NW     0x2
#define HEX_VERTEX_POS_SW     0x3
#define HEX_VERTEX_POS_S      0x4
#define HEX_VERTEX_POS_SE     0x5
//@}

namespace morph {

    /*!
     * Describes a regular hexagon arranged with vertices pointing vertically and two flat sides
     * perpendicular to the horizontal axis:
     *
     *            *
     *         *     *
     *         *     *
     *            *
     *
     * The centre of the hex in a Cartesian right hand coordinate system is represented with x, y
     * and z:
     *
     *  y
     *  ^
     *  |
     *  |
     *  0-----> x     z out of screen/page
     *
     * Directions are "r" "g" and "b" and their negatives:
     *
     *         b  * g
     * -r <--  *     * ---> r
     *         *     *
     *         -g * -b
     *
     *
     * I've defined numbering for the Hex's vertices and for its edges.
     *
     * Vertices: NE: 0, N: 1, NW: 2, SW: 3, S: 4, SE: 5.
     *
     * Edges/Sides: East: 0, North-East: 1, North-West: 2 West: 3, South-West: 4, South-East: 5
     */
    class Hex
    {
    public:
        /*!
         * Constructor taking index, dimension and integer position indices. Computes Cartesian
         * location from these.
         */
        Hex (const unsigned int& idx, const float& d_,
             const int& r_, const int& g_) {
            this->vi = idx;
            this->d = d_;
            this->ri = r_;
            this->gi = g_;
            this->computeLocation();
        }

        /*!
         * Construct using the passed in HDF5 file and path.
         */
        Hex (HdfData& h5data, const string& h5path) {
            this->load (h5data, h5path);
        }

        /*!
         * Comparison operation to enable use of set<Hex>
         */
        bool operator< (const Hex& rhs) const {
            // Compare position first.
            if (this->x < rhs.x) {
                return true;
            }
            if (this->x > rhs.x) {
                return false;
            }
            if (this->y < rhs.y) {
                return true;
            }
            if (this->y > rhs.y) {
                return false;
            }
            // If position can't differentiate, compare vector index
            if (this->vi < rhs.vi) {
                return true;
            }
            #if 0
            if (this->vi > rhs.vi) {
                return false;
            }
            #endif
            return false;
        }

        /*!
         * Save the data for this Hex into the already open HdfData object @h5data in the path
         * @h5path.
         */
        void save (HdfData& h5data, const string& h5path) const {
            string dpath = h5path + "/vi";
            h5data.add_val (dpath.c_str(), this->vi);
            dpath = h5path + "/di";
            h5data.add_val (dpath.c_str(), this->di);
            dpath = h5path + "/x";
            h5data.add_val (dpath.c_str(), this->x);
            dpath = h5path + "/y";
            h5data.add_val (dpath.c_str(), this->y);
            dpath = h5path + "/z";
            h5data.add_val (dpath.c_str(), this->z);
            dpath = h5path + "/r";
            h5data.add_val (dpath.c_str(), this->r);
            dpath = h5path + "/phi";
            h5data.add_val (dpath.c_str(), this->phi);
            dpath = h5path + "/d";
            h5data.add_val (dpath.c_str(), this->d);
            dpath = h5path + "/ri";
            h5data.add_val (dpath.c_str(), this->ri);
            dpath = h5path + "/gi";
            h5data.add_val (dpath.c_str(), this->gi);
            dpath = h5path + "/bi";
            h5data.add_val (dpath.c_str(), this->bi);
            dpath = h5path + "/distToBoundary";
            h5data.add_val (dpath.c_str(), this->distToBoundary);
            dpath = h5path + "/flags";
            h5data.add_val (dpath.c_str(), this->flags);
        }

        void load (HdfData& h5data, const string& h5path) {
            string dpath = h5path + "/vi";
            h5data.read_val (dpath.c_str(), this->vi);
            dpath = h5path + "/di";
            h5data.read_val (dpath.c_str(), this->di);
            dpath = h5path + "/x";
            h5data.read_val (dpath.c_str(), this->x);
            dpath = h5path + "/y";
            h5data.read_val (dpath.c_str(), this->y);
            dpath = h5path + "/z";
            h5data.read_val (dpath.c_str(), this->z);
            dpath = h5path + "/r";
            h5data.read_val (dpath.c_str(), this->r);
            dpath = h5path + "/phi";
            h5data.read_val (dpath.c_str(), this->phi);
            dpath = h5path + "/d";
            h5data.read_val (dpath.c_str(), this->d);
            dpath = h5path + "/ri";
            h5data.read_val (dpath.c_str(), this->ri);
            dpath = h5path + "/gi";
            h5data.read_val (dpath.c_str(), this->gi);
            dpath = h5path + "/bi";
            h5data.read_val (dpath.c_str(), this->bi);
            dpath = h5path + "/distToBoundary";
            h5data.read_val (dpath.c_str(), this->distToBoundary);
            unsigned int flgs = 0;
            dpath = h5path + "/flags";
            h5data.read_val (dpath.c_str(), flgs);
            this->flags = flgs;
        }

        /*!
         * Produce a string containing information about this hex, showing grid location in
         * dimensionless r,g (but not b) units. Also show nearest neighbours.
         */
        string output (void) const {
            string s("Hex ");
            s += to_string(this->vi) + " (";
            s += to_string(this->ri).substr(0,4) + ",";
            s += to_string(this->gi).substr(0,4) + "). ";

            if (this->has_ne()) {
                s += "E: (" + to_string(this->ne->ri).substr(0,4) + "," + to_string(this->ne->gi).substr(0,4) + ") " + (this->ne->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nse()) {
                s += "SE: (" + to_string(this->nse->ri).substr(0,4) + "," + to_string(this->nse->gi).substr(0,4) + ") " + (this->nse->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nsw()) {
                s += "SW: (" + to_string(this->nsw->ri).substr(0,4) + "," + to_string(this->nsw->gi).substr(0,4) + ") " + (this->nsw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nw()) {
                s += "W: (" + to_string(this->nw->ri).substr(0,4) + "," + to_string(this->nw->gi).substr(0,4) + ") " + (this->nw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nnw()) {
                s += "NW: (" + to_string(this->nnw->ri).substr(0,4) + "," + to_string(this->nnw->gi).substr(0,4) + ") " + (this->nnw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nne()) {
                s += "NE: (" + to_string(this->nne->ri).substr(0,4) + "," + to_string(this->nne->gi).substr(0,4) + ") " + (this->nne->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->boundaryHex()) {
                s += "(ON boundary)";
            } else  {
                s += "(not boundary)";
            }
            return s;
        }

        /*!
         * Produce a string containing information about this hex, focussing on Cartesian position
         * information.
         */
        string outputCart (void) const {
            string s("Hex ");
            s += to_string(this->vi).substr(0,2) + " (";
            s += to_string(this->ri).substr(0,4) + ",";
            s += to_string(this->gi).substr(0,4) + ") is at (x,y) = ("
                + to_string(this->x).substr(0,4) +"," + to_string(this->y).substr(0,4) + ")";
            return s;
        }

        /*!
         * Output "(x,y)" coordinate string
         */
        string outputXY (void) const {
            string s("(");
            s += to_string(this->x).substr(0,4) + "," + to_string(this->y).substr(0,4) + ")";
            return s;
        }

        /*!
         * Output a string containing just "RG(ri, gi)"
         */
        string outputRG (void) const {
            string s("RG(");
            s += to_string(this->ri).substr(0,4) + ",";
            s += to_string(this->gi).substr(0,4) + ")";
            return s;
        }

        /*!
         * Convert the neighbour position number into a short string representing the
         * direction/position of the neighbour.
         */
        static string neighbour_pos (unsigned short dir) {
            string s("");
            switch (dir) {
            case HEX_NEIGHBOUR_POS_E:
            {
                s = "E";
                break;
            }
            case HEX_NEIGHBOUR_POS_NE:
            {
                s = "NE";
                break;
            }
            case HEX_NEIGHBOUR_POS_NW:
            {
                s = "NW";
                break;
            }
            case HEX_NEIGHBOUR_POS_W:
            {
                s = "W";
                break;
            }
            case HEX_NEIGHBOUR_POS_SW:
            {
                s = "SW";
                break;
            }
            case HEX_NEIGHBOUR_POS_SE:
            {
                s = "SE";
                break;
            }
            };
            return s;
        }

        /*!
         * Convert ri, gi and bi indices into x and y coordinates and also r and phi coordinates,
         * based on the hex-to-hex distance d.
         */
        void computeLocation (void) {
            // Compute Cartesian location
            this->x = this->d*this->ri + (d/2.0f)*this->gi - (d/2.0f)*this->bi;
            float v = this->getV();
            this->y = v*this->gi + v*this->bi;
            // And location in the Polar coordinate system
            this->r = sqrt (x*x + y*y);
            this->phi = atan2 (y, x);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y) by @a
         * cartesianPoint to the centre of this Hex.
         */
        template <typename LFlt>
        float distanceFrom (const pair<LFlt, LFlt> cartesianPoint) const {
            float dx = cartesianPoint.first - x;
            float dy = cartesianPoint.second - y;
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y) by the
         * BezCoord @a cartesianPoint to the centre of this Hex.
         */
        float distanceFrom (const BezCoord& cartesianPoint) const {
            float dx = cartesianPoint.x() - x;
            float dy = cartesianPoint.y() - y;
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Compute the distance from another hex to this one.
         */
        float distanceFrom (const Hex& otherHex) const {
            float dx = otherHex.x - x;
            float dy = otherHex.y - y;
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Vector index. This is the index into those data vectors which hold the relevant data
         * pertaining to this hex. This is a scheme which allows me to keep the data in separate
         * vectors and all the hex position information in this class.  What happens when I delete
         * some hex elements?  Simple - I can re-set the vi indices after creating a grid of Hex
         * elements and then pruning down.
         */
        unsigned int vi;

        /*!
         * Index into the d_ vectors in HexGrid. Used to populate HexGrid::d_nne, HexGrid::d_nnw,
         * HexGrid::d_nsw and HexGrid::d_nse, etc.
         *
         * This indexes into the d_ vectors in the HexGrid object to which this Hex belongs. The d_
         * vectors are ordered differently from the list<Hex> object in HexGrid::hexen and hence we
         * have this attribute di in addition to the vector index vi, which provides an index into
         * list<Hex> or vector<Hex> objects which either are, or are arranged like, HexGrid::hexen
         */
        unsigned int di = 0;

        /*!
         * Cartesian coordinates of the centre of the Hex. Public, for direct access by client code.
         */
        //@{
        float x = 0.0f;
        float y = 0.0f;
        //@}

        /*!
         * Polar coordinates of the centre of the Hex. Public, for direct access by client code.
         */
        //@{
        float r = 0.0f;
        float phi = 0.0f;
        //@}

        /*!
         * Position z of the Hex is common to both Cartesian and Polar coordinate systems.
         */
        float z = 0.0f;

        /*!
         * Get the Cartesian position of this Hex as a fixed size array.
         */
        array<float, 3> position (void) const {
            array<float,3> rtn = { { this->x, this->y, this->z } };
            return rtn;
        }

        /*!
         * The centre-to-centre distance from one Hex to an immediately adjacent Hex.
         */
        float d = 1.0f;

        /*!
         * A getter for d, for completeness. d is the centre-to-centre distance between adjacent
         * hexes.
         */
        float getD (void) const {
            return this->d;
        }

        /*!
         * Get the shortest distance from the centre to the perimeter. This is the "short radius".
         */
        float getSR (void) const {
            return this->d/2;
        }

        /*!
         * The distance from the centre of the Hex to any of the vertices. This is the "long
         * radius".
         */
        float getLR (void) const {
            float lr = this->d/morph::SQRT_OF_3_F;
            return lr;
        }

        /*!
         * The vertical distance between hex centres on adjacent rows.
         */
        float getV (void) const {
            float v = (this->d*morph::SQRT_OF_3_F)/2.0f;
            return v;
        }

        /*!
         * The vertical distance from the centre of the hex to the "north east" vertex of the hex.
         */
        float getVtoNE (void) const {
            float v = this->d/(2.0f*morph::SQRT_OF_3_F);
            return v;
        }

        /*!
         * Return twice the vertical distance between hex centres on adjacent rows.
         */
        float getTwoV (void) const {
            float tv = (this->d*morph::SQRT_OF_3_F);
            return tv;
        }

        /*!
         * Indices in hex directions. These lie in the x-y plane. They index in positive and
         * negative directions, starting from the Hex at (0,0,z)
         */
        //@{
        /*!
         * Index in r direction - positive "East", that is in the +x direction.
         */
        int ri = 0;
        /*!
         * Index in g direction - positive "NorthEast". In a direction 30 degrees East of North or
         * 60 degrees North of East.
         */
        int gi = 0;
        /*!
         * Index in b direction - positive "NorthEast". In a direction 30 degrees East of North or
         * 60 degrees North of East.
         */
        int bi = 0;
        //@}

        /*!
         * Getter for this->flags
         */
        unsigned int getFlags (void) const {
            return this->flags;
        }

        /*!
         * Setter for this->flags
         */
        void setFromFlags (unsigned int flgs) {
            this->flags = flgs;
        }
        void setFlags (unsigned int flgs) {
            this->flags = flgs;
        }
        /*!
         * Set one or more flags, defined by flg, true
         */
        void setFlag (unsigned int flg) {
            this->flags |= flg;
        }
        /*!
         * Unset one or more flags, defined by flg, i.e. set false
         */
        void unsetFlag (unsigned int flg) {
            this->flags &= ~(flg);
        }
        /*!
         * If flags match flg, then return true
         */
        bool testFlags (unsigned int flg) const {
            return (this->flags & flg) == flg ? true : false;
        }

        /*!
         * Set to true if this Hex has been marked as being on a boundary. It is expected that
         * client code will then re-set the neighbour relations so that onBoundary() would return
         * true.
         */
        bool boundaryHex (void) const {
            return this->flags & HEX_IS_BOUNDARY ? true : false;
        }
        /*!
         * Mark the hex as a boundary hex. Boundary hexes are also, by definition, inside the
         * boundary.
         */
        void setBoundaryHex (void) {
            this->flags |= (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
        }
        void unsetBoundaryHex (void) {
            this->flags &= ~(HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY);
        }

        /*!
         * Set true if this Hex is known to be inside the boundary.
         */
        bool insideBoundary (void) const {
            return this->flags & HEX_INSIDE_BOUNDARY ? true : false;
        }
        void setInsideBoundary (void) {
            this->flags |= HEX_INSIDE_BOUNDARY;
        }
        void unsetInsideBoundary (void) {
            this->flags &= ~HEX_INSIDE_BOUNDARY;
        }

        /*!
         * Set true if this Hex is known to be inside a rectangular, parallelogram or hexagonal
         * 'domain'.
         */
        bool insideDomain (void) const {
            return this->flags & HEX_INSIDE_DOMAIN ? true : false;
        }
        void setInsideDomain (void) {
            this->flags |= HEX_INSIDE_DOMAIN;
        }
        void unsetInsideDomain (void) {
            this->flags &= ~HEX_INSIDE_DOMAIN;
        }

        /*!
         * Set the HEX_USER_FLAG_0/1/2/3 from the passed in unsigned int.
         *
         * E.g. hex->setUserFlags (HEX_USER_FLAG_0 | HEX_USER_FLAG_1);
         *
         * This will set HEX_USER_FLAG_0 and HEX_USER_FLAG_1 AND UNSET HEX_USER_FLAG_2 &
         * HEX_USER_FLAG_3.
         */
        void setUserFlags (unsigned int uflgs) {
            this->flags |= (uflgs & HEX_ALL_USER);
        }

        /*!
         * Set the single user flag 0, 1 2 or 3 as given by the passed-in unsigned int uflg_num.
         */
        void setUserFlag (unsigned int uflg_num) {
            unsigned int flg = 0x1UL << (28+uflg_num);
            this->flags |= flg;
        }

        /*!
         * Un-setter corresponding to setUserFlag(unsigned int)
         */
        void unsetUserFlag (unsigned int uflg_num) {
            unsigned int flg = 0x1UL << (28+uflg_num);
            this->flags &= ~flg;
        }

        /*!
         * Set all user flags to the unset state
         */
        void resetUserFlags (void) {
            this->flags &= HEX_NON_USER; // or ~HEX_ALL_USER;
        }

        /*!
         * Getter for each user flag
         */
        bool getUserFlag (unsigned int uflg_num) const {
            unsigned int flg = 0x1UL << (28+uflg_num);
            return ((this->flags & flg) == flg);
        }

        /*!
         * This can be populated with the distance to the nearest boundary hex, so that an algorithm
         * can set values in a hex based this metric.
         */
        float distToBoundary = -1.0f;

        /*!
         * Return true if this is a boundary hex - one on the outside edge of a hex grid. The result
         * is based on testing neihgbour relations, rather than examining the value of the
         * HEX_IS_BOUNDARY flag.
         */
        bool onBoundary() {
            return ((this->flags & HEX_HAS_NEIGHB_ALL) == HEX_HAS_NEIGHB_ALL) ? false : true;
        }

        /*!
         * Setters for neighbour iterators
         */
        //@{
        void set_ne (list<Hex>::iterator it) {
            this->ne = it;
            this->flags |= HEX_HAS_NE;
        }
        void set_nne (list<Hex>::iterator it) {
            this->nne = it;
            this->flags |= HEX_HAS_NNE;
        }
        void set_nnw (list<Hex>::iterator it) {
            this->nnw = it;
            this->flags |= HEX_HAS_NNW;
        }
        void set_nw (list<Hex>::iterator it) {
            this->nw = it;
            this->flags |= HEX_HAS_NW;
        }
        void set_nsw (list<Hex>::iterator it) {
            this->nsw = it;
            this->flags |= HEX_HAS_NSW;
        }
        void set_nse (list<Hex>::iterator it) {
            this->nse = it;
            this->flags |= HEX_HAS_NSE;
        }
        //@}

        //! Replacing bool members has_ne, has_nne etc.
        bool has_ne (void) const {
            return ((this->flags & HEX_HAS_NE) == HEX_HAS_NE);
        }
        bool has_nne (void) const {
            return ((this->flags & HEX_HAS_NNE) == HEX_HAS_NNE);
        }
        bool has_nnw (void) const {
            return ((this->flags & HEX_HAS_NNW) == HEX_HAS_NNW);
        }
        bool has_nw (void) const {
            return ((this->flags & HEX_HAS_NW) == HEX_HAS_NW);
        }
        bool has_nsw (void) const {
            return ((this->flags & HEX_HAS_NSW) == HEX_HAS_NSW);
        }
        bool has_nse (void) const {
            return ((this->flags & HEX_HAS_NSE) == HEX_HAS_NSE);
        }

        /*!
         * Un-set neighbour iterators
         */
        //@{
        void unset_ne (void) {
            this->flags ^= HEX_HAS_NE;
        }
        void unset_nne (void) {
            this->flags ^= HEX_HAS_NNE;
        }
        void unset_nnw (void) {
            this->flags ^= HEX_HAS_NNW;
        }
        void unset_nw (void) {
            this->flags ^= HEX_HAS_NW;
        }
        void unset_nsw (void) {
            //this->has_nsw = false;
            this->flags ^= HEX_HAS_NSW;
        }
        void unset_nse (void) {
            this->flags ^= HEX_HAS_NSE;
        }
        //@}

        /*!
         * Test if have neighbour at position p.
         * East: 0, North-East: 1, North-West: 2, West: 3, South-West: 4, South-East: 5
         */
        bool has_neighbour (unsigned short ni) {
            switch (ni) {
            case HEX_NEIGHBOUR_POS_E:
            {
                return (this->flags & HEX_HAS_NE) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_NE:
            {
                return (this->flags & HEX_HAS_NNE) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_NW:
            {
                return (this->flags & HEX_HAS_NNW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_W:
            {
                return (this->flags & HEX_HAS_NW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_SW:
            {
                return (this->flags & HEX_HAS_NSW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_SE:
            {
                return (this->flags & HEX_HAS_NSE) ? true : false;
                break;
            }
            default:
            {
                break;
            }
            }
            return false;
        }

        list<Hex>::iterator get_neighbour (unsigned short ni) {
            list<Hex>::iterator hi;
            switch (ni) {
            case HEX_NEIGHBOUR_POS_E:
            {
                hi = this->ne;
                break;
            }
            case HEX_NEIGHBOUR_POS_NE:
            {
                hi = this->nne;
                break;
            }
            case HEX_NEIGHBOUR_POS_NW:
            {
                hi = this->nnw;
                break;
            }
            case HEX_NEIGHBOUR_POS_W:
            {
                hi = this->nw;
                break;
            }
            case HEX_NEIGHBOUR_POS_SW:
            {
                hi = this->nsw;
                break;
            }
            case HEX_NEIGHBOUR_POS_SE:
            {
                hi = this->nse;
                break;
            }
            default:
            {
                break;
            }
            }
            return hi;
        }

        /*!
         * Turn the vertex index into a string name
         */
        static string vertex_name (unsigned short ni) {
            string s("");
            switch (ni) {
            case HEX_VERTEX_POS_NE:
            {
                s = "NE";
                break;
            }
            case HEX_VERTEX_POS_N:
            {
                s = "N";
                break;
            }
            case HEX_VERTEX_POS_NW:
            {
                s = "NW";
                break;
            }
            case HEX_VERTEX_POS_SW:
            {
                s = "SW";
                break;
            }
            case HEX_VERTEX_POS_S:
            {
                s = "S";
                break;
            }
            case HEX_VERTEX_POS_SE:
            {
                s = "SE";
                break;
            }
            default:
            {
                break;
            }
            }
            return s;
        }

        /*!
         * Get the Cartesian coordinates of the given vertex of the Hex. The Hex has a north vertex,
         * a north east vertex and vertices for SE, S, SW and NW. The single argument @ni specifies
         * which vertex to return the coordinate for. Use the definitions HEX_VERTEX_POS_N, etc to
         * pass in a human-readable label for the vertex.
         */
        pair<float, float> get_vertex_coord (unsigned short ni) const {
            pair<float, float> rtn = {0.0, 0.0};
            switch (ni) {
            case HEX_VERTEX_POS_NE:
            {
                rtn.first = this->x + this->getSR();
                rtn.second = this->y + this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_N:
            {
                rtn.first = this->x;
                rtn.second = this->y + this->getLR();
                break;
            }
            case HEX_VERTEX_POS_NW:
            {
                rtn.first = this->x - this->getSR();
                rtn.second = this->y + this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_SW:
            {
                rtn.first = this->x - this->getSR();
                rtn.second = this->y - this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_S:
            {
                rtn.first = this->x;
                rtn.second = this->y - this->getLR();
                break;
            }
            case HEX_VERTEX_POS_SE:
            {
                rtn.first = this->x + this->getSR();
                rtn.second = this->y - this->getVtoNE();
                break;
            }
            default:
            {
                rtn.first = -1.0f;
                rtn.second = -1.0f;
                break;
            }
            }
            return rtn;
        }

        pair<float, float> get_vertex_coord (unsigned int ni) const {
            pair<float, float> rtn = {-2.0, -2.0};
            if (ni > 6) {
                return rtn;
            }
            rtn = this->get_vertex_coord (static_cast<unsigned short> (ni));
            return rtn;
        }

        pair<float, float> get_vertex_coord (int ni) const {
            pair<float, float> rtn = {-3.0, -3.0};
            if (ni > 6) {
                rtn.first = -4.0f;
                return rtn;
            }
            if (ni < 0) {
                rtn.second = -4.0f;
                return rtn;
            }
            rtn = this->get_vertex_coord (static_cast<unsigned short> (ni));
            return rtn;
        }

        /*!
         * Return true if coord is reasonably close to being in the same location as the vertex at
         * vertex ni with the distance threshold being set from the Hex to Hex spacing. This is for
         * distinguishing between vertices and hex centres on a HexGrid.
         */
        template <typename LFlt>
        bool compare_vertex_coord (int ni, pair<LFlt, LFlt>& coord) const {
            pair<float, float> vc = this->get_vertex_coord (ni);
            if (abs(vc.first - coord.first) < this->d/100
                && abs(vc.second - coord.second) < this->d/100) {
                return true;
            }
            return false;
        }

        /*!
         * Return true if the Hex contains the vertex
         */
        template <typename LFlt>
        bool contains_vertex (pair<LFlt, LFlt>& coord) const {
            // check each of my vertices, if any match coord, then return true.
            bool rtn = false;
            for (unsigned int ni = 0; ni < 6; ++ni) {
                if (this->compare_vertex_coord (ni, coord) == true) {
                    rtn = true;
                    break;
                }
            }
            return rtn;
        }

        /*!
         * Return true if coord is reasonably close to being in the same location as the centre of
         * the Hex, with the distance threshold being set from the Hex to Hex spacing. This is for
         * distinguishing between vertices and hex centres on a HexGrid.
         */
        template <typename LFlt>
        bool compare_coord (pair<LFlt, LFlt>& coord) const {
            if (abs(this->x - coord.first) < this->d/100
                && abs(this->y - coord.second) < this->d/100) {
                return true;
            }
            return false;
        }

        /*!
         * Un-set the pointers on all my neighbours so that THEY no longer point to ME.
         */
        void disconnectNeighbours (void) {
#if 0
            // FIXME Could be 6 stanzas like this to avoid the nested ifs
            if ((this->flags & (HEX_HAS_NE | HEX_HAS_NW)) == (HEX_HAS_NE | HEX_HAS_NW)) {
                this->ne->unset_nw();
            }
#endif
            if (this->has_ne()) {
                if (this->ne->has_nw()) {
                    this->ne->unset_nw();
                }
            }
            if (this->has_nne()) {
                if (this->nne->has_nsw()) {
                    this->nne->unset_nsw();
                }
            }
            if (this->has_nnw()) {
                if (this->nnw->has_nse()) {
                    this->nnw->unset_nse();
                }
            }
            if (this->has_nw()) {
                if (this->nw->has_ne()) {
                    this->nw->unset_ne();
                }
            }
            if (this->has_nsw()) {
                if (this->nsw->has_nne()) {
                    this->nsw->unset_nne();
                }
            }
            if (this->has_nse()) {
                if (this->nse->has_nnw()) {
                    this->nse->unset_nnw();
                }
            }
        }

        /*!
         * Nearest neighbours
         */
        //@{
        /*!
         * Nearest neighbour to the East; in the plus r direction.
         */
        list<Hex>::iterator ne;

        /*!
         * Nearest neighbour to the NorthEast; in the plus g direction.
         */
        list<Hex>::iterator nne;

        /*!
         * Nearest neighbour to the NorthWest; in the plus b direction.
         */
        list<Hex>::iterator nnw;

        /*!
         * Nearest neighbour to the West; in the minus r direction.
         */
        list<Hex>::iterator nw;

        /*!
         * Nearest neighbour to the SouthWest; in the minus g direction.
         */
        list<Hex>::iterator nsw;

        /*!
         * Nearest neighbour to the SouthEast; in the minus b direction.
         */
        list<Hex>::iterator nse;
        //@}

    private:
        /*!
         * The flags for this Hex.
         */
        unsigned int flags = 0x0;
    };

} // namespace morph

#endif // _HEX_H_
