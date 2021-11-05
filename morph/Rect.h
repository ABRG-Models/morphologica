/*
 * \file
 *
 * Defines a class to manage a rectangle which lives in a Cartesian grid (CartGrid.h)
 *
 * \author: Seb James
 * \date: 2021/02
 */
#pragma once

#include <string>
#include <list>
#include <utility>
#include <cmath>
#include <morph/Vector.h>
#include <morph/BezCoord.h>
#include <morph/HdfData.h>

/*
 * Flags
 */

/*!
 * Set true when ne has been set. Use of iterators (Rect::ne etc) rather than pointers
 * for neighbouring rects means we can't do any kind of check to see if the iterator is
 * valid, so we have to keep separate boolean flags for whether or not each Rect has a
 * neighbour. Those flags are kept in Rect::flags.
 */
#define RECT_HAS_NE                0x1
//! True when this rect has a Neighbour to the North East
#define RECT_HAS_NNE               0x2
//! True when this rect has a Neighbour to the North West
#define RECT_HAS_NNW               0x4
//! True when this rect has a Neighbour to the North
#define RECT_HAS_NN                0x8
//! True when this rect has a Neighbour to the West
#define RECT_HAS_NW               0x10
//! True when this rect has a Neighbour to the South West
#define RECT_HAS_NSW              0x20
//! True when this rect has a Neighbour to the South
#define RECT_HAS_NS               0x40
//! True when this rect has a Neighbour to the South East
#define RECT_HAS_NSE              0x80
//! A short cut for testing all the neighbour flags at once
#define RECT_HAS_NEIGHB_ALL       0xff // RECT_HAS_NE | RECT_HAS_NNE | ...etc

//! All rects marked as boundary rects, including some that are additional to requirements:
#define RECT_IS_BOUNDARY         0x100
//! All rects inside boundary plus as much of the boundary as needed to make a contiguous boundary:
#define RECT_INSIDE_BOUNDARY     0x200
//! All rects inside the domain of computation:
#define RECT_INSIDE_DOMAIN       0x400
//! Rect is a 'region boundary rect'. Regions are intended to be temporary to aid client code.
#define RECT_IS_REGION_BOUNDARY  0x800
//! Rect is inside the region
#define RECT_INSIDE_REGION      0x1000

//! Four flags for client code to use for its own devices. For an example of use, see DirichDom.h
#define RECT_USER_FLAG_0    0x10000000
#define RECT_USER_FLAG_1    0x20000000
#define RECT_USER_FLAG_2    0x40000000
#define RECT_USER_FLAG_3    0x80000000
//! Four bits high: all user flags set
#define RECT_ALL_USER       0xf0000000
//! Bitmask for all the flags that aren't the 4 user flags.
#define RECT_NON_USER       0x0fffffff

//! Neighbour (or edge, or side) positions
#define RECT_NEIGHBOUR_POS_E       0x0
#define RECT_NEIGHBOUR_POS_NE      0x1
#define RECT_NEIGHBOUR_POS_N       0x2
#define RECT_NEIGHBOUR_POS_NW      0x3
#define RECT_NEIGHBOUR_POS_W       0x4
#define RECT_NEIGHBOUR_POS_SW      0x5
#define RECT_NEIGHBOUR_POS_S       0x6
#define RECT_NEIGHBOUR_POS_SE      0x7

//! Vertex positions
#define RECT_VERTEX_POS_NE     0x0
#define RECT_VERTEX_POS_NW     0x1
#define RECT_VERTEX_POS_SW     0x2
#define RECT_VERTEX_POS_SE     0x3

namespace morph {

    /*!
     * Describes a regular rectangular 'pixel'.
     *
     * The centre of the rect in a Cartesian right hand coordinate system is represented
     * with x, y and z:
     *
     *  y
     *  ^
     *  |
     *  |
     *  0-----> x     z out of screen/page
     *
     * I've defined numbering for the Rect's vertices and for its edges.
     *
     * Vertices: NE: 0, NW: 1, SW: 2, SE: 3
     *
     * Edges/Sides: East: 0, North: 1, West: 2, South: 3
     */
    class Rect
    {
    public:
        /*!
         * Constructor taking index, dimension for a square pixel and integer position
         * indices. Computes Cartesian location from these.
         */
        Rect (const unsigned int& idx, const float& d_, const int& xi_, const int& yi_)
        {
            this->vi = idx;
            this->dx = d_;
            this->dy = d_;
            this->xi = xi_;
            this->yi = yi_;
            this->computeLocation();
        }

