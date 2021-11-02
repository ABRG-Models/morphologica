/*!
 * A Bezier curve coordinate class.
 */

#pragma once

#include <utility>
#include <cmath>
#include <ostream>
#include <cmath>

namespace morph
{
    /*!
     * A class defining a bezier curve coordinate, along with its parameter value and
     * the distance remaining to the end of the curve.
     */
    template <typename Flt>
    class BezCoord
    {
    public: // methods

        //! Construct empty BezCoord. Defaults to non-null.
        BezCoord ()
        {
            this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = false;
            this->coord.first = 0.0f; this->coord.second = 0.0f;
        }

        //! Construct empty coordinate, which may or may not be set to null.
        BezCoord (bool nullcoord)
        {
            this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = nullcoord;
            this->coord.first = 0.0f; this->coord.second = 0.0f;
        }

        //! Construct using just a 2D coordinate
        BezCoord (std::pair<Flt,Flt> r)
            : coord(r)
            { this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = false; }

        //! Construct with coordinate and corresponding t parameter
        BezCoord (Flt t, std::pair<Flt,Flt> r)
            : coord(r)
            , param(t)
            { this->remaining = -1.0f; this->nullCoordinate = false; }

        //! Construct with coord & t and also set the para remaining
        BezCoord (Flt t, std::pair<Flt,Flt> r, Flt remain)
            : coord(r)
            , param(t)
            , remaining(remain)
            { this->nullCoordinate = false; }

        // Accessors
        std::pair<Flt,Flt> getCoord (void) const { return this->coord; }
        Flt getParam (void) const { return this->param; }
        Flt getRemaining (void) const { return this->remaining; }
        bool getNullCoordinate (void) const { return this->nullCoordinate; }
        bool isNull (void) const { return this->nullCoordinate; }
        void setCoord (std::pair<Flt,Flt> c) { this->coord = c; }
        void setParam (Flt p) { this->param = p; }
        void setRemaining (Flt r) { this->remaining = r; }
        void setNullCoordinate (bool b) { this->nullCoordinate = b; }

        // Single character accessors, for easy-to-read client code.
        Flt x (void) const { return this->coord.first; }
        Flt y (void) const { return this->coord.second; }
        Flt t (void) const { return this->param; }

        //! Use this if you need to invert the y axis
        void invertY (void)
        {
            this->coord.second = -this->coord.second;
        }

        /*!
         * Normalize the length that is made by drawing a vector from the origin to this
         * coordinate.
         */
        void normalize (void)
        {
            BezCoord origin(std::make_pair(0.0f,0.0f));
            Flt len = origin.distanceTo (*this);
            this->coord.first /= len;
            this->coord.second /= len;
        }

        /*!
         * Compute the Euclidean distance from the current coordinate to the given
         * coordinate.
         */
        Flt distanceTo (BezCoord& other) const
        {
            std::pair<Flt,Flt> se;
            se.first = this->x() - other.x();
            se.second = this->y() - other.y();
            return (std::sqrt (se.first*se.first + se.second*se.second));
        }

        //! Horizontal distance between two BezCoords.
        Flt horzDistanceTo (BezCoord& other) const
        {
            return (std::abs(this->x() - other.x()));
        }

        //! Vertical distance between two BezCoords.
        Flt vertDistanceTo (BezCoord& other) const
        {
            return (std::abs(this->y() - other.y()));
        }

        //! Subtract the coordinate c from this BezCoord.
        void subtract (const std::pair<Flt,Flt>& c)
        {
            this->coord.first -= c.first;
            this->coord.second -= c.second;
        }

        //! Subtract the coordinate c from this BezCoord.
        void subtract (const BezCoord& c)
        {
            this->coord.first -= c.x();
            this->coord.second -= c.y();
        }

        //! Add the coordinate c to this BezCoord.
        void add (const std::pair<Flt,Flt>& c)
        {
            this->coord.first += c.first;
            this->coord.second += c.second;
        }

        //! Add the coordinate c to this BezCoord.
        void add (const BezCoord& c)
        {
            this->coord.first += c.x();
            this->coord.second += c.y();
        }

        BezCoord<Flt> operator- (const BezCoord& br) const
        {
            std::pair<Flt,Flt> p;
            p.first = this->coord.first - br.x();
            p.second = this->coord.second - br.y();
            return BezCoord<Flt>(p); // Note returned object contains remaining and param == -1
        }

        friend std::ostream& operator<< (std::ostream& output, const BezCoord& b)
        {
            output << b.t() << "," << b.x() << "," << b.y();
            return output;
        }

    private: // attributes

        /*!
         * Cartesian coordinates of the point. In keeping with SVG, coord.first (x) is
         * positive rightwards and coord.second is positive downwards.
         */
        std::pair<Flt,Flt> coord;

        /*!
         * The parameter value used to obtain this coordinate. Note this is only
         * meaningful when this BezCoord is considered in conjunction with a BezCurve
         * instance.
         *
         * Range is 0 to 1.0. If set to -1.0, then this means "unset".
         */
        Flt param;

        /*!
         * If set >-1, stores the remaining distance to the end point of the curve.
         *
         * Range is 0 to FLOATMAX. If set to -1.0, then this means "unset".
         */
        Flt remaining;

        /*!
         * If this is a null coordinate, set this to true. Note that a BezCoord may have
         * a null coordinate but non-null param or remaining attributes, in the cases
         * where that might be useful.
         */
        bool nullCoordinate;
    };

} // namespace morph
