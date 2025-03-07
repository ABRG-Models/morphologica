/*
 * \file
 *
 * Defines a class to manage a hexagon which lives in a grid of hexagons.
 *
 * \author: Seb James
 * \date: 2018/07
 */
#pragma once

#include <string>
#include <list>
#include <array>
#include <cmath>
#include <morph/BezCoord.h>
// If the HexGrid::save and HexGrid::load methods are required, define
// HEXGRID_COMPILE_LOAD_AND_SAVE. A link to libhdf5 will be required in your program.
#ifdef HEXGRID_COMPILE_LOAD_AND_SAVE
# include <morph/HdfData.h>
#endif
#include <morph/mathconst.h>
#include <morph/vec.h>
//#define DEBUG_WITH_COUT 1
#ifdef DEBUG_WITH_COUT
#include <iostream>
#endif

/*
 * Flags
 */

/*!
 * Set true when ne has been set. Use of iterators (Hex::ne etc) rather than pointers for
 * neighbouring hexes means we can't do any kind of check to see if the iterator is valid, so we
 * have to keep separate boolean flags for whether or not each Hex has a neighbour. Those flags are
 * kept in Hex::flags.
 */
#define HEX_HAS_NE                0x1
//! True when this hex has a Neighbour to the North East
#define HEX_HAS_NNE               0x2
//! True when this hex has a Neighbour to the North West
#define HEX_HAS_NNW               0x4
//! True when this hex has a Neighbour to the West
#define HEX_HAS_NW                0x8
//! True when this hex has a Neighbour to the South West
#define HEX_HAS_NSW              0x10
//! True when this hex has a Neighbour to the South East
#define HEX_HAS_NSE              0x20
//! A short cut for testing all the neighbour flags at once
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
#define HEX_USER_FLAG_0    0x10000000
#define HEX_USER_FLAG_1    0x20000000
#define HEX_USER_FLAG_2    0x40000000
#define HEX_USER_FLAG_3    0x80000000
//! Four bits high: all user flags set
#define HEX_ALL_USER       0xf0000000
//! Bitmask for all the flags that aren't the 4 user flags.
#define HEX_NON_USER       0x0fffffff

//! Neighbour (or edge, or side) positions
#define HEX_NEIGHBOUR_POS_E       0x0
#define HEX_NEIGHBOUR_POS_NE      0x1
#define HEX_NEIGHBOUR_POS_NW      0x2
#define HEX_NEIGHBOUR_POS_W       0x3
#define HEX_NEIGHBOUR_POS_SW      0x4
#define HEX_NEIGHBOUR_POS_SE      0x5

//! Vertex positions
#define HEX_VERTEX_POS_NE     0x0
#define HEX_VERTEX_POS_N      0x1
#define HEX_VERTEX_POS_NW     0x2
#define HEX_VERTEX_POS_SW     0x3
#define HEX_VERTEX_POS_S      0x4
#define HEX_VERTEX_POS_SE     0x5

namespace morph {

