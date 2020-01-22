/*!
 * Implementation of BezCurve methods
 */

// To enable debug cout messages:
//#define DEBUG 1
//#define DEBUG2 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

#include "BezCurve.h"
#include <stdexcept>
#include <math.h>
#include <iostream>
#include <sstream>

using namespace std;
using morph::BezCoord;

morph::BezCurve::BezCurve (vector<pair<float,float>> cp)
{
    this->controls = cp;
    this->order = cp.size()-1;
    this->linlength = sqrtf ( (controls[order].first-controls[0].first)*(controls[order].first-controls[0].first)
                              + (controls[order].second-controls[0].second)*(controls[order].second-controls[0].second) );
    this->linlengthscaled = this->scale * this->linlength;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           vector<pair<float,float>> cp)
{
    this->linlength = sqrtf ( (fp.first-ip.first)*(fp.first-ip.first)
                              + (fp.second-ip.second)*(fp.second-ip.second) );
    this->linlengthscaled = this->scale * this->linlength;
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.insert (this->controls.end(), cp.begin(), cp.end());
    this->controls.push_back (fp);
    this->order = cp.size()-1;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1,
                           pair<float,float> c2)
{
    this->linlength = sqrtf ( (fp.first-ip.first)*(fp.first-ip.first)
                              + (fp.second-ip.second)*(fp.second-ip.second) );
    this->linlengthscaled = this->scale * this->linlength;
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (c1);
    this->controls.push_back (c2);
    this->controls.push_back (fp);
    this->order = 3;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1)
{
    this->linlength = sqrtf ( (fp.first-ip.first)*(fp.first-ip.first)
                              + (fp.second-ip.second)*(fp.second-ip.second) );
    this->linlengthscaled = this->scale * this->linlength;
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (c1);
    this->controls.push_back (fp);
    this->order = 2;
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp)
{
    this->linlength = sqrtf ( (fp.first-ip.first)*(fp.first-ip.first)
                              + (fp.second-ip.second)*(fp.second-ip.second) );
    this->linlengthscaled = this->scale * this->linlength;
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (fp);
    this->order = 1;
}

vector<BezCoord>
morph::BezCurve::computePoints (unsigned int n) const
{
    vector<BezCoord> rtn;
    for (unsigned int i = 0; i < n; ++i) {
        float t = i/static_cast<float>(n);
        rtn.push_back (this->computePoint (t));
    }
    return rtn;
}

vector<BezCoord>
morph::BezCurve::computePoints (float l, float firstl) const
{
    DBG2 ("computePoints (float l="<<l<<", float firstl="<<firstl<<") called");
    vector<BezCoord> rtn;
    float t = 0.0f;
    bool lastnull = false;

    if (firstl > 0.0f) {
        // firstl is the desired distance to the first point and, if
        // non-zero, overrides l for the first point.
        BezCoord b = this->computePoint (t, firstl);
        rtn.push_back (b);
        t = b.t();
        lastnull = b.getNullCoordinate();
    }

    // This searches forward to try to find a point which is 'l' further on. If at any
    // point t exceeds 1.0, we have to break out.
    while (t != 1.0f && lastnull == false) {
        BezCoord b = this->computePoint (t, l);
        rtn.push_back (b);
        t = rtn.back().t();
        lastnull = b.getNullCoordinate();
    }
    return rtn;
}

vector<BezCoord>
morph::BezCurve::computePointsHorz (float x) const
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
morph::BezCurve::computePoint (float t) const
{
    DBG2 ("Called computePoint(float t = "  << t << ") order=" << this->order);
    switch (this->order) {
    case 1:
        return this->computePointLinear (t);
    case 2:
        return this->computePointQuadratic (t);
    case 3:
        return this->computePointCubic (t);
    default:
        return this->computePointGeneral (t);
    }
}

BezCoord
morph::BezCurve::computePoint (float t, float l) const
{
    DBG2 ("Called computePoint(float t="<<t<<", float l="<<l<<")");
    switch (this->order) {
    case 1:
        return this->computePointLinear (t, l);
    case 2:
    case 3:
    default:
        return this->computePointBySearch (t, l);
    }
}

BezCoord
morph::BezCurve::computePointLinear (float t, float l) const
{
    DBG2 ("Called computePointLinear(float t="<<t<<", float l="<<l<<")");
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
    float dt = l/this->linlengthscaled;
    t = t+dt;
    return this->computePointLinear (t);
}

string
morph::BezCurve::output (unsigned int numPoints) const
{
    stringstream ss;
    vector<BezCoord> points = this->computePoints (numPoints);
    vector<BezCoord>::const_iterator i = points.begin();
    while (i != points.end()) {
        if (!i->isNull()) {
            ss << i->x() << "," << i->y() << endl;
        }
        ++i;
    }
    return ss.str();
}

string
morph::BezCurve::output (float step) const
{
    stringstream ss;
    vector<BezCoord> points = this->computePoints (step);
    vector<BezCoord>::const_iterator i = points.begin();
    while (i != points.end()) {
        if (!i->isNull()) {
            ss << i->x() << "," << i->y() << endl;
        }
        ++i;
    }
    return ss.str();
}

void
morph::BezCurve::setScale (const float s)
{
    this->scale = s;
    this->linlengthscaled = this->scale * this->linlength;
}

void
morph::BezCurve::setLthresh (const float l)
{
    this->lthresh = l;
}

