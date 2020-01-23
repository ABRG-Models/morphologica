/*!
 * Implementation of BezCurve methods
 *
 * See https://pomax.github.io/bezierinfo for a world of Bezier knowledge. I drew on
 * that page to do the curve fitting algorithm.
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

// For matrix representation of a BezCurve, useful when line fitting.
#include <armadillo>

using namespace std;
using morph::BezCoord;
using arma::mat;

void
morph::BezCurve::init (void)
{
    this->order = this->controls.size()-1;
    this->linlength = sqrtf ( (controls[order].first-controls[0].first)*(controls[order].first-controls[0].first)
                              + (controls[order].second-controls[0].second)*(controls[order].second-controls[0].second) );
    this->linlengthscaled = this->scale * this->linlength;
    this->matrixSetup();
}

void
morph::BezCurve::fit (vector<pair<float,float>> points)
{
    // Zero out the controls vector
    this->controls.clear();

    // Set the order for the curve
    size_t n = points.size();
    this->order = n - 1;

    // This call to matrixSetup will set up this->M (required for the fit)
    this->matrixSetup();

    int i = 0;

    arma::Mat<float> P (n, 2, arma::fill::zeros);
    for (auto p : points) {
        P(i,0) = p.first;
        P(i++,1) = p.second;
    }
    cout << "P:\n" << P << endl;

    // Compute candidate t values for the points.
    i = 0;
    pair<float, float> p_l;
    bool first = true;
    arma::Mat<float> S (n, 1, arma::fill::zeros);
    float total_len = 0.0f;
    for (auto p : points) {
        if (first) {
            //S (i) = 0.0f; // No need
            first = false;
        } else {
            register float xdiff = (p.first - p_l.first);
            register float ydiff = (p.second - p_l.second);
            register float len = sqrtf (xdiff*xdiff + ydiff*ydiff);
            total_len += len;
            S(i,0) = total_len;
        }
        p_l = p;
        ++i;
    }
    for (i = 0; i < n; ++i) {
        S(i,0) = S(i,0) / total_len;
    }
    // S now contains the t values for the fitting.
    cout << "S:\n" << S << endl;

    // Make TT matrix (T with double bar in
    // https://pomax.github.io/bezierinfo/#curvefitting) This takes each t and makes
    // one column containing all the powers of t relevant to the order that we're
    // looking for.
    arma::Mat<float> TT (n, n, arma::fill::ones);
    for (i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            float s = S(i,0);
            TT(i, j) = pow (s, j);
        }
    }
    cout << "TT:\n" << TT;

    // Magic matrix incantation to find the best set of coordinates:
    this->C = this->M.i() * (TT.t()*TT).i() * TT.t() * P;
    cout << "Drumroll... C is\n" << this->C;

    // Copy elements of C into this->controls.
    this->controls.clear();
    for (i = 0; i<n; ++i) {
        this->controls.push_back (make_pair (this->C(i,0), this->C(i,1)));
    }

    // Re-init
    this->init();
}


morph::BezCurve::BezCurve (void)
{
    this->order = 0;
}

morph::BezCurve::BezCurve (vector<pair<float,float>> cp)
{
    this->controls = cp;
    this->init();
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           vector<pair<float,float>> cp)
{
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.insert (this->controls.end(), cp.begin(), cp.end());
    this->controls.push_back (fp);
    this->init();
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1,
                           pair<float,float> c2)
{
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (c1);
    this->controls.push_back (c2);
    this->controls.push_back (fp);
    this->init();
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp,
                           pair<float,float> c1)
{
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (c1);
    this->controls.push_back (fp);
    this->init();
}

morph::BezCurve::BezCurve (pair<float,float> ip,
                           pair<float,float> fp)
{
    this->controls.clear();
    this->controls.push_back (ip);
    this->controls.push_back (fp);
    this->init();
}

void
morph::BezCurve::matrixSetup (void)
{
    // Scheme to write out the matrix comes from Cohen & Riesenfeld (1982) General
    // Matrix Representations...

    // Check order here
    if (this->order >= PascalRows) {
        stringstream ee;
        ee << "This code is limited to Bezier Curves of order " << (PascalRows-1)
           << " by the current size of the morph::Pascal lookup table";
        throw runtime_error (ee.str());
    }
    if (this->order == 0) {
        throw runtime_error ("No curves if order=0");
    }

    // Set up M.
    int m = (int)this->order;
    int mp = m+1;
    int r = 0;
    this->M.set_size (mp, mp);
    this->M.zeros();
    for (int i=0; i<mp; ++i) { // i is column
        for (r=0; r<mp-i; ++r) { // r is row
            float element = static_cast<float>(BezCurve::binomial_lookup (m, i))
                * static_cast<float>(BezCurve::binomial_lookup (m-i, m-i-r))
                * pow (-1.0f, static_cast<float>(m-i-r));
            // Ensure the matrix is inverted 'm-i', not just 'i'
            this->M(m-i, r) = element;
        }
    }
    //cout << "matrixSetup M:\n" << this->M;

    // Set up C
    this->C.set_size (mp, 2);
    this->C.zeros();
    r = 0;
    for (auto c : this->controls) {
        // Just put the controls into C, in order.
        this->C(r,0) = c.first;
        this->C(r,1) = c.second;
        ++r;
    }
    //cout << "matrixSetup C:\n" << this->C << endl;

    this->MC = this->M * this->C;
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
    //DBG2 ("Called computePoint(float t = "  << t << ") order=" << this->order);
    switch (this->order) {
    case 1:
        return this->computePointLinear (t);
    case 2:
        return this->computePointQuadratic (t);
    case 3:
        return this->computePointCubic (t);
    default:
        // Default to matrix, as this is faster than computePointGeneral
        return this->computePointMatrix (t);
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

string
morph::BezCurve::outputControl (void) const
{
    stringstream ss;
    for (auto c : this->controls) {
        ss << c.first << "," << c.second << endl;
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
    this->checkt (t);
    float t_ = 1-t;
    pair<float,float> b;

    // x
    b.first = pow(t_, this->order) * this->controls[0].first;
    for(unsigned int k=1; k<this->order; k++) {
        b.first += static_cast<float> (BezCurve::binomial_lookup(this->order, k))
            * pow (t_, this->order-k) * pow (t, k) * this->controls[k].first;
    }
    b.first += pow (t, this->order) * this->controls[this->order].first;
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
morph::BezCurve::computePointMatrix (float t) const
{
    this->checkt(t);
    int mp = this->order+1;
    arma::Mat<float> T(1, mp, arma::fill::ones);// First element is one anyway
    for (int i = 1; i<mp; ++i) {
        T(i) = pow (t, static_cast<float>(i));
    }
    arma::Mat<float> bp = T * this->MC;
    return BezCoord (t, make_pair(bp(0), bp(1)));
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
