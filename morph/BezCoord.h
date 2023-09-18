/*!
 * A Bezier curve coordinate class.
 */

#pragma once

#include <cmath>
#include <ostream>
#include <cmath>
#include <morph/vec.h>

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
        BezCoord()
        {
            this->param = Flt{-1}; this->remaining = Flt{-1}; this->nullCoordinate = false;
            this->coord = {Flt{0}, Flt{0}};
        }

        //! Construct empty coordinate, which may or may not be set to null.
        BezCoord (bool nullcoord)
        {
            this->param = Flt{-1}; this->remaining = Flt{-1}; this->nullCoordinate = nullcoord;
            this->coord = {Flt{0}, Flt{0}};
        }

        //! Construct using just a 2D coordinate
        BezCoord (const morph::vec<Flt, 2>& r)
            : coord(r)
            { this->param = Flt{-1}; this->remaining = Flt{-1}; this->nullCoordinate = false; }

        //! Construct with coordinate and corresponding t parameter
        BezCoord (Flt t, const morph::vec<Flt, 2>& r)
            : coord(r)
            , param(t)
            { this->remaining = Flt{-1}; this->nullCoordinate = false; }

        //! Construct with coord & t and also set the para remaining
        BezCoord (Flt t, const morph::vec<Flt, 2>& r, Flt remain)
            : coord(r)
            , param(t)
            , remaining(remain)
            { this->nullCoordinate = false; }

        // Accessors
        //morph::vec<Flt, 2> getCoord() const { return this->coord; } // now public
        //Flt getParam() const { return this->param; } // now public
        Flt getRemaining() const { return this->remaining; }
        bool getNullCoordinate() const { return this->nullCoordinate; }
        bool isNull() const { return this->nullCoordinate; }
        //void setCoord (const morph::vec<Flt, 2>& c) { this->coord = c; } // now public
        //void setParam (Flt p) { this->param = p; }
        void setRemaining (Flt r) { this->remaining = r; }
        void setNullCoordinate (bool b) { this->nullCoordinate = b; }

        // Single character accessors, for easy-to-read client code.
        Flt x() const { return this->coord[0]; }
        Flt y() const { return this->coord[1]; }
        Flt t() const { return this->param; }

        //! Use this if you need to invert the y axis
        void invertY() { this->coord[1] = -this->coord[1]; }

        /*!
         * Normalize the length that is made by drawing a vector from the origin to this
         * coordinate.
         */
        void normalize()
        {
            BezCoord origin(morph::vec<Flt, 2>({0.0f,0.0f}));
            Flt len = origin.distanceTo (*this);
            this->coord /= len;
        }

        /*!
         * Compute the Euclidean distance from the current coordinate to the given
         * coordinate.
         */
        Flt distanceTo (BezCoord& other) const { return (this->coord - other.coord).length(); }

        //! Horizontal distance between two BezCoords.
        Flt horzDistanceTo (BezCoord& other) const { return (std::abs(this->x() - other.x())); }

        //! Vertical distance between two BezCoords.
        Flt vertDistanceTo (BezCoord& other) const { return (std::abs(this->y() - other.y())); }

        //! Subtract the coordinate c from this BezCoord.
        void subtract (const morph::vec<Flt, 2>& c) { this->coord -= c; }

        //! Subtract the coordinate c from this BezCoord.
        void subtract (const BezCoord& c) { this->coord -= c.coord; }

        //! Add the coordinate c to this BezCoord.
        void add (const morph::vec<Flt, 2>& c) { this->coord += c; }

        //! Add the coordinate c to this BezCoord.
        void add (const BezCoord& c) { this->coord += c.coord; }

        BezCoord<Flt> operator- (const BezCoord& br) const
        {
            morph::vec<Flt, 2> p;
            p = this->coord - br.coord;
            return BezCoord<Flt>(p); // Note returned object contains remaining and param == -1
        }

        friend std::ostream& operator<< (std::ostream& output, const BezCoord& b)
        {
            output << b.t() << "," << b.x() << "," << b.y();
            return output;
        }

    public: // attributes
        /*!
         * Cartesian coordinates of the point. In keeping with SVG, coord.first (x) is
         * positive rightwards and coord.second is positive downwards.
         */
        morph::vec<Flt, 2> coord;

        /*!
         * The parameter value used to obtain this coordinate. Note this is only
         * meaningful when this BezCoord is considered in conjunction with a BezCurve
         * instance.
         *
         * Range is 0 to 1.0. If set to -1.0, then this means "unset".
         */
        Flt param;

    private: // attributes
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
