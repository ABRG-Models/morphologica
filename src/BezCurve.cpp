/*!
 * Implementation of BezCurve methods
 */

#include "BezCurve.h"
#include <stdexcept>
#include <math.h>
#include <iostream>

using namespace std;
using morph::BezCoord;

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1,
                           pair<float,float> c2)
{
    this->beztype = morph::BEZCUBIC;
    this->p0 = ip;
    this->p1 = fp;
    this->control1 = c1;
    this->control2 = c2;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1)
{
    this->beztype = morph::BEZQUADRATIC;
    this->p0 = ip;
    this->p1 = fp;
    this->control1 = c1;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp)
{
    this->beztype = morph::BEZLINEAR;
    this->p0 = ip;
    this->p1 = fp;
}

vector<BezCoord>
morph::BezCurve::computePoints (int n)
{
    vector<BezCoord> rtn;
    for (int i = 0; i < n; ++i) {
        float t = i/static_cast<float>(n);
        rtn.push_back (this->computePoint (t));
    }
    return rtn;
}

vector<BezCoord>
morph::BezCurve::computePoints (float l)
{
    vector<BezCoord> rtn;
    float t = 0.0f;
    bool lastnull = false;
    while (t != 1.0f && lastnull == false) {
        BezCoord b = this->computePoint (t, l);
        rtn.push_back (b);
        t = rtn.back().t();
        lastnull = b.getNullCoordinate();
    }
    return rtn;
}

vector<BezCoord>
morph::BezCurve::computePointsHorz (float x)
{
    vector<BezCoord> rtn;
    float t = 0.0f;
    bool lastnull = false;
    while (t != 1.0f && lastnull == false) {
        BezCoord b = this->computePointBySearchHorz (t, x);
        rtn.push_back (b);
        t = rtn.back().t();
        lastnull = b.getNullCoordinate();
    }
    return rtn;
}

BezCoord
morph::BezCurve::computePoint (float t)
{
    switch (this->beztype) {
    case morph::BEZLINEAR:
        return this->computePointLinear (t);
    case morph::BEZQUADRATIC:
        return this->computePointQuadratic (t);
    case morph::BEZCUBIC:
    default:
        return this->computePointCubic (t);
    }
}

BezCoord
morph::BezCurve::computePoint (float t, float l)
{
    switch (this->beztype) {
    case morph::BEZLINEAR:
        return this->computePointLinear (t, l);
    case morph::BEZQUADRATIC:
    case morph::BEZCUBIC:
    default:
        return this->computePointBySearch (t, l);
    }
}

BezCoord
morph::BezCurve::computePointLinear (float t, float l)
{
    BezCoord b1 = this->computePoint (t);
    BezCoord e1 = this->computePoint (1.0f);
    float toEnd = b1.distanceTo (e1);
    if (toEnd < l) {
        // Return null coordinate as the result and set remaining to
        // toEnd and the last param to t.
        BezCoord rtn (true);
        rtn.setRemaining (toEnd);
        rtn.setParam (t);
        return rtn;
    }
    // Compute new t from l.
    float denom = sqrtf ( (p1.first-p0.first)*(p1.first-p0.first) + (p1.second-p0.second)*(p1.second-p0.second) );
    float dt = l/denom;
    t = t+dt;
    return this->computePointLinear (t);
}

