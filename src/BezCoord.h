/*!
 * A Bezier curve coordinate class.
 */

#ifndef _BEZCOORD_H_
#define _BEZCOORD_H_

#include <utility>
#include <math.h>
#include <ostream>

using std::pair;
using std::ostream;

namespace morph
{
    /*!
     * A class defining a bezier curve coordinate, along with its
     * parameter value and the distance remaining to the end of the
     * curve.
     */
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
        BezCoord (pair<float,float> r)
            : coord(r)
            { this->param = -1.0f; this->remaining = -1.0f; this->nullCoordinate = false; }

        BezCoord (float t, pair<float,float> r)
            : coord(r)
            , param(t)
            { this->remaining = -1.0f; this->nullCoordinate = false; }

        BezCoord (float t, pair<float,float> r, float remain)
            : coord(r)
            , param(t)
            , remaining(remain)
            { this->nullCoordinate = false; }
        //@}

        /*!
         * Accessors
         */
        //@{
        pair<float,float> getCoord (void) const { return this->coord; }
        float getParam (void) const { return this->param; }
        float getRemaining (void) const { return this->remaining; }
        bool getNullCoordinate (void) const { return this->nullCoordinate; }
        void setCoord (pair<float,float> c) { this->coord = c; }
        void setParam (float p) { this->param = p; }
        void setRemaining (float r) { this->remaining = r; }
        void setNullCoordinate (bool b) { this->nullCoordinate = b; }
        //@}

        /*!
         * Access x position, y position and t with single character
         * named accessors, for easy-to-read client code.
         */
        //@{
        float x (void) const { return this->coord.first; }
        float y (void) const { return this->coord.second; }
        float t (void) const { return this->param; }
        //@}

        /*!
         * Compute the Euclidean distance from the current coordinate
         * to the given coordinate.
         */
        float distanceTo (BezCoord& other) const {
            pair<float,float> se;
            se.first = this->x() - other.x();
            se.second = this->y() - other.y();
            return (sqrtf (se.first*se.first + se.second*se.second));
        }

        /*!
         * Horizontal distance between two BezCoords.
         */
        float horzDistanceTo (BezCoord& other) const {
            return (fabs(this->x() - other.x()));
        }

        /*!
         * Vertical distance between two BezCoords.
         */
        float vertDistanceTo (BezCoord& other) const {
            return (fabs(this->y() - other.y()));
        }

        /*!
         * Operators
         */
        //@{
        BezCoord operator- (const BezCoord& br) {
            pair<float,float> p;
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
         * Cartesian coordinates of the point.
         */
        pair<float,float> coord;

        /*!
         * The parameter value used to obtain this coordinate. Note
         * this is only meaningful when this BezCoord is considered in
         * conjunction with a BezCurve instance.
         *
         * Range is 0 to 1.0. If set to -1.0, then this means "unset".
         */
        float param;

        /*!
         * If set >-1, stores the remaining distance to the end point
         * of the curve.
         *
         * Range is 0 to FLOATMAX. If set to -1.0, then this means
         * "unset".
         */
        float remaining;

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