BezCoord
morph::BezCurve::computePointLinear (float t) const
{
    DBG2 ("computePointLinear (t=" << t << ")");
    this->checkt(t);
    pair<float,float> b;
    b.first = ((1-t) * this->controls[0].first + t * this->controls[1].first) * this->scale;
    b.second = ((1-t) * this->controls[0].second + t * this->controls[1].second) * this->scale;
    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointQuadratic (float t) const
{
    this->checkt (t);
    pair<float,float> b;
    float t_ = 1-t;
    b.first = (t_ * t_ * this->controls[0].first
               + 2 * t_ * t * this->controls[1].first
               + t * t * this->controls[2].first) * this->scale;
    b.second = (t_ * t_ * this->controls[0].second
                + 2 * t_ * t * this->controls[1].second
                + t * t * this->controls[2].second) * this->scale;
    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointCubic (float t) const
{
    this->checkt (t);
    pair<float,float> b;
    float t_ = 1-t;
    b.first = (t_ * t_ * t_ * this->controls[0].first
               + 3 * t_ * t_ * t * this->controls[1].first
               + 3 * t_ * t * t * this->controls[2].first
               + t * t * t * this->controls[3].first) * this->scale;
    b.second = (t_ * t_ * t_ * this->controls[0].second
                + 3 * t_ * t_ * t * this->controls[1].second
                + 3 * t_ * t * t * this->controls[2].second
                + t * t * t * this->controls[3].second) * this->scale;
    return BezCoord(t, b);
}

unsigned int
morph::BezCurve::binomial_lookup (unsigned int n, unsigned int k) {
    /* To get the values from row n, where n starts at 0 (and ends at N-1), you step
       along a number given by the triangle sequence (n(n+1)/2) and then read n+1
       values. OR to get n,k, step along a number given by the triangle sequence
       (n(n+1)/2) and then step another k space to the result. */
    unsigned int idx = (n * (n+1) / 2) + k;
    return morph::Pascal[idx];
}

BezCoord
morph::BezCurve::computePointGeneral (float t) const
{
    if (this->order >= PascalRows) {
        stringstream ee;
        ee << "Limited to Bezier Curves order " << (PascalRows-1)
           << " by current size of morph::Pascal lookup table";
        throw runtime_error (ee.str());
    }

    if (this->order == 0) {
        throw runtime_error ("No curve if order=0");
    }

    this->checkt (t);
    float t_ = 1-t;
    pair<float,float> b;

    // x
    b.first = pow(t_, this->order) * this->controls[0].first;
    for(unsigned int k=1; k<this->order; k++) {
        b.first += static_cast<float> (BezCurve::binomial_lookup(this->order, k))
            * pow (t_, this->order-k) * pow (t, k) * this->controls[k].first;
    }
    b.first += pow(t, this->order) * this->controls[this->order].first;
    b.first *= this->scale;
    // y
    b.second = pow(t_, this->order) * this->controls[0].second;
    for(unsigned int k=1; k<this->order; k++) {
        b.second += static_cast<float> (BezCurve::binomial_lookup(this->order, k))
            * pow (t_, this->order-k) * pow (t, k) * this->controls[k].second;
    }
    b.second += pow(t, this->order) * this->controls[this->order].second;
    b.second *= this->scale;

    return BezCoord(t, b);
}

BezCoord
morph::BezCurve::computePointBySearch (float t, float l) const
{
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

    // On every call, compute a threshold. lthresh is a percentage, so compute the
    // absolute threshold, lt as a percentage of l.
    float lt = this->lthresh * 0.01f * l;

    // Do a binary search to find the value of dt which gives a b2 that is l further on
    BezCoord b2 (true);
    bool finished = false;
    while (!finished && ((t+dt) <= 1.0f)) {

        // Compute position of candidate point dt beyond t in param space
        b2 = this->computePoint (t+dt);
        float dl = b1.distanceTo (b2);
        if (fabs(l-dl) < lt) {
            // Stop here.
            finished = true;
        } else {
            if (dl > l) {
                dtmax = dt;
            } else { // dl < l
                dtmin = dt;
            }
            dt = dtmin + (dtmax-dtmin)/2.0f;
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
morph::BezCurve::computePointBySearchHorz (float t, float x) const
{
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

    // How close we need to be to the target x for a given choice of dt.
    float lt = this->lthresh * 0.01f * x;

    // Do a binary search to find the value of dt
    BezCoord b2 (true);
    bool finished = false;
    float lastdt = 0.0f;
    while (!finished && ((t+dt) <= 1.0f) && lastdt != dt) {

        // Compute position of candidate point dt beyound t in param space
        b2 = this->computePoint (t+dt);
        float dx = b1.horzDistanceTo (b2);
        //cout << "t+dt= " << t+dt << ", dx = " << dx << endl;
        if (fabs(x-dx) < lt) {
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
morph::BezCurve::checkt (float t) const
{
    if (t < 0.0 || t > 1.0) {
        throw std::runtime_error ("t out of range [0,1]");
    }
}

pair<float,float>
morph::BezCurve::getInitialPointUnscaled (void) const
{
    return this->controls[0];
}

pair<float,float>
morph::BezCurve::getFinalPointUnscaled (void) const
{
    return this->controls[this->order];
}

pair<float,float>
morph::BezCurve::getInitialPointScaled (void) const
{
    pair<float,float> rtn = this->controls[0];
    rtn.first *= this->scale;
    rtn.second *= this->scale;
    return rtn;
}

pair<float,float>
morph::BezCurve::getFinalPointScaled (void) const
{
    pair<float,float> rtn = this->controls[this->order];
    rtn.first *= this->scale;
    rtn.second *= this->scale;
    return rtn;
}