    /*!
     * Describes a regular hexagon arranged with vertices pointing vertically and two flat sides
     * perpendicular to the horizontal axis:
     *\code{.unparsed}
     *            *
     *         *     *
     *         *     *
     *            *
     *\endcode
     * The centre of the hex in a Cartesian right hand coordinate system is represented with x, y
     * and z:
     *\code{.unparsed}
     *  y
     *  ^
     *  |
     *  |
     *  0-----> x     z out of screen/page
     *\endcode
     * Directions are "r" "g" and "b" and their negatives:
     *\code{.unparsed}
     *         b  * g
     * -r <--  *     * ---> r
     *         *     *
     *         -g * -b
     *\endcode
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
        Hex (const unsigned int& idx, const float& d_, const int& r_, const int& g_)
        {
            this->vi = idx;
            this->d = d_;
            this->ri = r_;
            this->gi = g_;
            this->computeLocation();
        }

#ifdef HEXGRID_COMPILE_LOAD_AND_SAVE
        //! Construct using the passed in HDF5 file and path.
        Hex (HdfData& h5data, const std::string& h5path) { this->load (h5data, h5path); }
#endif
        //! Comparison operation to enable use of set<Hex>
        bool operator< (const Hex& rhs) const
        {
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

#ifdef HEXGRID_COMPILE_LOAD_AND_SAVE
        /*!
         * Save the data for this Hex into the already open HdfData object @h5data in the path
         * @h5path.
         */
        void save (HdfData& h5data, const std::string& h5path) const
        {
            std::string dpath = h5path + "/vi";
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

        //! Load the data for this Hex from a morph::HdfData file
        void load (HdfData& h5data, const std::string& h5path)
        {
            std::string dpath = h5path + "/vi";
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
#endif
        /*!
         * Produce a string containing information about this hex, showing grid location in
         * dimensionless r,g (but not b) units. Also show nearest neighbours.
         */
        std::string output() const
        {
            std::string s("Hex ");
            s += std::to_string(this->vi) + " (";
            s += std::to_string(this->ri).substr(0,4) + ",";
            s += std::to_string(this->gi).substr(0,4) + "). ";

            if (this->has_ne()) {
                s += "E: (" + std::to_string(this->ne->ri).substr(0,4) + "," + std::to_string(this->ne->gi).substr(0,4) + ") " + (this->ne->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nse()) {
                s += "SE: (" + std::to_string(this->nse->ri).substr(0,4) + "," + std::to_string(this->nse->gi).substr(0,4) + ") " + (this->nse->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nsw()) {
                s += "SW: (" + std::to_string(this->nsw->ri).substr(0,4) + "," + std::to_string(this->nsw->gi).substr(0,4) + ") " + (this->nsw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nw()) {
                s += "W: (" + std::to_string(this->nw->ri).substr(0,4) + "," + std::to_string(this->nw->gi).substr(0,4) + ") " + (this->nw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nnw()) {
                s += "NW: (" + std::to_string(this->nnw->ri).substr(0,4) + "," + std::to_string(this->nnw->gi).substr(0,4) + ") " + (this->nnw->boundaryHex() == true ? "OB":"") + " ";
            }
            if (this->has_nne()) {
                s += "NE: (" + std::to_string(this->nne->ri).substr(0,4) + "," + std::to_string(this->nne->gi).substr(0,4) + ") " + (this->nne->boundaryHex() == true ? "OB":"") + " ";
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
        std::string outputCart() const
        {
            std::string s("Hex ");
            s += std::to_string(this->vi).substr(0,2) + " (";
            s += std::to_string(this->ri).substr(0,4) + ",";
            s += std::to_string(this->gi).substr(0,4) + ") is at (x,y) = ("
                + std::to_string(this->x).substr(0,4) +"," + std::to_string(this->y).substr(0,4) + ")";
            return s;
        }

        //! Output "(x,y)" coordinate string
        std::string outputXY() const
        {
            std::string s("(");
            s += std::to_string(this->x).substr(0,4) + "," + std::to_string(this->y).substr(0,4) + ")";
            return s;
        }

        //! Output a string containing just "RG(ri, gi)"
        std::string outputRG() const
        {
            std::string s("RG(");
            s += std::to_string(this->ri).substr(0,4) + ",";
            s += std::to_string(this->gi).substr(0,4) + ")";
            return s;
        }

        /*!
         * Convert the neighbour position number into a short string representing the
         * direction/position of the neighbour.
         */
        static std::string neighbour_pos (unsigned short dir)
        {
            std::string s("");
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
        void computeLocation()
        {
            // Compute Cartesian location
            this->x = this->d*this->ri + (d/2.0f)*this->gi - (d/2.0f)*this->bi;
            float v = this->getV();
            this->y = v*this->gi + v*this->bi;
            // And location in the Polar coordinate system
            this->r = std::sqrt (x*x + y*y);
            this->phi = atan2 (y, x);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y) by @a
         * cartesianPoint to the centre of this Hex.
         */
        template <typename LFlt>
        float distanceFrom (const morph::vec<LFlt, 2> cartesianPoint) const
        {
            float dx = cartesianPoint[0] - x;
            float dy = cartesianPoint[1] - y;
            return std::sqrt (dx*dx + dy*dy);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y) by the
         * BezCoord @a cartesianPoint to the centre of this Hex.
         */
        float distanceFrom (const BezCoord<float>& cartesianPoint) const
        {
            float dx = cartesianPoint.x() - x;
            float dy = cartesianPoint.y() - y;
            return std::sqrt (dx*dx + dy*dy);
        }

        //! Compute the distance from another hex to this one.
        float distanceFrom (const Hex& otherHex) const
        {
            float dx = otherHex.x - x;
            float dy = otherHex.y - y;
            return std::sqrt (dx*dx + dy*dy);
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
         * This is the index into the d_ vectors in HexGrid which can be used to find
         * the variables recorded for this Hex. It's used in morph::HexGrid to populate
         * HexGrid::d_nne, HexGrid::d_nnw, HexGrid::d_nsw and HexGrid::d_nse, etc.
         *
         * This indexes into the d_ vectors in the HexGrid object to which this Hex belongs. The d_
         * vectors are ordered differently from the list<Hex> object in HexGrid::hexen and hence we
         * have this attribute di in addition to the vector index vi, which provides an index into
         * list<Hex> or vector<Hex> objects which either are, or are arranged like, HexGrid::hexen
         */
        unsigned int di = 0;

        //! Cartesian coordinate 'x' of the centre of the Hex. Public, for direct access by client code.
        float x = 0.0f;
        //! Cartesian 'y' coordinate of the centre of the Hex.
        float y = 0.0f;
        // Getter for (x,y) as a morph::vec
        morph::vec<float, 2> x_y() { return morph::vec<float, 2>({this->x, this->y}); }

        //! Polar coordinates of the centre of the Hex. Public, for direct access by client code.
        float r = 0.0f;
        //! Polar coordinate angle
        float phi = 0.0f;

        //! Position z of the Hex is common to both Cartesian and Polar coordinate systems.
        float z = 0.0f;

        //! Get the Cartesian position of this Hex as a fixed size array.
        std::array<float, 3> position() const
        {
            std::array<float,3> rtn = { { this->x, this->y, this->z } };
            return rtn;
        }

        //! The centre-to-centre distance from one Hex to an immediately adjacent Hex.
        float d = 1.0f;

        //! A getter for d, for completeness. d is the centre-to-centre distance between adjacent hexes.
        float getD() const { return this->d; }

        //! Get the shortest distance from the centre to the perimeter. This is the "short radius".
        float getSR() const { return this->d/2; }

        //! The distance from the centre of the Hex to any of the vertices. This is the "long radius".
        //! Also the side-length of an edge of the Hex.
        float getLR() const
        {
            float lr = this->d * morph::mathconst<float>::one_over_root_3;
            return lr;
        }

        //! Compute and return the area of the hex
        float getArea() const { return (this->d * this->d * morph::mathconst<float>::root_3_over_2); }

        //! The vertical distance between hex centres on adjacent rows.
        float getV() const
        {
            float v = this->d * morph::mathconst<float>::root_3_over_2;
            return v;
        }

        //! The vertical distance from the centre of the hex to the "north east" vertex of the hex.
        float getVtoNE() const
        {
            float v = this->d * morph::mathconst<float>::one_over_2_root_3;
            return v;
        }

        //! Return twice the vertical distance between hex centres on adjacent rows.
        float getTwoV() const
        {
            float tv = this->d * morph::mathconst<float>::sqrt_of_3;
            return tv;
        }

        /*
         * Indices in hex directions. These lie in the x-y plane. They index in positive and
         * negative directions, starting from the Hex at (0,0,z)
         */

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
         * Index in b direction - positive "NorthWest". In a direction 30 degrees West of North
         */
        int bi = 0;

        //! Getter for this->flags
        unsigned int getFlags() const { return this->flags; }

        //! Set one or more flags, defined by flg, true
        void setFlag (unsigned int flg) { this->flags |= flg; }
        //! Alias for Hex::setFlag
        void setFlags (unsigned int flgs) { this->flags |= flgs; }

        //! Unset one or more flags, defined by flg, i.e. set false
        void unsetFlag (unsigned int flg) { this->flags &= ~(flg); }
        //! Alias for Hex::unsetFlag
        void unsetFlags (unsigned int flgs) { this->flags &= ~(flgs); }

        //! If flags match flg, then return true
        bool testFlag (unsigned int flg) const { return (this->flags & flg) == flg ? true : false; }
        //! Alias for Hex::testFlag
        bool testFlags (unsigned int flgs) const { return (this->flags & flgs) == flgs ? true : false; }

        /*!
         * Set to true if this Hex has been marked as being on a boundary. It is expected that
         * client code will then re-set the neighbour relations so that onBoundary() would return
         * true.
         */
        bool boundaryHex() const { return this->flags & HEX_IS_BOUNDARY ? true : false; }
        /*!
         * Mark the hex as a boundary hex. Boundary hexes are also, by definition, inside the
         * boundary.
         */
        void setBoundaryHex() { this->flags |= (HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY); }
        void unsetBoundaryHex() { this->flags &= ~(HEX_IS_BOUNDARY | HEX_INSIDE_BOUNDARY); }

        //! Returns true if this Hex is known to be inside the boundary.
        bool insideBoundary() const { return this->flags & HEX_INSIDE_BOUNDARY ? true : false; }
        //! Set the flag that says this Hex is known to be inside the boundary.
        void setInsideBoundary() { this->flags |= HEX_INSIDE_BOUNDARY; }
        //! Unset the flag that says this Hex is inside the boundary.
        void unsetInsideBoundary() { this->flags &= ~HEX_INSIDE_BOUNDARY; }

        //! Returns true if this Hex is known to be inside a rectangular, parallelogram or hexagonal 'domain'.
        bool insideDomain() const { return this->flags & HEX_INSIDE_DOMAIN ? true : false; }
        //! Set flag that says this Hex is known to be inside a rectangular, parallelogram or hexagonal 'domain'.
        void setInsideDomain() { this->flags |= HEX_INSIDE_DOMAIN; }
        //! Unset flag that says this Hex is known to be inside domain.
        void unsetInsideDomain() { this->flags &= ~HEX_INSIDE_DOMAIN; }

        /*!
         * Set the HEX_USER_FLAG_0/1/2/3 from the passed in unsigned int.
         *
         * E.g. hex->setUserFlags (HEX_USER_FLAG_0 | HEX_USER_FLAG_1);
         *
         * This will set HEX_USER_FLAG_0 and HEX_USER_FLAG_1 AND UNSET HEX_USER_FLAG_2 &
         * HEX_USER_FLAG_3.
         */
        void setUserFlags (unsigned int uflgs) { this->flags |= (uflgs & HEX_ALL_USER); }

        //! Set the single user flag 0, 1 2 or 3 as given by the passed-in unsigned int uflg_num.
        void setUserFlag (unsigned int uflg_num)
        {
            unsigned int flg = 0x1UL << (28+uflg_num);
            this->flags |= flg;
        }

        //! Un-setter corresponding to setUserFlag(unsigned int)
        void unsetUserFlag (unsigned int uflg_num)
        {
            unsigned int flg = 0x1UL << (28+uflg_num);
            this->flags &= ~flg;
        }

        //! Set all user flags to the unset state
        void resetUserFlags() { this->flags &= HEX_NON_USER; }

        //! Getter for each user flag
        bool getUserFlag (unsigned int uflg_num) const
        {
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
        bool onBoundary() const
        {
            return ((this->flags & HEX_HAS_NEIGHB_ALL) == HEX_HAS_NEIGHB_ALL) ? false : true;
        }

        //! Set that \a it is the Neighbour to the East
        void set_ne (std::list<Hex>::iterator it)
        {
            this->ne = it;
            this->flags |= HEX_HAS_NE;
        }
        //! Set that \a it is the Neighbour to the North East
        void set_nne (std::list<Hex>::iterator it)
        {
            this->nne = it;
            this->flags |= HEX_HAS_NNE;
        }
        //! Set that \a it is the Neighbour to the North West
        void set_nnw (std::list<Hex>::iterator it)
        {
            this->nnw = it;
            this->flags |= HEX_HAS_NNW;
        }
        //! Set that \a it is the Neighbour to the West
        void set_nw (std::list<Hex>::iterator it)
        {
            this->nw = it;
            this->flags |= HEX_HAS_NW;
        }
        //! Set that \a it is the Neighbour to the South West
        void set_nsw (std::list<Hex>::iterator it)
        {
            this->nsw = it;
            this->flags |= HEX_HAS_NSW;
        }
        //! Set that \a it is the Neighbour to the South East
        void set_nse (std::list<Hex>::iterator it)
        {
            this->nse = it;
            this->flags |= HEX_HAS_NSE;
        }

        //! Return true if this Hex has a Neighbour to the East
        bool has_ne() const { return ((this->flags & HEX_HAS_NE) == HEX_HAS_NE); }
        //! Return true if this Hex has a Neighbour to the North East
        bool has_nne() const { return ((this->flags & HEX_HAS_NNE) == HEX_HAS_NNE); }
        //! Return true if this Hex has a Neighbour to the North West
        bool has_nnw() const { return ((this->flags & HEX_HAS_NNW) == HEX_HAS_NNW); }
        //! Return true if this Hex has a Neighbour to the West
        bool has_nw() const { return ((this->flags & HEX_HAS_NW) == HEX_HAS_NW); }
        //! Return true if this Hex has a Neighbour to the South West
        bool has_nsw() const { return ((this->flags & HEX_HAS_NSW) == HEX_HAS_NSW); }
        //! Return true if this Hex has a Neighbour to the South East
        bool has_nse() const { return ((this->flags & HEX_HAS_NSE) == HEX_HAS_NSE); }

        //! Set flags to say that this Hex has NO neighbour to East
        void unset_ne() { this->flags ^= HEX_HAS_NE; }
        //! Set flags to say that this Hex has NO neighbour to North East
        void unset_nne() { this->flags ^= HEX_HAS_NNE; }
        //! Set flags to say that this Hex has NO neighbour to North West
        void unset_nnw() { this->flags ^= HEX_HAS_NNW; }
        //! Set flags to say that this Hex has NO neighbour to West
        void unset_nw() { this->flags ^= HEX_HAS_NW; }
        //! Set flags to say that this Hex has NO neighbour to South West
        void unset_nsw() { this->flags ^= HEX_HAS_NSW; }
        //! Set flags to say that this Hex has NO neighbour to South East
        void unset_nse() { this->flags ^= HEX_HAS_NSE; }

        /*!
         * Test if have neighbour at position \a ni.
         * East: 0, North-East: 1, North-West: 2, West: 3, South-West: 4, South-East: 5
         */
        bool has_neighbour (unsigned short ni) const
        {
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

        /*!
         * Get a list<Hex>::iterator to the neighbour at position \a ni.
         * East: 0, North-East: 1, North-West: 2, West: 3, South-West: 4, South-East: 5
         */
        std::list<Hex>::iterator get_neighbour (unsigned short ni) const
        {
            std::list<Hex>::iterator hi;
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

        //! Turn the vertex index \a ni into a string name and return it.
        static std::string vertex_name (unsigned short ni)
        {
            std::string s("");
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
        morph::vec<float, 2> get_vertex_coord (unsigned short ni) const
        {
            morph::vec<float, 2> rtn = { 0.0f, 0.0f };
            switch (ni) {
            case HEX_VERTEX_POS_NE:
            {
                rtn[0] = this->x + this->getSR();
                rtn[1] = this->y + this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_N:
            {
                rtn[0] = this->x;
                rtn[1] = this->y + this->getLR();
                break;
            }
            case HEX_VERTEX_POS_NW:
            {
                rtn[0] = this->x - this->getSR();
                rtn[1] = this->y + this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_SW:
            {
                rtn[0] = this->x - this->getSR();
                rtn[1] = this->y - this->getVtoNE();
                break;
            }
            case HEX_VERTEX_POS_S:
            {
                rtn[0] = this->x;
                rtn[1] = this->y - this->getLR();
                break;
            }
            case HEX_VERTEX_POS_SE:
            {
                rtn[0] = this->x + this->getSR();
                rtn[1] = this->y - this->getVtoNE();
                break;
            }
            default:
            {
                rtn[0] = -1.0f;
                rtn[1] = -1.0f;
                break;
            }
            }
            return rtn;
        }

        /*!
         * Get the Cartesian coordinates of the given vertex of the Hex. This sub-calls
         * the overload of get_vertex_coord which accepts a single, unsigned short
         * argument.
         */
        morph::vec<float, 2> get_vertex_coord (unsigned int ni) const
        {
            morph::vec<float, 2> rtn = { -2.0f, -2.0f };
            if (ni > 5) { return rtn; }
            rtn = this->get_vertex_coord (static_cast<unsigned short> (ni));
            return rtn;
        }

        /*!
         * Get the Cartesian coordinates of the given vertex of the Hex. This sub-calls
         * the overload of get_vertex_coord which accepts a single, unsigned short
         * argument.
         */
        morph::vec<float, 2> get_vertex_coord (int ni) const
        {
            morph::vec<float, 2> rtn = { -3.0f, -3.0f };
            if (ni > 5) {
                rtn[0] = -4.0f;
                return rtn;
            }
            if (ni < 0) {
                rtn[1] = -4.0f;
                return rtn;
            }
            rtn = this->get_vertex_coord (static_cast<unsigned short> (ni));
            return rtn;
        }

        /*!
         * Return true if coord is reasonably close to being in the same location as the vertex at
         * vertex \a ni with the distance threshold being set from the Hex to Hex spacing. This is for
         * distinguishing between vertices and hex centres on a HexGrid.
         */
        template <typename LFlt>
        bool compare_vertex_coord (int ni, morph::vec<LFlt, 2>& coord) const
        {
            morph::vec<float, 2> vc = this->get_vertex_coord (ni);
            if (std::abs(vc[0] - coord[0]) < this->d/100
                && std::abs(vc[1] - coord[1]) < this->d/100) {
                return true;
            }
            return false;
        }

        //! Return true if the Hex contains the vertex at \a coord
        template <typename LFlt>
        bool contains_vertex (morph::vec<LFlt, 2>& coord) const
        {
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
        bool compare_coord (morph::vec<LFlt, 2>& coord) const
        {
            if (std::abs(this->x - coord[0]) < this->d/100
                && std::abs(this->y - coord[1]) < this->d/100) {
                return true;
            }
            return false;
        }

        //! Un-set the pointers on all my neighbours so that THEY no longer point to ME.
        void disconnectNeighbours()
        {
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

        /*
         * Nearest neighbours
         */

        //! Nearest neighbour to the East; in the plus r direction.
        std::list<Hex>::iterator ne;
        //! Nearest neighbour to the NorthEast; in the plus g direction.
        std::list<Hex>::iterator nne;
        //! Nearest neighbour to the NorthWest; in the plus b direction.
        std::list<Hex>::iterator nnw;
        //! Nearest neighbour to the West; in the minus r direction.
        std::list<Hex>::iterator nw;
        //! Nearest neighbour to the SouthWest; in the minus g direction.
        std::list<Hex>::iterator nsw;
        //! Nearest neighbour to the SouthEast; in the minus b direction.
        std::list<Hex>::iterator nse;

    private:
        //! The flags for this Hex.
        unsigned int flags = 0x0;
    };

} // namespace morph
