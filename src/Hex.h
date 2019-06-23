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

using std::string;
using std::to_string;
using std::list;
using std::array;
using std::abs;
using std::sqrt;
using std::pair;
using morph::BezCoord;

#define DEBUG_WITH_COUT 1
#ifdef DEBUG_WITH_COUT
#include <iostream>
using std::cout;
using std::endl;
#endif

/*!
 * Flags
 */
//@{
#define HEX_HAS_NE        0x1
#define HEX_HAS_NNE       0x2
#define HEX_HAS_NNW       0x4
#define HEX_HAS_NW        0x8
#define HEX_HAS_NSW      0x10
#define HEX_HAS_NSE      0x20

// Neighbour positions
#define HEX_NEIGHBOUR_POS_E   0x0
#define HEX_NEIGHBOUR_POS_NE  0x1
#define HEX_NEIGHBOUR_POS_NW  0x2
#define HEX_NEIGHBOUR_POS_W   0x3
#define HEX_NEIGHBOUR_POS_SW  0x4
#define HEX_NEIGHBOUR_POS_SE  0x5

// All hexes marked as boundary hexes, including some that are additional to requirements:
#define HEX_IS_BOUNDARY      0x40
// All hexes inside boundary plus as much of the boundary as needed to make a contiguous boundary:
#define HEX_INSIDE_BOUNDARY  0x80
// All hexes inside the domain of computation:
#define HEX_INSIDE_DOMAIN   0x100
//@}

namespace morph {