BezCoord
morph::BezCurve::computePointLinear (float t)
{
    this->checkt(t);
    pair<float,float> b;
    b.first = (1-t) * this->p0.first + t * this->p1.first;
    b.second = (1-t) * this->p0.second + t * this->p1.second;
    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointQuadratic (float t)
{
    this->checkt (t);
    pair<float,float> b;
    float t_ = 1-t;
    b.first = t_ * t_ * this->p0.first
        + 2 * t_ * t * this->control1.first
        + t * t * this->p1.first;
    b.second = t_ * t_ * this->p0.second
        + 2 * t_ * t * this->control1.second
        + t * t * this->p1.second;
    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointCubic (float t)
{
    this->checkt (t);
    pair<float,float> b;
    float t_ = 1-t;
    b.first = t_ * t_ * t_ * this->p0.first
        + 3 * t_ * t_ * t * this->control1.first
        + 3 * t_ * t * t * this->control2.first
        + t * t * t * this->p1.first;
    b.second = t_ * t_ * t_ * this->p0.second
        + 3 * t_ * t_ * t * this->control1.second
        + 3 * t_ * t * t * this->control2.second
        + t * t * t * this->p1.second;
    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointBySearch (float t, float l)
{
    //cout << "Starting parameter: " << t << endl;

    // Min and max of possible range for dt to make a step of length l in posn space
    float dtmin = 0.0f;
    float dtmax = 1.0f - t;

    // First guess for dt. Arb. units in parameter space.
    float dt = dtmin + (dtmax-dtmin)/2.0f;

    BezCoord b1 = this->computePoint (t);

    // Find distance from the initial position to the end of the
    // curve. If this is a shorter distance than l, then return.
    BezCoord e1 = this->computePoint (1.0f);
    float toEnd = b1.distanceTo (e1);
    if (toEnd < l) {
        // Return null coordinate as the result and set remaining to
        // toEnd and the last param to t.
        BezCoord rtn (true);
        rtn.setRemaining (toEnd);
        rtn.setParam (t);
        return rtn;
    }

    // How close we need to be to the target l for a given choice of dt.
    float lthresh = 0.001; // arb. units in position space (not parameter space)

    // Do a binary search to find the value of dt
    BezCoord b2 (true);
    bool finished = false;
    while (!finished && ((t+dt) <= 1.0f)) {

        // Compute position of candidate point dt beyound t in param space
        b2 = this->computePoint (t+dt);
        float dl = b1.distanceTo (b2);
        if (fabs(l-dl) < lthresh) {
            // Stop here.
            finished = true;
        } else {
            if (dl > l) {
                dtmax = dt;
            } else { // dl < l
                dtmin = dt;
            }
            dt = dtmin + (dtmax-dtmin)/2.0f;
            //cout << "dt: " << dt << endl;
        }
    }

    if (!finished) {
        // Return a null coordinate
        BezCoord rtn (true);
        return rtn;
    }

    return b2;
}

BezCoord
morph::BezCurve::computePointBySearchHorz (float t, float x)
{
    //cout << "Starting parameter: " << t << "\t";

    // Min and max of possible range for dt to make a step of length l in posn space
    float dtmin = 0.0f;
    float dtmax = 1.0f - t;

    // First guess for dt. Arb. units in parameter space.
    float dt = dtmin + (dtmax-dtmin)/2.0f;

    BezCoord b1 = this->computePoint (t);

    // Find distance from the initial position to the end of the
    // curve. If this is a shorter distance than l, then return.
    BezCoord e1 = this->computePoint (1.0f);
    float toEnd = b1.horzDistanceTo (e1);
    if (toEnd < x) {
        // Return null coordinate as the result and set remaining to
        // toEnd and the last param to t.
        BezCoord rtn (true);
        rtn.setRemaining (toEnd);
        rtn.setParam (t);
        return rtn;
    }

    // How close we need to be to the target l for a given choice of dt.
    float lthresh = 0.001; // arb. units in position space (not parameter space)

    // Do a binary search to find the value of dt
    BezCoord b2 (true);
    bool finished = false;
    float lastdt = 0.0f;
    while (!finished && ((t+dt) <= 1.0f) && lastdt != dt) {

        // Compute position of candidate point dt beyound t in param space
        b2 = this->computePoint (t+dt);
        float dx = b1.horzDistanceTo (b2);
        //cout << "t+dt= " << t+dt << ", dx = " << dx << endl;
        if (fabs(x-dx) < lthresh) {
            // Stop here.
            finished = true;
        } else {
            if (dx > x) {
                dtmax = dt;
            } else { // dl < l
                dtmin = dt;
            }
            lastdt = dt;
            dt = dtmin + (dtmax-dtmin)/2.0f;
            //cout << "dt: " << dt << endl;
        }
    }

    if (!finished) {
        // Return a null coordinate
        BezCoord rtn (true);
        return rtn;
    }

    return b2;
}

void
morph::BezCurve::checkt (float t)
{
    if (t < 0.0 || t > 1.0) {
        throw std::runtime_error ("t out of range [0,1]");
    }
}
