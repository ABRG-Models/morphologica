/*!
 * A Bezier curve coordinate class.
 */

#ifndef _BEZCOORD_H_
#define _BEZCOORD_H_

#include <utility>
#include <math.h>
#include <ostream>
#include <cmath>

using std::pair;
using std::make_pair;
using std::ostream;
using std::sqrt;
using std::abs;

namespace morph
{
    /*!
     * A class defining a bezier curve coordinate, along with its
     * parameter value and the distance remaining to the end of the
     * curve.
     */
    template <typename Flt>
    class BezCoord
    {
    public: // methods

        /*!
         * Construct empty BezCoord. Defaults to non-null.
         */
        BezCoord () {
            this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = false;
            this->coord.first = 0.0f; this->coord.second = 0.0f;
        }

        /*!
         * Construct empty coordinate, which may or may not be set to null.
         */
        BezCoord (bool nullcoord) {
            this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = nullcoord;
            this->coord.first = 0.0f; this->coord.second = 0.0f;
        }

        /*!
         * Construct a coordinate using the info given.
         */
        //@{
        BezCoord (pair<Flt,Flt> r)
            : coord(r)
            { this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = false; }

        BezCoord (Flt t, pair<Flt,Flt> r)
            : coord(r)
            , param(t)
            { this->remaining = -1.0f; this->nullCoordinate = false; }

        BezCoord (Flt t, pair<Flt,Flt> r, Flt remain)
            : coord(r)
            , param(t)
            , remaining(remain)
            { this->nullCoordinate = false; }
        //@}

        /*!
         * Accessors
         */
        //@{
        pair<Flt,Flt> getCoord (void) const { return this->coord; }
        Flt getParam (void) const { return this->param; }
        Flt getRemaining (void) const { return this->remaining; }
        bool getNullCoordinate (void) const { return this->nullCoordinate; }
        bool isNull (void) const { return this->nullCoordinate; }
        void setCoord (pair<Flt,Flt> c) { this->coord = c; }
        void setParam (Flt p) { this->param = p; }
        void setRemaining (Flt r) { this->remaining = r; }
        void setNullCoordinate (bool b) { this->nullCoordinate = b; }
        //@}

        /*!
         * Access x position, y position and t with single character
         * named accessors, for easy-to-read client code.
         */
        //@{
        Flt x (void) const { return this->coord.first; }
        Flt y (void) const { return this->coord.second; }
        Flt t (void) const { return this->param; }
        //@}

        /*!
         * Use this if you need to invert the y axis
         */
        void invertY (void) {
            this->coord.second = -this->coord.second;
        }

        /*!
         * Normalize the length that is made by drawing a vector from the origin to this
         * coordinate.
         */
        void normalize (void) {
            BezCoord origin(make_pair(0.0f,0.0f));
            Flt len = origin.distanceTo (*this);
            this->coord.first /= len;
            this->coord.second /= len;
        }

        /*!
         * Compute the Euclidean distance from the current coordinate
         * to the given coordinate.
         */
        Flt distanceTo (BezCoord& other) const {
            pair<Flt,Flt> se;
            se.first = this->x() - other.x();
            se.second = this->y() - other.y();
            return (sqrt (se.first*se.first + se.second*se.second));
        }

        /*!
         * Horizontal distance between two BezCoords.
         */
        Flt horzDistanceTo (BezCoord& other) const {
            return (abs(this->x() - other.x()));
        }

        /*!
         * Vertical distance between two BezCoords.
         */
        Flt vertDistanceTo (BezCoord& other) const {
            return (abs(this->y() - other.y()));
        }

        /*!
         * Subtract the coordinate c from this BezCoord.
         */
        void subtract (const pair<Flt,Flt>& c) {
            this->coord.first -= c.first;
            this->coord.second -= c.second;
        }

        /*!
         * Subtract the coordinate c from this BezCoord.
         */
        void subtract (const BezCoord& c) {
            this->coord.first -= c.x();
            this->coord.second -= c.y();
        }

        /*!
         * Add the coordinate c to this BezCoord.
         */
        void add (const pair<Flt,Flt>& c) {
            this->coord.first += c.first;
            this->coord.second += c.second;
        }

        /*!
         * Add the coordinate c to this BezCoord.
         */
        void add (const BezCoord& c) {
            this->coord.first += c.x();
            this->coord.second += c.y();
        }

        /*!
         * Operators
         */
        //@{
        BezCoord operator- (const BezCoord& br) {
            pair<Flt,Flt> p;
            p.first = this->coord.first - br.x();
            p.second = this->coord.second - br.y();
            return BezCoord(p); // Note returned object contains remaining and param == -1
        }

        friend ostream& operator<< (ostream& output, const BezCoord& b) {
#if 0
            output << b.x() << "," << b.y() << " (t=" << b.t() << " rem: " << b.getRemaining() << ")";
            if (b.getNullCoordinate() == true) {
                output << " [null coord]";
            }
#else
            output << b.t() << "," << b.x() << "," << b.y();
#endif
            return output;
        }
        //@}

    private: // attributes

        /*!
         * Cartesian coordinates of the point. In keeping with SVG,
         * coord.first (x) is positive rightwards and coord.second is
         * positive downwards.
         */
        pair<Flt,Flt> coord;

        /*!
         * The parameter value used to obtain this coordinate. Note
         * this is only meaningful when this BezCoord is considered in
         * conjunction with a BezCurve instance.
         *
         * Range is 0 to 1.0. If set to -1.0, then this means "unset".
         */
        Flt param;

        /*!
         * If set >-1, stores the remaining distance to the end point
         * of the curve.
         *
         * Range is 0 to FLOATMAX. If set to -1.0, then this means
         * "unset".
         */
        Flt remaining;

        /*!
         * If this is a null coordinate, set this to true. Note that a
         * BezCoord may have a null coordinate but non-null param or
         * remaining attributes, in the cases where that might be
         * useful.
         */
        bool nullCoordinate;
    };

} // namespace morph

#endif // _BEZCOORD_H_