    const float SQRT_OF_3_F = 1.73205081;
    /*!
     * Describes a regular hexagon arranged with vertices pointing
     * vertically and two flat sides perpendicular to the horizontal
     * axis:
     *
     *            *
     *         *     *
     *         *     *
     *            *
     *
     * The centre of the hex in a Cartesian right hand coordinate
     * system is represented with x, y and z:
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
     */
    class Hex
    {
    public:
        /*!
         * Constructor taking index, dimension and integer position
         * indices. Computes Cartesian location from these.
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
         * Produce a string containing information about this hex,
         * showing grid location in dimensionless r,g (but not b)
         * units. Also show nearest neighbours.
         */
        string output (void) const {
            string s("Hex ");
            s += to_string(this->vi) + " (";
            s += to_string(this->ri).substr(0,4) + ",";
            s += to_string(this->gi).substr(0,4) + "). ";

            if (this->has_ne) {
                s += "E: (" + to_string(this->ne->ri).substr(0,4) + "," + to_string(this->ne->gi).substr(0,4) + ") " + (this->ne->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->has_nse) {
                s += "SE: (" + to_string(this->nse->ri).substr(0,4) + "," + to_string(this->nse->gi).substr(0,4) + ") " + (this->nse->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->has_nsw) {
                s += "SW: (" + to_string(this->nsw->ri).substr(0,4) + "," + to_string(this->nsw->gi).substr(0,4) + ") " + (this->nsw->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->has_nw) {
                s += "W: (" + to_string(this->nw->ri).substr(0,4) + "," + to_string(this->nw->gi).substr(0,4) + ") " + (this->nw->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->has_nnw) {
                s += "NW: (" + to_string(this->nnw->ri).substr(0,4) + "," + to_string(this->nnw->gi).substr(0,4) + ") " + (this->nnw->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->has_nne) {
                s += "NE: (" + to_string(this->nne->ri).substr(0,4) + "," + to_string(this->nne->gi).substr(0,4) + ") " + (this->nne->boundaryHex == true ? "OB":"") + " ";
            }
            if (this->boundaryHex) {
                s += "(ON boundary)";
            } else  {
                s += "(not boundary)";
            }
            return s;
        }

        /*!
         * Produce a string containing information about this hex,
         * focussing on Cartesian position information.
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
         * Convert the neighbour position number into a short string
         * representing the direction/position of the neighbour.
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

#ifdef UNTESTED_UNUSED
        /*!
         * Change this Hex's position in space. Subtract
         * cartesianPoint from this Hex's position and update all the
         */
        void subtractLocation (const pair<float, float> cartesianPoint) {
            // Compute ri and gi that represent
            // cartesianPoint. Subtract from this->ri and
            // this->gi. Call this->computeLocation().
            float sr = this->getSR();
            float v = this->getV();
            float gi_f = cartesianPoint.second / v;
            float ri_f = cartesianPoint.second / sr + cartesianPoint.first / d;
            int gi_i = round (gi_f);
            int ri_i = round (ri_f);
            cout << "Subtracting ri_i: " << ri_i << " from ri: " << ri << endl;
            cout << "Subtracting gi_i: " << gi_i << " from gi: " << gi << endl;
            this->ri -= ri_i;
            this->gi -= gi_i;
            this->computeLocation();
        }
#endif

        /*!
         * Convert ri, gi and bi indices into x and y coordinates and
         * also r and phi coordinates, based on the hex-to-hex
         * distance d.
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
         * Compute the distance from the point given (in
         * two-dimensions only; x and y) by @a cartesianPoint to the
         * centre of this Hex.
         */
        float distanceFrom (const pair<float, float> cartesianPoint) const {
            float dx = abs(cartesianPoint.first - x);
            float dy = abs(cartesianPoint.second - y);
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Compute the distance from the point given (in
         * two-dimensions only; x and y) by the BezCoord @a
         * cartesianPoint to the centre of this Hex.
         */
        float distanceFrom (const BezCoord& cartesianPoint) const {
            float dx = abs(cartesianPoint.x() - x);
            float dy = abs(cartesianPoint.y() - y);
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Compute the distance from another hex to this one.
         */
        float distanceFrom (const Hex& otherHex) const {
            float dx = abs(otherHex.x - x);
            float dy = abs(otherHex.y - y);
            return sqrt (dx*dx + dy*dy);
        }

        /*!
         * Vector index. This is the index into those data vectors
         * which hold the relevant data pertaining to this hex. This
         * is a scheme which allows me to keep the data in separate
         * vectors and all the hex position information in this class.
         * What happens when I delete some hex elements?  Simple - I
         * can re-set the vi indices after creating a grid of Hex
         * elements and then pruning down.
         */
        unsigned int vi;

        /*!
         * Index into the d_ vectors in HexGrid. Used to populate
         * HexGrid::d_nne, HexGrid::d_nnw, HexGrid::d_nsw and
         * HexGrid::d_nse, etc.
         *
         * This indexes into the d_ vectors in the HexGrid object to
         * which this Hex belongs.
         */
        unsigned int di = 0;

        /*!
         * Cartesian coordinates of the centre of the Hex. Public, for
         * direct access by client code.
         */
        //@{
        float x = 0.0f;
        float y = 0.0f;
        //@}

        /*!
         * Polar coordinates of the centre of the Hex. Public, for
         * direct access by client code.
         */
        //@{
        float r = 0.0f;
        float phi = 0.0f;
        //@}

        /*!
         * Position z of the Hex is common to both Cartesian and Polar
         * coordinate systems.
         */
        float z = 0.0f;

        /*!
         * Get the Cartesian position of this Hex as a fixed size array.
         */
        array<float, 3> position (void) {
            array<float,3> rtn = { { this->x, this->y, this->z } };
            return rtn;
        }

        /*!
         * The centre-to-centre distance from one Hex to an
         * immediately adjacent Hex.
         */
        float d = 1.0f;

        /*!
         * A getter for d, for completeness. d is the centre-to-centre
         * distance between adjacent hexes.
         */
        float getD (void) {
            return this->d;
        }

        /*!
         * Get the shortest distance from the centre to the
         * perimeter. This is the "short radius".
         */
        float getSR (void) {
            return this->d/2;
        }

        /*!
         * The distance from the centre of the Hex to any of the
         * vertices. This is the "long radius".
         */
        float getLR (void) {
            float lr = this->d/morph::SQRT_OF_3_F;
            return lr;
        }

        /*!
         * The vertical distance between hex centres on adjacent rows.
         */
        float getV (void) {
            float v = (this->d*morph::SQRT_OF_3_F)/2.0f;
            return v;
        }

        /*!
         * The vertical distance from the centre of the hex to the
         * "north east" vertex of the hex.
         */
        float getVtoNE (void) {
            float v = this->d/(2.0f*morph::SQRT_OF_3_F);
            return v;
        }

        /*!
         * Return twice the vertical distance between hex centres on
         * adjacent rows.
         */
        float getTwoV (void) {
            float tv = (this->d*morph::SQRT_OF_3_F);
            return tv;
        }

        /*!
         * Indices in hex directions. These lie in the x-y
         * plane. They index in positive and negative directions,
         * starting from the Hex at (0,0,z)
         */
        //@{
        /*!
         * Index in r direction - positive "East", that is in the +x
         * direction.
         */
        int ri = 0;
        /*!
         * Index in g direction - positive "NorthEast". In a direction
         * 30 degrees East of North or 60 degrees North of East.
         */
        int gi = 0;
        /*!
         * Index in b direction - positive "NorthEast". In a direction
         * 30 degrees East of North or 60 degrees North of East.
         */
        int bi = 0;
        //@}

        /*!
         * Get all the flags packed into an unsigned int. Only uses 9
         * bits of the 32 bits available.
         */
        unsigned int getFlags (void) {

            unsigned int flgs = 0x0;

            if (boundaryHex == true) {
                flgs |= HEX_IS_BOUNDARY;
            }
            if (insideBoundary == true) {
                flgs |= HEX_INSIDE_BOUNDARY;
            }
            if (insideDomain == true) {
                flgs |= HEX_INSIDE_DOMAIN;
            }
            if (has_ne == true) {
                flgs |= HEX_HAS_NE;
            }
            if (has_nne == true) {
                flgs |= HEX_HAS_NNE;
            }
            if (has_nnw == true) {
                flgs |= HEX_HAS_NNW;
            }
            if (has_nw == true) {
                flgs |= HEX_HAS_NW;
            }
            if (has_nsw == true) {
                flgs |= HEX_HAS_NSW;
            }
            if (has_nse == true) {
                flgs |= HEX_HAS_NSE;
            }

            return flgs;
        }

        /*!
         * Set to true if this Hex has been marked as being on a
         * boundary. It is expected that client code will then re-set
         * the neighbour relations so that onBoundary() would return
         * true.
         */
        bool boundaryHex = false;

        /*!
         * Set true if this Hex is known to be inside the boundary.
         */
        bool insideBoundary = false;

        /*!
         * Set true if this Hex is known to be inside a
         * rectangular, parallelogram or hexagonal 'domain'.
         */
        bool insideDomain = false;

        /*!
         * This can be populated with the distance to the nearest
         * boundary hex, so that an algorithm can set values in a hex
         * based this metric.
         */
        float distToBoundary = -1.0f;

        /*!
         * Return true if this is a boundary hex - one on the outside
         * edge of a hex grid.
         */
        bool onBoundary() {
            if (this->has_ne == false
                || this->has_nne == false
                || this->has_nnw == false
                || this->has_nw == false
                || this->has_nsw == false
                || this->has_nse == false) {
                return true;
            }
            return false;
        }

        /*!
         * Setters for neighbour iterators
         */
        //@{
        void set_ne (list<Hex>::iterator it) {
            this->ne = it;
            this->has_ne = true;
        }
        void set_nne (list<Hex>::iterator it) {
            this->nne = it;
            this->has_nne = true;
        }
        void set_nnw (list<Hex>::iterator it) {
            this->nnw = it;
            this->has_nnw = true;
        }
        void set_nw (list<Hex>::iterator it) {
            this->nw = it;
            this->has_nw = true;
        }
        void set_nsw (list<Hex>::iterator it) {
            this->nsw = it;
            this->has_nsw = true;
        }
        void set_nse (list<Hex>::iterator it) {
            this->nse = it;
            this->has_nse = true;
        }
        //@}

        /*!
         * Un-set neighbour iterators
         */
        //@{
        void unset_ne (void) {
            this->has_ne = false;
        }
        void unset_nne (void) {
            this->has_nne = false;
        }
        void unset_nnw (void) {
            this->has_nnw = false;
        }
        void unset_nw (void) {
            this->has_nw = false;
        }
        void unset_nsw (void) {
            this->has_nsw = false;
        }
        void unset_nse (void) {
            this->has_nse = false;
        }
        //@}

        /*!
         * Test if have neighbour at position p.
         * East: 0, North-East: 1, North-West: 2
         * West: 3, South-West: 4, South-East: 5
         */
        bool has_neighbour (unsigned short ni) {
            unsigned int flags = this->getFlags();
            switch (ni) {
            case HEX_NEIGHBOUR_POS_E:
            {
                return (flags & HEX_HAS_NE) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_NE:
            {
                return (flags & HEX_HAS_NNE) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_NW:
            {
                return (flags & HEX_HAS_NNW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_W:
            {
                return (flags & HEX_HAS_NW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_SW:
            {
                return (flags & HEX_HAS_NSW) ? true : false;
                break;
            }
            case HEX_NEIGHBOUR_POS_SE:
            {
                return (flags & HEX_HAS_NSE) ? true : false;
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
         * Un-set the pointers on all my neighbours so that THEY no longer point to ME.
         */
        void disconnectNeighbours (void) {
            if (this->has_ne) {
                if (this->ne->has_nw) {
                    this->ne->unset_nw();
                }
            }
            if (this->has_nne) {
                if (this->nne->has_nsw) {
                    this->nne->unset_nsw();
                }
            }
            if (this->has_nnw) {
                if (this->nnw->has_nse) {
                    this->nnw->unset_nse();
                }
            }
            if (this->has_nw) {
                if (this->nw->has_ne) {
                    this->nw->unset_ne();
                }
            }
            if (this->has_nsw) {
                if (this->nsw->has_nne) {
                    this->nsw->unset_nne();
                }
            }
            if (this->has_nse) {
                if (this->nse->has_nnw) {
                    this->nse->unset_nnw();
                }
            }
        }

        //private:??
        /*!
         * Nearest neighbours
         */
        //@{
        /*!
         * Nearest neighbour to the East; in the plus r direction.
         */
        list<Hex>::iterator ne;
        /*!
         * Set true when ne has been set. Use of iterators rather than
         * pointers means we can't do any kind of check to see if the
         * iterator is valid, so we have to keep a separate boolean
         * value.
         */
        bool has_ne = false;

        /*!
         * Nearest neighbour to the NorthEast; in the plus g
         * direction.
         */
        //@{
        list<Hex>::iterator nne;
        bool has_nne = false;
        //@}

        /*!
         * Nearest neighbour to the NorthWest; in the plus b
         * direction.
         */
        //@{
        list<Hex>::iterator nnw;
        bool has_nnw = false;
        //@}

        /*!
         * Nearest neighbour to the West; in the minus r direction.
         */
        //@{
        list<Hex>::iterator nw;
        bool has_nw = false;
        //@}

        /*!
         * Nearest neighbour to the SouthWest; in the minus g
         * direction.
         */
        //@{
        list<Hex>::iterator nsw;
        bool has_nsw = false;
        //@}

        /*!
         * Nearest neighbour to the SouthEast; in the minus b
         * direction.
         */
        //@{
        list<Hex>::iterator nse;
        bool has_nse = false;
        //@}

        //@}
    };

} // namespace morph

#endif // _HEX_H_