        //! Constructor for a rectangular pixel
        Rect (const unsigned int& idx, const float& dx_, const float& dy_, const int& xi_, const int& yi_)
        {
            this->vi = idx;
            this->dx = dx_;
            this->dy = dy_;
            this->xi = xi_;
            this->yi = yi_;
            this->computeLocation();
        }

        //! Construct using the passed in HDF5 file and path.
        Rect (HdfData& h5data, const std::string& h5path) { this->load (h5data, h5path); }

        //! Comparison operation to enable use of set<Rect>
        bool operator< (const Rect& rhs) const
        {
            // Compare position first.
            if (this->x < rhs.x) { return true; }
            if (this->x > rhs.x) { return false; }
            if (this->y < rhs.y) { return true; }
            if (this->y > rhs.y) { return false; }
            // If position can't differentiate, compare vector index
            if (this->vi < rhs.vi) { return true; }
            return false;
        }

        /*!
         * Save the data for this Rect into the already open HdfData object \a h5data in
         * the path \a h5path.
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
            dpath = h5path + "/dx";
            h5data.add_val (dpath.c_str(), this->dx);
            dpath = h5path + "/dy";
            h5data.add_val (dpath.c_str(), this->dy);
            dpath = h5path + "/xi";
            h5data.add_val (dpath.c_str(), this->xi);
            dpath = h5path + "/yi";
            h5data.add_val (dpath.c_str(), this->yi);
            dpath = h5path + "/distToBoundary";
            h5data.add_val (dpath.c_str(), this->distToBoundary);
            dpath = h5path + "/flags";
            h5data.add_val (dpath.c_str(), this->flags);
        }

        //! Load the data for this Rect from a morph::HdfData file
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
            dpath = h5path + "/dx";
            h5data.read_val (dpath.c_str(), this->dx);
            dpath = h5path + "/dy";
            h5data.read_val (dpath.c_str(), this->dy);
            dpath = h5path + "/xi";
            h5data.read_val (dpath.c_str(), this->xi);
            dpath = h5path + "/yi";
            h5data.read_val (dpath.c_str(), this->yi);
            dpath = h5path + "/distToBoundary";
            h5data.read_val (dpath.c_str(), this->distToBoundary);
            unsigned int flgs = 0;
            dpath = h5path + "/flags";
            h5data.read_val (dpath.c_str(), flgs);
            this->flags = flgs;
        }

        /*!
         * Produce a string containing information about this rect, showing grid
         * location in dimensionless xi,yi units. Also show nearest neighbours.
         */
        std::string output() const
        {
            std::string s("Rect ");
            s += std::to_string(this->vi) + " (";
            s += std::to_string(this->xi).substr(0,4) + ",";
            s += std::to_string(this->yi).substr(0,4) + "). ";

            if (this->has_ne()) {
                s += "E: (" + std::to_string(this->ne->xi).substr(0,4) + "," + std::to_string(this->ne->yi).substr(0,4) + ") " + (this->ne->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nse()) {
                s += "SE: (" + std::to_string(this->nse->xi).substr(0,4) + "," + std::to_string(this->nse->yi).substr(0,4) + ") " + (this->nse->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_ns()) {
                s += "S: (" + std::to_string(this->ns->xi).substr(0,4) + "," + std::to_string(this->ns->yi).substr(0,4) + ") " + (this->ns->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nsw()) {
                s += "SW: (" + std::to_string(this->nsw->xi).substr(0,4) + "," + std::to_string(this->nsw->yi).substr(0,4) + ") " + (this->nsw->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nw()) {
                s += "W: (" + std::to_string(this->nw->xi).substr(0,4) + "," + std::to_string(this->nw->yi).substr(0,4) + ") " + (this->nw->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nnw()) {
                s += "NW: (" + std::to_string(this->nnw->xi).substr(0,4) + "," + std::to_string(this->nnw->yi).substr(0,4) + ") " + (this->nnw->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nn()) {
                s += "N: (" + std::to_string(this->nn->xi).substr(0,4) + "," + std::to_string(this->nn->yi).substr(0,4) + ") " + (this->nn->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->has_nne()) {
                s += "NE: (" + std::to_string(this->nne->xi).substr(0,4) + "," + std::to_string(this->nne->yi).substr(0,4) + ") " + (this->nne->boundaryRect() == true ? "OB":"") + " ";
            }
            if (this->boundaryRect()) {
                s += "(ON boundary)";
            } else  {
                s += "(not boundary)";
            }
            return s;
        }

        /*!
         * Produce a string containing information about this rect, focussing on
         * Cartesian position information.
         */
        std::string outputCart() const
        {
            std::string s("Rect ");
            s += std::to_string(this->vi) + " (";
            s += std::to_string(this->xi).substr(0,4) + ",";
            s += std::to_string(this->yi).substr(0,4) + ") is at (x,y) = ("
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

        //! Output a string containing just "XiYi(xi, yi)"
        std::string outputXiYi() const
        {
            std::string s("XiYi(");
            s += std::to_string(this->xi).substr(0,4) + ",";
            s += std::to_string(this->yi).substr(0,4) + ")";
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
            case RECT_NEIGHBOUR_POS_E:
            {
                s = "E";
                break;
            }
            case RECT_NEIGHBOUR_POS_NE:
            {
                s = "NE";
                break;
            }
            case RECT_NEIGHBOUR_POS_N:
            {
                s = "N";
                break;
            }
            case RECT_NEIGHBOUR_POS_NW:
            {
                s = "NW";
                break;
            }
            case RECT_NEIGHBOUR_POS_W:
            {
                s = "W";
                break;
            }
            case RECT_NEIGHBOUR_POS_SW:
            {
                s = "SW";
                break;
            }
            case RECT_NEIGHBOUR_POS_S:
            {
                s = "S";
                break;
            }
            case RECT_NEIGHBOUR_POS_SE:
            {
                s = "SE";
                break;
            }
            };
            return s;
        }

        /*!
         * Convert xi and yi indices into x and y coordinates and also r and phi
         * coordinates, based on the rect-to-rect distances dx and dy.
         */
        void computeLocation()
        {
            // Compute Cartesian location
            this->x = this->dx*this->xi;
            this->y = this->dy*this->yi;
            // And location in the Polar coordinate system
            this->r = std::sqrt (x*x + y*y);
            this->phi = atan2 (y, x);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y)
         * by \a cartesianPoint to the centre of this Rect.
         */
        template <typename LFlt>
        float distanceFrom (const std::pair<LFlt, LFlt> cartesianPoint) const
        {
            float deltax = cartesianPoint.first - x;
            float deltay = cartesianPoint.second - y;
            return std::sqrt (deltax*deltax + deltay*deltay);
        }

        /*!
         * Compute the distance from the point given (in two-dimensions only; x and y)
         * by the BezCoord \a cartesianPoint to the centre of this Rect.
         */
        float distanceFrom (const BezCoord<float>& cartesianPoint) const
        {
            float deltax = cartesianPoint.x() - x;
            float deltay = cartesianPoint.y() - y;
            return std::sqrt (deltax*deltax + deltay*deltay);
        }

        //! Compute the distance from another rect to this one.
        float distanceFrom (const Rect& otherRect) const
        {
            float deltax = otherRect.x - x;
            float deltay = otherRect.y - y;
            return std::sqrt (deltax*deltax + deltay*deltay);
        }

        /*!
         * Vector index. This is the index into those data vectors which hold the
         * relevant data pertaining to this rect. This is a scheme which allows me to
         * keep the data in separate vectors and all the rect position information in
         * this class.  What happens when I delete some rect elements?  Simple - I can
         * re-set the vi indices after creating a grid of Rect elements and then pruning
         * down.
         */
        unsigned int vi;

        /*!
         * This is the index into the d_ vectors in CartGrid which can be used to find
         * the variables recorded for this Rect. It's used in morph::CartGrid to
         * populate CartGrid::d_nne, CartGrid::d_nnw, etc.
         *
         * This indexes into the d_ vectors in the CartGrid object to which this Rect
         * belongs. The d_ vectors are ordered differently from the list<Rect> object in
         * CartGrid::rects and hence we have this attribute di in addition to the vector
         * index vi, which provides an index into list<Rect> or vector<Rect> objects
         * which either are, or are arranged like, RectGrid::rects
         */
        unsigned int di = 0;

        //! Cartesian coordinate 'x' of the centre of the Rect. Public, for direct access by client code.
        float x = 0.0f;
        //! Cartesian 'y' coordinate of the centre of the Rect.
        float y = 0.0f;

        //! Polar coordinates of the centre of the Rect.
        float r = 0.0f;
        //! Polar coordinate angle
        float phi = 0.0f;

        //! Position z of the Rect is common to both Cartesian and Polar coordinate systems.
        float z = 0.0f;

        //! Get the Cartesian position of this Rect as a fixed size array.
        morph::Vector<float, 3> position() const
        {
            morph::Vector<float,3> rtn = { { this->x, this->y, this->z } };
            return rtn;
        }

        //! The distance from one Rect to an immediately adjacent Rect to W or E.
        float dx = 1.0f;
        //! The distance from one Rect to an immediately adjacent Rect to N or S.
        float dy = 1.0f;

        // Getters for dx, dy
        float get_dx() const { return this->dx; }
        float get_dy() const { return this->dy; }

        //! Get the shortest distance from the centre of the Rect to its perimeter. This is the "short radius".
        float getSR() const { return ((this->dx < this->dy ? this->dx : this->dy) * 0.5f); }

        //! The distance from the centre of the Rect to any of the vertices. This is the "long radius".
        float getLR() const { return (std::sqrt (this->dx * this->dx + this->dy * this->dy) * 0.5f); }

        //! Compute and return the area of the Rect
        float getArea() const { return (this->dx * this->dy); }

        //! The vertical distance between Rect centres on adjacent rows.
        float getV() const { return this->dy; }

        //! The vertical distance from the centre of the Rect to the "north east" vertex of the Rect
        float getVtoNE() const { return std::sqrt (this->dx * this->dx + this->dy * this->dy); }

        /*!
         * Return twice the vertical distance between Rect centres on adjacent
         * rows. (unlikely to be useful, but included to match API of morph::Hex)
         */
        float getTwoV() const { return 2.0f * this->dy; }

        /*
         * Indices in x/y directions. These lie in the x-y plane. They index in positive
         * and negative directions, starting from the Rect at (0,0,z)
         */

        //! Index in +x direction - positive East
        int xi = 0;
        //! Index in +y direction - positive North
        int yi = 0;

        //! Getter for this->flags
        unsigned int getFlags() const { return this->flags; }

        //! Set one or more flags, defined by flg, true
        void setFlag (unsigned int flg) { this->flags |= flg; }
        //! Alias for Rect::setFlag
        void setFlags (unsigned int flgs) { this->flags |= flgs; }

        //! Unset one or more flags, defined by flg, i.e. set false
        void unsetFlag (unsigned int flg) { this->flags &= ~(flg); }
        //! Alias for Rect::unsetFlag
        void unsetFlags (unsigned int flgs) { this->flags &= ~(flgs); }

        //! If flags match flg, then return true
        bool testFlag (unsigned int flg) const { return (this->flags & flg) == flg ? true : false; }
        //! Alias for Rect::testFlag
        bool testFlags (unsigned int flgs) const { return (this->flags & flgs) == flgs ? true : false; }

        /*!
         * Set to true if this Rect has been marked as being on a boundary. It is
         * expected that client code will then re-set the neighbour relations so that
         * onBoundary() would return true.
         */
        bool boundaryRect() const { return this->flags & RECT_IS_BOUNDARY ? true : false; }
        /*!
         * Mark the Rect as a boundary Rect. Boundary rects are also, by definition,
         * inside the boundary.
         */
        void setBoundaryRect() { this->flags |= (RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY); }
        void unsetBoundaryREct() { this->flags &= ~(RECT_IS_BOUNDARY | RECT_INSIDE_BOUNDARY); }

        //! Returns true if this Rect is known to be inside the boundary.
        bool insideBoundary() const { return this->flags & RECT_INSIDE_BOUNDARY ? true : false; }
        //! Set the flag that says this Rect is known to be inside the boundary.
        void setInsideBoundary() { this->flags |= RECT_INSIDE_BOUNDARY; }
        //! Unset the flag that says this Rect is inside the boundary.
        void unsetInsideBoundary() { this->flags &= ~RECT_INSIDE_BOUNDARY; }

        //! Returns true if this Rect is known to be inside a 'domain'.
        bool insideDomain() const { return this->flags & RECT_INSIDE_DOMAIN ? true : false; }
        //! Set flag that says this Rect is known to be inside a 'domain'.
        void setInsideDomain() { this->flags |= RECT_INSIDE_DOMAIN; }
        //! Unset flag that says this Rect is known to be inside domain.
        void unsetInsideDomain() { this->flags &= ~RECT_INSIDE_DOMAIN; }

        /*!
         * Set the RECT_USER_FLAG_0/1/2/3 from the passed in unsigned int.
         *
         * E.g. rect->setUserFlags (RECT_USER_FLAG_0 | RECT_USER_FLAG_1);
         *
         * This will set RECT_USER_FLAG_0 and RECT_USER_FLAG_1 AND UNSET RECT_USER_FLAG_2 &
         * RECT_USER_FLAG_3.
         */
        void setUserFlags (unsigned int uflgs) { this->flags |= (uflgs & RECT_ALL_USER); }

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
        void resetUserFlags() { this->flags &= RECT_NON_USER; }

        //! Getter for each user flag
        bool getUserFlag (unsigned int uflg_num) const
        {
            unsigned int flg = 0x1UL << (28+uflg_num);
            return ((this->flags & flg) == flg);
        }

        /*!
         * This can be populated with the distance to the nearest boundary rect, so that
         * an algorithm can set values in a rect based this metric.
         */
        float distToBoundary = -1.0f;

        /*!
         * Return true if this is a boundary rect - one on the outside edge of a rect
         * grid. The result is based on testing neihgbour relations, rather than
         * examining the value of the RECT_IS_BOUNDARY flag.
         */
        bool onBoundary()
        {
            return ((this->flags & RECT_HAS_NEIGHB_ALL) == RECT_HAS_NEIGHB_ALL) ? false : true;
        }

        //! Set that \a it is the Neighbour to the East
        void set_ne (std::list<Rect>::iterator it)
        {
            this->ne = it;
            this->flags |= RECT_HAS_NE;
        }
        //! Set that \a it is the Neighbour to the North East
        void set_nne (std::list<Rect>::iterator it)
        {
            this->nne = it;
            this->flags |= RECT_HAS_NNE;
        }
        //! Set that \a it is the Neighbour to the North
        void set_nn (std::list<Rect>::iterator it)
        {
            this->nn = it;
            this->flags |= RECT_HAS_NN;
        }
        //! Set that \a it is the Neighbour to the North West
        void set_nnw (std::list<Rect>::iterator it)
        {
            this->nnw = it;
            this->flags |= RECT_HAS_NNW;
        }
        //! Set that \a it is the Neighbour to the West
        void set_nw (std::list<Rect>::iterator it)
        {
            this->nw = it;
            this->flags |= RECT_HAS_NW;
        }
        //! Set that \a it is the Neighbour to the South West
        void set_nsw (std::list<Rect>::iterator it)
        {
            this->nsw = it;
            this->flags |= RECT_HAS_NSW;
        }
        //! Set that \a it is the Neighbour to the South
        void set_ns (std::list<Rect>::iterator it)
        {
            this->ns = it;
            this->flags |= RECT_HAS_NS;
        }
        //! Set that \a it is the Neighbour to the South East
        void set_nse (std::list<Rect>::iterator it)
        {
            this->nse = it;
            this->flags |= RECT_HAS_NSE;
        }

        //! Return true if this Rect has a Neighbour to the East
        bool has_ne() const { return ((this->flags & RECT_HAS_NE) == RECT_HAS_NE); }
        //! Return true if this Rect has a Neighbour to the North East
        bool has_nne() const { return ((this->flags & RECT_HAS_NNE) == RECT_HAS_NNE); }
        //! Return true if this Rect has a Neighbour to the North
        bool has_nn() const { return ((this->flags & RECT_HAS_NN) == RECT_HAS_NN); }
        //! Return true if this Rect has a Neighbour to the North West
        bool has_nnw() const { return ((this->flags & RECT_HAS_NNW) == RECT_HAS_NNW); }
        //! Return true if this Rect has a Neighbour to the West
        bool has_nw() const { return ((this->flags & RECT_HAS_NW) == RECT_HAS_NW); }
        //! Return true if this Rect has a Neighbour to the South West
        bool has_nsw() const { return ((this->flags & RECT_HAS_NSW) == RECT_HAS_NSW); }
        //! Return true if this Rect has a Neighbour to the South
        bool has_ns() const { return ((this->flags & RECT_HAS_NS) == RECT_HAS_NS); }
        //! Return true if this Rect has a Neighbour to the South East
        bool has_nse() const { return ((this->flags & RECT_HAS_NSE) == RECT_HAS_NSE); }

        //! Set flags to say that this Rect has NO neighbour to East
        void unset_ne() { this->flags ^= RECT_HAS_NE; }
        //! Set flags to say that this Rect has NO neighbour to North East
        void unset_nne() { this->flags ^= RECT_HAS_NNE; }
        //! Set flags to say that this Rect has NO neighbour to North
        void unset_nn() { this->flags ^= RECT_HAS_NN; }
        //! Set flags to say that this Rect has NO neighbour to North West
        void unset_nnw() { this->flags ^= RECT_HAS_NNW; }
        //! Set flags to say that this Rect has NO neighbour to West
        void unset_nw() { this->flags ^= RECT_HAS_NW; }
        //! Set flags to say that this Rect has NO neighbour to South West
        void unset_nsw() { this->flags ^= RECT_HAS_NSW; }
        //! Set flags to say that this Rect has NO neighbour to South
        void unset_ns() { this->flags ^= RECT_HAS_NS; }
        //! Set flags to say that this Rect has NO neighbour to South East
        void unset_nse() { this->flags ^= RECT_HAS_NSE; }

        /*!
         * Test if have neighbour at position \a ni.  East: 0, North-East: 1, North: 2,
         * North-West: 3, West: 4, South-West: 5, South: 6, South-East: 7
         */
        bool has_neighbour (unsigned short ni) const
        {
            switch (ni) {
            case RECT_NEIGHBOUR_POS_E:
            {
                return (this->flags & RECT_HAS_NE) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_NE:
            {
                return (this->flags & RECT_HAS_NNE) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_N:
            {
                return (this->flags & RECT_HAS_NN) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_NW:
            {
                return (this->flags & RECT_HAS_NNW) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_W:
            {
                return (this->flags & RECT_HAS_NW) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_SW:
            {
                return (this->flags & RECT_HAS_NSW) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_S:
            {
                return (this->flags & RECT_HAS_NS) ? true : false;
                break;
            }
            case RECT_NEIGHBOUR_POS_SE:
            {
                return (this->flags & RECT_HAS_NSE) ? true : false;
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
         * Get a list<Rect>::iterator to the neighbour at position \a ni.  East: 0,
         * North-East: 1, North: 2, North-West: 3, West: 4, South-West: 5, South: 6,
         * South-East: 7
         */
        std::list<Rect>::iterator get_neighbour (unsigned short ni) const
        {
            std::list<Rect>::iterator hi;
            switch (ni) {
            case RECT_NEIGHBOUR_POS_E:
            {
                hi = this->ne;
                break;
            }
            case RECT_NEIGHBOUR_POS_NE:
            {
                hi = this->nne;
                break;
            }
            case RECT_NEIGHBOUR_POS_N:
            {
                hi = this->nn;
                break;
            }
            case RECT_NEIGHBOUR_POS_NW:
            {
                hi = this->nnw;
                break;
            }
            case RECT_NEIGHBOUR_POS_W:
            {
                hi = this->nw;
                break;
            }
            case RECT_NEIGHBOUR_POS_SW:
            {
                hi = this->nsw;
                break;
            }
            case RECT_NEIGHBOUR_POS_S:
            {
                hi = this->ns;
                break;
            }
            case RECT_NEIGHBOUR_POS_SE:
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
            case RECT_VERTEX_POS_NE:
            {
                s = "NE";
                break;
            }
            case RECT_VERTEX_POS_NW:
            {
                s = "NW";
                break;
            }
            case RECT_VERTEX_POS_SW:
            {
                s = "SW";
                break;
            }
            case RECT_VERTEX_POS_SE:
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
         * Get the Cartesian coordinates of the given vertex of the Rect. The single
         * argument \a ni specifies which vertex to return the coordinate for. Use the
         * definitions RECT_VERTEX_POS_N, etc to pass in a human-readable label for the
         * vertex.
         */
        std::pair<float, float> get_vertex_coord (unsigned short ni) const
        {
            std::pair<float, float> rtn = {0.0f, 0.0f};
            switch (ni) {
            case RECT_VERTEX_POS_NE:
            {
                rtn.first = this->x + this->getSR();
                rtn.second = this->y + this->getVtoNE();
                break;
            }
            case RECT_VERTEX_POS_NW:
            {
                rtn.first = this->x - this->getSR();
                rtn.second = this->y + this->getVtoNE();
                break;
            }
            case RECT_VERTEX_POS_SW:
            {
                rtn.first = this->x - this->getSR();
                rtn.second = this->y - this->getVtoNE();
                break;
            }
            case RECT_VERTEX_POS_SE:
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

        /*!
         * Get the Cartesian coordinates of the given vertex of the Rect. This sub-calls
         * the overload of get_vertex_coord which accepts a single, unsigned short
         * argument.
         */
        std::pair<float, float> get_vertex_coord (unsigned int ni) const
        {
            std::pair<float, float> rtn = {-2.0f, -2.0f};
            if (ni > 3) {
                return rtn;
            }
            rtn = this->get_vertex_coord (static_cast<unsigned short> (ni));
            return rtn;
        }

        /*!
         * Get the Cartesian coordinates of the given vertex of the Rect. This sub-calls
         * the overload of get_vertex_coord which accepts a single, unsigned short
         * argument.
         */
        std::pair<float, float> get_vertex_coord (int ni) const
        {
            std::pair<float, float> rtn = {-3.0f, -3.0f};
            if (ni > 3) {
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
         * Return true if coord is reasonably close to being in the same location as the
         * vertex at vertex \a ni with the distance threshold being set from the Rect to
         * Rect spacing. This is for distinguishing between vertices and centres on a
         * RectGrid.
         */
        template <typename LFlt>
        bool compare_vertex_coord (int ni, std::pair<LFlt, LFlt>& coord) const
        {
            std::pair<float, float> vc = this->get_vertex_coord (ni);
            float sr_thresh = this->getSR()/100.0f;
            if (std::abs(vc.first - coord.first) < sr_thresh
                && std::abs(vc.second - coord.second) < sr_thresh) {
                return true;
            }
            return false;
        }

        //! Return true if the Rect contains the vertex at \a coord
        template <typename LFlt>
        bool contains_vertex (std::pair<LFlt, LFlt>& coord) const
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
         * Return true if coord is reasonably close to being in the same location as the
         * centre of the Rect, with the distance threshold being set from the Rect to
         * Rect spacing. This is for distinguishing between vertices and centres on a
         * RectGrid.
         */
        template <typename LFlt>
        bool compare_coord (std::pair<LFlt, LFlt>& coord) const
        {
            float sr_thresh = this->getSR()/100.0f;
            if (std::abs(this->x - coord.first) < sr_thresh
                && std::abs(this->y - coord.second) < sr_thresh) {
                return true;
            }
            return false;
        }

        //! Un-set the pointers on all my neighbours so that THEY no longer point to ME.
        void disconnectNeighbours()
        {
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
            if (this->has_nn()) {
                if (this->nn->has_ns()) {
                    this->nn->unset_ns();
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
            if (this->has_ns()) {
                if (this->ns->has_nn()) {
                    this->ns->unset_nn();
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

        //! Nearest neighbour to the East; in the plus x direction
        std::list<Rect>::iterator ne;
        //! Nearest neighbour to the NorthEast
        std::list<Rect>::iterator nne;
        //! Nearest neighbour to the North; in the plus y direction.
        std::list<Rect>::iterator nn;
        //! Nearest neighbour to the NorthWest
        std::list<Rect>::iterator nnw;
        //! Nearest neighbour to the West
        std::list<Rect>::iterator nw;
        //! Nearest neighbour to the SouthWest
        std::list<Rect>::iterator nsw;
        //! Nearest neighbour to the South
        std::list<Rect>::iterator ns;
        //! Nearest neighbour to the SouthEast
        std::list<Rect>::iterator nse;

    private:
        //! The flags for this Rect.
        unsigned int flags = 0x0;
    };

} // namespace morph
