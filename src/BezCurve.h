/*!
 * A Bezier curve class which allows the computation of Cartesian
 * coordinates of points on a Bezier curve which is specified using a
 * parameter (often called t) which is in the range [0, 1]
 */

#ifndef _BEZCURVE_H_
#define _BEZCURVE_H_

#include <utility>
#include <vector>

#include "BezCoord.h"

using std::pair;
using std::vector;

namespace morph
{
    /*!
     * An enumerated type to define the algorithm which is used to
     * compute the points on the curve.
     */
    enum BezCurveType {
        BEZCUBIC,
        BEZQUADRATIC,
        BEZLINEAR,
        N_BEZCURVETYPE
    };

    /*!
     * A class defining a bezier curve
     */
    class BezCurve
    {
    public: // methods
        /*!
         * Construct a quadratic Bezier curve with a specification of
         * the curve as inital and final position with two control
         * points.
         */
        BezCurve (pair<float,float> ip,
                  pair<float,float> fp,
                  pair<float,float> c1,
                  pair<float,float> c2);

        /*!
         * Construct a cubic Bezier curve with a specification of the
         * curve as inital and final position with a single control point
         */
        BezCurve (pair<float,float> ip,
                  pair<float,float> fp,
                  pair<float,float> c1);

        /*!
         * Construct a linear Bezier curve for production of straight
         * lines.
         */
        BezCurve (pair<float,float> ip,
                  pair<float,float> fp);

        /*!
         * Compute n points on the curve whose parameters, t, are
         * equally spaced in parameter space. The first point will be
         * the start of the curve (t==0) and the last point will be at
         * the end of the curve (t==1).
         */
        vector<BezCoord> computePoints (int n);

        /*!
         * Compute points on the curve which are distance l from each
         * other in Cartesian space. This will return 1 or more points
         * in the vector. The last point in the vector will be a
         * nullCoordinate BezCoord which will contain the Euclidean
         * distance to the end of the curve.
         *
         * If firstl is set and non-zero, then the first point will be
         * a Cartesian distance firstl from the initial point of the
         * curve, rather than being a distance l from the initial
         * point.
         */
        vector<BezCoord> computePoints (float l, float firstl = 0.0f);

        /*!
         * Get a vector of points on the curve with horizontal spacing
         * x.
         */
        vector<BezCoord> computePointsHorz (float x);

        /*!
         * Compute one point on the curve, distance t along the curve
         * from the starting position with t in range [0,1]
         */
        BezCoord computePoint (float t);

        /*!
         * Compute one point on the curve, starting at the curve point
         * which is found for parameter value t and extending a
         * (Euclidean) distance l along the curve from the starting
         * position.
         */
        BezCoord computePoint (float t, float l);

    private: // methods
        /*!
         * Compute one point on the linear curve, distance t along the
         * curve from the starting position.
         */
        BezCoord computePointLinear (float t);

        /*!
         * Compute one point on the linear curve, starting at the
         * curve point which is found for parameter value t and
         * extending a distance l along the curve from the starting
         * position.
         *
         * The key to this is to compute a change in t from the l that
         * you want to move along the line. It's not hard to do the
         * maths for the linear case; see LinearBez1.jpg and
         * LinearBez2.jpg for the sums.
         */
        BezCoord computePointLinear (float t, float l);

        /*!
         * Compute one point on the quadratic curve, distance t along
         * the curve from the starting position.
         */
        BezCoord computePointQuadratic (float t);

        /*!
         * Compute one point on the cubic curve, distance t along the
         * curve from the starting position.
         */
        BezCoord computePointCubic (float t);

        /*!
         * A computePoint starting from the point for parameter value
         * t and going to a point which is Euclidean distance l from
         * the starting point.
         *
         * This one uses a binary search to find the next point, and
         * works for quadratic and cubic Bezier curves for which it is
         * difficult to compute the t that would give a Euclidean
         * extension l (it would work for linear curves too).
         */
        BezCoord computePointBySearch (float t, float l);

        /*!
         * Like computePointsBySearch, but instead of using the
         * Euclidean distance, space points with x between them in the
         * first coordinate - the horizonal coordinate.
         */
        BezCoord computePointBySearchHorz (float t, float x);

        /*!
         * Test that t is in range [0,1]. Throw exception otherwise.
         */
        void checkt (float t);

    private: // attributes
        /*!
         * Initial and final positions
         */
        //@{
        pair<float,float> p0;
        pair<float,float> p1;
        //@}

        /*!
         * Control points
         */
        //@{
        pair<float,float> control1;
        pair<float,float> control2;
        //@}

        /*!
         * The type of the bezier curve.
         */
        BezCurveType beztype = BEZCUBIC;
    };

} // namespace morph

#endif // _BEZCURVE_H_
