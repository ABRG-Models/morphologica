#ifndef _BEZCURVE_H_
#define _BEZCURVE_H_

#include <utility>
using std::pair;
#include <vector>
using std::vector;
#include <array>
using std::array;
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <limits>
using std::numeric_limits;
#include <stdexcept>
using std::runtime_error;
#include <cmath>
using std::sqrt;
using std::pow;
using std::abs;
using std::cos;
using std::acos;
using std::atan2;
using std::sin;
#include <armadillo>
#include "MathConst.h"
#include "MathAlgo.h"
#include "NM_Simplex.h"
using morph::NM_Simplex;
using morph::NM_Simplex_State;
#include <random>
using std::random_device;
using std::mt19937;
using std::mt19937_64;
using std::uniform_real_distribution;

#include "BezCoord.h"
#define DBGSTREAM std::cout
#include "MorphDbg.h"

namespace morph
{
    /*!
     * Store the first N=21 rows of Pascal's triangle in a linear array. To get the
     * values from row n, where n starts at 0 (and ends at N-1), you step along a number
     * given by the triangle sequence (n(n+1)/2) and then read n+1 values. The triangle
     * has a total number of elements given by N(N+1)/2, which for 21 rows is 21(22)/2 =
     * 210.
     */
    const array<unsigned int, 231> Pascal =
    {1,
     1,1,
     1,2,1,
     1,3,3,1,
     1,4,6,4,1,
     1,5,10,10,5,1,
     1,6,15,20,15,6,1,
     1,7,21,35,35,21,7,1,
     1,8,28,56,70,56,28,8,1,
     1,9,36,84,126,126,84,36,9,1,
     1,10,45,120,210,252,210,120,45,10,1,
     1,11,55,165,330,462,462,330,165,55,11,1,
     1,12,66,220,495,792,924,792,495,220,66,12,1,
     1,13,78,286,715,1287,1716,1716,1287,715,286,78,13,1,
     1,14,91,364,1001,2002,3003,3432,3003,2002,1001,364,91,14,1,
     1,15,105,455,1365,3003,5005,6435,6435,5005,3003,1365,455,105,15,1,
     1,16,120,560,1820,4368,8008,11440,12870,11440,8008,4368,1820,560,120,16,1,
     1,17,136,680,2380,6188,12376,19448,24310,24310,19448,12376,6188,2380,680,136,17,1,
     1,18,153,816,3060,8568,18564,31824,43758,48620,43758,31824,18564,8568,3060,816,153,18,1,
     1,19,171,969,3876,11628,27132,50388,75582,92378,92378,75582,50388,27132,11628,3876,969,171,19,1,
     1,20,190,1140,4845,15504,38760,77520,125970,167960,184756,167960,125970,77520,38760,15504,4845,1140,190,20,1};
    //! How many rows in the table above.
    const unsigned int PascalRows = 21;

    /*!
     * A Bezier curve class which allows the computation of Cartesian coordinates
     * (though with x right, y down, and hence a left-hand coordinate system) of points
     * on a Bezier curve which is specified using a parameter (often called t) which is
     * in the range [0, 1]
     */
    template <class Flt>
    class BezCurve
    {
    public: // methods

        //! Do-nothing constructor
        BezCurve (void) {
            this->order = 0;
        }

        /*!
         * Construct a Bezier curve of order cp.size()-1 with the initial and final
         * points making up part of cp.
         */
        BezCurve (vector<pair<Flt, Flt>> cp) {
            this->C.set_size (cp.size(), 2);
            int i = 0;
            for (auto c : cp) {
                this->C(i,0) = c.first;
                this->C(i,1) = c.second;
                ++i;
            }
            this->init();
        }

        /*!
         * Construct a Bezier curve using the control points provided in the matrix @cmat.
         */
        BezCurve (const arma::Mat<Flt>& cmat) {
            //this->C.copy_size (cmat); // necessary?
            this->C = cmat;
            this->init();
        }

        /*!
         * Construct a cubic Bezier curve with a specification of the curve as inital
         * and final position with two control points.
         */
        BezCurve (pair<Flt,Flt> ip,
                  pair<Flt,Flt> fp,
                  pair<Flt,Flt> c1,
                  pair<Flt,Flt> c2) {

            this->C.set_size (4,2);
            this->C(0,0) = ip.first;
            this->C(0,1) = ip.second;
            this->C(1,0) = c1.first;
            this->C(1,1) = c1.second;
            this->C(2,0) = c2.first;
            this->C(2,1) = c2.second;
            this->C(3,0) = fp.first;
            this->C(3,1) = fp.second;

            this->init();
        }

        /*!
         * Construct a quadratic Bezier curve with a specification of the curve as
         * inital and final position with a single control point
         */
        BezCurve (pair<Flt,Flt> ip,
                  pair<Flt,Flt> fp,
                  pair<Flt,Flt> c1) {
            this->C.set_size (3,2);
            this->C(0,0) = ip.first;
            this->C(0,1) = ip.second;
            this->C(1,0) = c1.first;
            this->C(1,1) = c1.second;
            this->C(2,0) = fp.first;
            this->C(2,1) = fp.second;
            this->init();
        }

        /*!
         * Construct a linear Bezier curve for production of straight lines.
         */
        BezCurve (pair<Flt,Flt> ip,
                  pair<Flt,Flt> fp) {
            this->C.set_size (2,2);
            this->C(0,0) = ip.first;
            this->C(0,1) = ip.second;
            this->C(1,0) = fp.first;
            this->C(1,1) = fp.second;
            this->init();
        }

        /*!
         * Construct a Bezier curve of order cp.size()+1
         */
        BezCurve (pair<Flt,Flt> ip,
                  pair<Flt,Flt> fp,
                  vector<pair<Flt, Flt>> cp){
            unsigned int n_ctrls = cp.size()+2;
            this->C.set_size (n_ctrls, 2);
            this->C(0,0) = ip.first;
            this->C(0,1) = ip.second;
            unsigned int i = 1;
            for (auto cpi : cp) {
                this->C(i,0) = cpi.first;
                this->C(i,1) = cpi.second;
                ++i;
            }
            this->C(n_ctrls-1,0) = fp.first;
            this->C(n_ctrls-1,1) = fp.second;
            this->init();
        }

        void updateControls (vector<pair<Flt, Flt>> cp) {
            this->C.set_size (cp.size(), 2);
            int i = 0;
            for (auto c : cp) {
                this->C(i,0) = c.first;
                this->C(i,1) = c.second;
                ++i;
            }
            this->init();
        }

        /*!
         * Fit a curve to @points, lining up with the curve @c. Assumes this curve
         * appends to the end of @c. *May also modify @c*. Set @optimize to true to try
         * out experimental fit improvements.
         */
        void fit (vector<pair<Flt, Flt>> points, BezCurve<Flt>& c, bool optimize=false) {

            this->fit (points);

            // preceding control points.
            vector<pair<Flt, Flt>> prec_ctrl = c.getControls();
            size_t len = prec_ctrl.size();
            if (len < 3) {
                return;
            }

            // va is vector from join to the previous ctrl
            Flt va_x = prec_ctrl[len-2].first - prec_ctrl[len-1].first; // "prev ctrl - join"
            Flt va_y = prec_ctrl[len-2].second - prec_ctrl[len-1].second;
            // vb is vector from join to the next ctrl.
            Flt vb_x = C(1,0) - C(0,0); // "next ctrl - join"
            Flt vb_y = C(1,1) - C(0,1);
            // Use atan2 to get angles with direction here.
            Flt ang_a = atan2 (va_y, va_x); // NB: args in order y, x!
            Flt ang_b = atan2 (vb_y, vb_x);
            // theta is the angle between vector a and vector b.
            Flt theta = ang_a - ang_b;
#ifdef DEBUG__
            cout << "ang_a = " << ang_a << " rads "
                 << (ang_a * 180 / static_cast<Flt>(morph::PI_D)) << " deg" << endl;
            cout << "ang_b = " << ang_b << " rads "
                 << (ang_b * 180 / static_cast<Flt>(morph::PI_D)) << " deg" << endl;
            cout << "theta = " << theta << " rads "
                 << (theta * 180 / static_cast<Flt>(morph::PI_D)) << " deg" << endl;
#endif
            // phi is the angle that conforms to: theta + 2 phi = pi radians
            // thus 2 phi = pi - theta
            // thus   phi = 1/2(pi - theta)
            Flt phi = 0.5 * (static_cast<Flt>(morph::PI_D) - theta);
            // BUT if phi > pi/2 (more than a right angle) then don't add 90-and-a-bit, instead
            // subtract 90-less-a-bit:
            if (phi > static_cast<Flt>(morph::PI_OVER_2_D)) {
                phi = static_cast<Flt>(morph::PI_D) - phi;
            }
#ifdef DEBUG__
            cout << "phi = " << phi << " rads "
                 << (phi * 180 / static_cast<Flt>(morph::PI_D)) << " deg" << endl;
#endif
            // Construct rotn matrix
            arma::Mat<Flt> rotmat (2,2);
            rotmat(0,0) = cos (-phi);
            rotmat(0,1) = sin (-phi);
            rotmat(1,0) = -rotmat(0,1);
            rotmat(1,1) = rotmat(0,0);

            // Now we rotate each point by phi
            // The point at the end of the curve
            arma::Mat<Flt> p0 = C.row(0);
            arma::Mat<Flt> pm1 (1,2);
            pm1(0,0) = prec_ctrl[len-2].first;
            pm1(0,1) = prec_ctrl[len-2].second;
            // Offset so we rotate about p0
            arma::Mat<Flt> pm1_r = pm1 - p0;
            // Apply rotation
            arma::Mat<Flt> pm1_r_after = pm1_r * rotmat;

            // Change two elements in the rotation matrix to reverse the rotational direction
            rotmat(1,0) = rotmat(0,1);
            rotmat(0,1) = -rotmat(0,1); // sin (-phi);
            arma::Mat<Flt> pm2 = C.row(1);
            arma::Mat<Flt> pm2_r = pm2 - p0;
            arma::Mat<Flt> pm2_r_after = pm2_r * rotmat;

            arma::Mat<Flt> pm1_r_final = pm1_r_after + p0;
            arma::Mat<Flt> pm2_r_final = pm2_r_after + p0;

            C.row(1) = pm2_r_final;

            this->init();

            // Update the other curve's control points, also.
            prec_ctrl[len-2].first = pm1_r_final(0,0);
            prec_ctrl[len-2].second = pm1_r_final(0,1);
            c.updateControls (prec_ctrl);

#ifdef DEBUG__
            cout << "Preceding controls: " << c.outputControl();
            cout << "C (after line-up):\n" << this->C;
            // First, need a cost function:
#endif
            // If client code requests NOT to optimize, then return
            if (!optimize) { return; }

            /*
             * Nelder-Mead gradient descent optimization of intermediate control points.
             */

            // Optimization stage. Move control points other than those we just fixed to
            // be in line with each other, to minimize the deviation of this curve and
            // @c from the user-points provided.
            cout << "Optimization..." << endl;

            Flt startsos = this->computeObjective (points);
            cout << "Objective with no optimization: " << startsos << endl;
            arma::Mat<Flt> Copy = this->C;

            // Convert the middle rows of C to vector<Flt> to be the first NM vertex
            vector<Flt> v0;
            int startrow = 2;
            int endrow = 2; // 2 means don't change the angle of the end of the curve
            for (int r = startrow; r < (int)C.n_rows-endrow; ++r) {
                v0.push_back (this->C(r,0));
                v0.push_back (this->C(r,1));
            }

            if (v0.empty()) {
                cout << "No further optimization possible" << endl;
                return;
            }

            // Make a set of random vertices to init the NM_Simplex algo with.
            vector<vector<Flt>> nm_vertices;

            // First, push back the existing set of controls as the first NM vertex
            nm_vertices.push_back (v0);

            // Set up random
            random_device rd;  // Will be used to obtain a seed for the random number engine
            mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
            uniform_real_distribution<Flt> dis(static_cast<Flt>(0.0), static_cast<Flt>(1.0));

            // Add some more vertices:
            Flt propchange = static_cast<Flt>(0.2);
            Flt propchangeov2 = propchange / static_cast<Flt>(2.0);
            for (size_t i = 0; i < v0.size(); ++i) {
                vector<Flt> v;
                for (size_t j = 0; j<v0.size(); ++j) {
                    // Perturbate v0[j] a bit and add to vector<Flt>
                    Flt v_j = v0[j];
                    Flt v_1 = (v0[j]*propchange);
                    Flt rn = dis(gen);
                    //cout << "rn: " << dis(gen) << endl;
                    v_j += (v_1 * rn);
                    v_j -= (v0[j]*propchangeov2);
                    //cout << "Pushing back " << v_j << endl;
                    v.push_back (v_j);
                }
#ifdef DEBUG__
                cout << "v has size " << v.size();
                cout << ", is ";
                for (auto vv : v) {
                    cout << vv << ",";
                }
#endif
                nm_vertices.push_back (v);
#ifdef DEBUG__
                cout << " nm_vertices has size " << nm_vertices.size() << endl;
#endif
            }

            // Start out with a simplex with a vertex at the centroid of the domain vertices, and
            // then two other vertices at the first domain vertex (v) and its neighbour (vn).
            NM_Simplex<Flt> simp (nm_vertices);
            // Set a termination threshold for the SD of the vertices of the simplex
            simp.termination_threshold = static_cast<Flt>(0.00001);//2.0 * numeric_limits<Flt>::epsilon();
            // Set an operation limit, in case the above threshold can't be reached
            simp.too_many_operations = 1000;

            // Tweak the NM parameters to help it find solutions
            // The reflection coefficient
            simp.alpha = 0.1; // 1
            // The expansion coefficient
            simp.gamma = 0.2; // 2
            // The contraction coefficient
            simp.rho = 0.05; // .5
            // The shrink coefficient
            simp.sigma = 0.05; // .5

            //cout << "Set up simplex with " << simp.n << " vertices" << endl;

            while (simp.state != NM_Simplex_State::ReadyToStop) {

                if (simp.state == NM_Simplex_State::NeedToComputeThenOrder) {
                    // 1. apply objective to each vertex
                    for (unsigned int i = 0; i <= simp.n; ++i) {
                        this->setCFromV (simp.vertices[i], startrow);
                        this->init(); // Re-setup this BezCurve
                        simp.values[i] = this->computeObjective (points);
                        //cout << "Obj for that one was " << simp.values[i] << endl;
                    }
                    simp.order();

                } else if (simp.state == NM_Simplex_State::NeedToOrder) {
                    simp.order();

                } else if (simp.state == NM_Simplex_State::NeedToComputeReflection) {
                    this->setCFromV (simp.xr, startrow);
                    this->init(); // Re-setup this BezCurve
                    Flt val = this->computeObjective (points);
                    //cout << "Obj for reflected one was " << val << endl;
                    simp.apply_reflection (val);

                } else if (simp.state == NM_Simplex_State::NeedToComputeExpansion) {
                    this->setCFromV (simp.xe, startrow);
                    this->init(); // Re-setup this BezCurve
                    Flt val = this->computeObjective (points);
                    //cout << "Obj for expanded one was " << val << endl;
                    simp.apply_expansion (val);

                } else if (simp.state == NM_Simplex_State::NeedToComputeContraction) {
                    this->setCFromV (simp.xc, startrow);
                    this->init(); // Re-setup this BezCurve
                    Flt val = this->computeObjective (points);
                    //cout << "Obj for contracted one was " << val << endl;
                    simp.apply_contraction (val);
                }
                //cout << "C:\n" << C;
            }
            cout << "NM finished in " << simp.operation_count << " simplex change operations)" << endl;
            vector<Flt> vP = simp.best_vertex();
            Flt min_sos = simp.best_value();
            cout << "Best value had objective = " << min_sos << endl;
            if (min_sos < startsos) {
                cout << "This was an improvement" << endl;
                this->setCFromV (simp.best_vertex(), startrow);
                this->init(); // Re-setup this BezCurve
                Flt bestval = this->computeObjective (points);
                cout << "FINISHED! Best approximation:\n" << this->C << "has value " << bestval << endl;
            } else {
                cout << "Optimization failed to improve. Back to C." << endl;
                this->C = Copy;
                this->init(); // Re-setup this BezCurve
            }
        }

        Flt computeObjective (const vector<pair<Flt, Flt>>& points) const {
            // Compute relative positions of pairs in @points
            vector<Flt> sample_t;
            sample_t.push_back (static_cast<Flt>(0.0));
            Flt totaldist = static_cast<Flt>(0.0);
            for (size_t i = 1; i < points.size(); ++i) {
                Flt lindist = MathAlgo<Flt>::distance (points[i-1], points[i]);
                sample_t.push_back (lindist);
                totaldist += lindist;
            }
            vector<pair<Flt,Flt>> curvePoints;
            for (size_t i = 0; i < sample_t.size(); ++i) {
                sample_t[i] /= totaldist;
                // Have the t parameter value to sample our Bezier curve at now...
                BezCoord<Flt> bc = this->computePoint (sample_t[i]);
                curvePoints.push_back (make_pair(bc.x(),bc.y()));
            }
            // Can now compare points and curvePoints.
            if (curvePoints.size() != points.size()) {
                cout << "Can't optimize" << endl;
                return static_cast<Flt>(-1.0);
            }
            Flt sos = static_cast<Flt>(0.0);
            for (size_t i = 0; i < points.size(); ++i) {
                sos += MathAlgo<Flt>::distance_sq (points[i], curvePoints[i]);
            }

#if 0
            // Add a penalty for the length of the curve, also, which should be as close
            // to the linear length from point to point.
            Flt clen = this->computeLength(50);
            Flt distpart = (clen-totaldist)*(clen-totaldist);
            cout << "sos part: " << sos << ", distance part: " << distpart << endl;
            return sos + distpart;
#else
            return sos;
#endif
        }

#if 0
        /*!
         * Fit a curve to @points, ensuring that the line segment between points[0]
         * (aka fitted_ctrls[0]) and fitted_ctrls[1] is parallel with the line segment
         * between @c and points[0]. This make it possible to get the best fit
         * curve, which also lines up with a previous best fit curve.
         *
         * How to achieve this?
         *
         * 1) Could do BezCurve<>::fit(points) first, then change fitted_ctrls[1] by
         * rotating it until it lines up with extCtrl---points[0]
         *ccc
         * 2) Can I do a version of the fitting which then finds the optimum position
         * on the line segment by a gradient descent to minimise the objective error? Possibly.
         */
        void fit (vector<pair<Flt, Flt>> points, const pair<Flt, Flt> c) {

            // First fit with the analytic solution for points on their
            // own. this->controls and this->C then contain the fitted points.
            this->fit (points);

            // c is the control point
            cout << "external control point is (" << c.first << "," << c.second << ")\n";
            // this->C.row(0) is the start and should be same as points[0]
            cout << "C(0,:) is (" << C(0,0) << "," << C(0,1) << ")\n";
            cout << "C(1,:) (to be changed) is ("  << C(1,0) << "," << C(1,1) << ")\n";

            // Compute distance from control point 0 to control point 1.
            Flt xdiff = C(0,0) - C(1,0);
            Flt ydiff = C(0,1) - C(1,1);
            Flt control0to1 = sqrt (xdiff*xdiff + ydiff+ydiff);

            // Compute vector from c to control point 0
            Flt v_x = C(0,0) - c.first;
            Flt v_y = C(0,1) - c.second;
            Flt v_len = sqrt (v_x*v_x + v_y*v_y);
            v_x /= v_len;
            v_y /= v_len;

            // With unit vector v_x/y can now make up the vector of length control0to1
            // to make our new control point:
            Flt newCtrl_x = C(0,0) + v_x * control0to1;
            Flt newCtrl_y = C(0,1) + v_y * control0to1;

            // This is a possible first stab:
            C(1,0) = newCtrl_x;
            C(1,1) = newCtrl_y;
            // cout << "C(1,:) (updated) is ("  << C(1,0) << "," << C(1,1) << ")\n";

            // Last job, vary contorl0to1 between 0 and 2*control0to1 and find the
            // best one.
            cout << "Write me. Optimizing version" << endl;

            // Last thing, re-init to set up the matrices again after changing C.
            this->init();
        }
#endif
        /*!
         * Using the given points, make this a best-fit Bezier curve with
         * points.size()-1 control points.
         */
        void fit (vector<pair<Flt, Flt>> points) {

            // Set the order for the curve
            int n = points.size();
            this->order = n - 1;

            // Empty C here, in advance of this->matrixSetup(), and make sure it has the
            // right size, though it can just be zeros.
            this->C = arma::Mat<Flt> (n, 2, arma::fill::zeros);

            // This call to matrixSetup will set up this->M (required for the fit)
            this->matrixSetup();

            int i = 0;

            // Note that you really need double precision in the matrices whilst
            // computing a Bezier best fit. If we use single precision, the fits are
            // only good up to Bezier order 4 or 5, rather than 8-10.
            arma::Mat<double> P (n, 2, arma::fill::zeros);
            for (auto p : points) {
                P(i,0) = p.first;
                P(i++,1) = p.second;
            }
            //cout << "P:\n" << P;

            // Compute candidate t values for the points.
            i = 0;
            arma::Mat<double> D (n, 1, arma::fill::zeros);
            arma::Mat<double> S (n, 1, arma::fill::zeros);
            double total_len = static_cast<Flt>(0.0);
            for (i = 1; i < n; ++i) {
                double xdiff = P(i,0) - P(i-1,0);
                double ydiff = P(i,1) - P(i-1,1);
                double len = sqrt (xdiff*xdiff + ydiff*ydiff);
                total_len += len;
                D(i,0) = total_len;
            }
            for (i = 0; i < n; ++i) {
                S(i,0) = D(i,0) / total_len;
            }
            // S now contains the t values for the fitting.
            //cout << "S:\n" << S;

            // Make TT matrix (T with double bar in
            // https://pomax.github.io/bezierinfo/#curvefitting) This takes each t and
            // makes one column containing all the powers of t relevant to the order
            // that we're looking for.
            arma::Mat<double> TT (n, n, arma::fill::ones);
            for (i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    double s = S(i,0);
                    TT(i, j) = pow (s, j);
                }
            }
            //cout << "TT:\n" << TT;

            // Could we check/use the preprocessor to avoid this line if Flt is double?
            arma::Mat<double> Md = arma::conv_to<arma::Mat<double>>::from (this->M);

            // Magic matrix incantation to find the best set of coordinates:
            arma::Mat<double> Cd = Md.i() * (TT.t()*TT).i() * TT.t() * P;

            //cout << "Drumroll... C is\n" << Cd;

            // Cast back to Flts
            this->C = arma::conv_to<arma::Mat<Flt>>::from (Cd);

            // Re-init
            this->init();
        }

        /*!
         * Obtain the derivative of this Bezier curve
         */
        BezCurve<Flt> derivative (void) const {
            arma::Mat<Flt> deriv_cp(this->order, 2);
            for (unsigned int i = 0; i < this->order; ++i) {
                deriv_cp.row(i) = this->order * (this->C.row(i+1) - this->C.row(i));
            }
            return morph::BezCurve<Flt> (deriv_cp);
        }

        /*!
         * Return (control points for) two Bezier curves that split up this one.
         *
         * Using the matrix representation find, from this->C, a C1 and C2 that trace
         * the same trajectory.
         */
        pair<arma::Mat<Flt>, arma::Mat<Flt>>
        split (Flt z) const {
            int n = this->order + 1;
            // 'z prime':
            Flt zp = z-static_cast<Flt>(1.0);
            arma::Mat<Flt> C1 (n, 2, arma::fill::zeros);
            arma::Mat<Flt> C2 (n, 2, arma::fill::zeros);
            Flt sign0 = static_cast<Flt>(1.0);
            Flt sign = sign0;
            arma::Mat<Flt> Q (n, n, arma::fill::zeros);
            for (int i = 0; i < n; ++i) {
                sign = sign0;
                for (int j = 0; j <= i; ++j) {
                    Flt binom = static_cast<Flt>(BezCurve::binomial_lookup(i, j));
                    Q(i,j) = sign * binom * pow(z, j) * pow (zp, i-j);
                    sign = sign > static_cast<Flt>(0.0) ? static_cast<Flt>(-1.0) : static_cast<Flt>(1.0);
                }
                sign0 = sign0 > static_cast<Flt>(0.0) ? static_cast<Flt>(-1.0) : static_cast<Flt>(1.0);
            }
            C1 = Q * this->C;
            // Shift rows then flip
            for (int i = 0; i < n; ++i) {
                Q.row(i) = arma::shift (Q.row(i), (n-i-1));
            }
            C2 = arma::flipud (Q) * this->C;

            return make_pair(C1, C2);
        }

        /*!
         * Compute n points on the curve whose parameters, t, are equally spaced in
         * parameter space. The first point will be the start of the curve (t==0) and
         * the last point will be at the end of the curve (t==1).
         */
        vector<BezCoord<Flt>> computePoints (unsigned int n) const {
            vector<BezCoord<Flt>> rtn;
            for (unsigned int i = 0; i < n; ++i) {
                Flt t = i/static_cast<Flt>(n);
                rtn.push_back (this->computePoint (t));
            }
            return rtn;
        }

        /*!
         * Compute points on the curve which are distance l from each other in Cartesian
         * space. This will return 1 or more points in the vector. The last point in the
         * vector will be a nullCoordinate BezCoord which will contain the Euclidean
         * distance to the end of the curve.
         *
         * If firstl is set and non-zero, then the first point will be a Cartesian
         * distance firstl from the initial point of the curve, rather than being a
         * distance l from the initial point.
         */
        vector<BezCoord<Flt>> computePoints (Flt l, Flt firstl = static_cast<Flt>(0.0)) const {
            DBG2 ("computePoints (Flt l="<<l<<", Flt firstl="<<firstl<<") called");
            vector<BezCoord<Flt>> rtn;
            Flt t = static_cast<Flt>(0.0);
            bool lastnull = false;

            if (firstl > static_cast<Flt>(0.0)) {
                // firstl is the desired distance to the first point and, if non-zero,
                // overrides l for the first point.
                BezCoord<Flt> b = this->computePoint (t, firstl);
                rtn.push_back (b);
                t = b.t();
                lastnull = b.getNullCoordinate();
            }

            // This searches forward to try to find a point which is 'l' further on. If
            // at any point t exceeds 1.0, we have to break out.
            while (t != static_cast<Flt>(1.0) && lastnull == false) {
                BezCoord<Flt> b = this->computePoint (t, l);
                rtn.push_back (b);
                t = rtn.back().t();
                lastnull = b.getNullCoordinate();
            }
            return rtn;
        }

        /*!
         * Get a vector of points on the curve with horizontal spacing x.
         */
        vector<BezCoord<Flt>> computePointsHorz (Flt x) const {
            vector<BezCoord<Flt>> rtn;
            Flt t = static_cast<Flt>(0.0);
            bool lastnull = false;
            while (t != static_cast<Flt>(1.0) && lastnull == false) {
                BezCoord<Flt> b = this->computePointBySearchHorz (t, x);
                rtn.push_back (b);
                t = rtn.back().t();
                lastnull = b.getNullCoordinate();
            }
            return rtn;
        }

        /*!
         * Compute one point on the curve, distance t along the curve from the starting
         * position with t in range [0,1]. This chooses either optimzed quartic/cubic
         * functions, or defaults to the matrix computation method.
         */
        BezCoord<Flt> computePoint (Flt t) const {
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

        /*!
         * Compute a Bezier curve of general order using the matrix method.
         */
        BezCoord<Flt> computePointMatrix (Flt t) const {
            this->checkt(t);
            int mp = this->order+1;
            arma::Mat<Flt> T(1, mp, arma::fill::ones);// First element is one anyway
            for (int i = 1; i < mp; ++i) {
                T(i) = pow (t, static_cast<double>(i));
            }
            arma::Mat<Flt> bp = T * this->MC;
            return BezCoord<Flt> (t, make_pair(static_cast<Flt>(bp(0)), static_cast<Flt>(bp(1))));
        }

        /*!
         * Compute a Bezier curve of general order using the conventional method.
         */
        BezCoord<Flt> computePointGeneral (Flt t) const {
            this->checkt (t);
            Flt t_ = 1-t;
            pair<Flt,Flt> b;
            // x
            b.first = pow(t_, this->order) * this->C(0,0);
            for(unsigned int k=1; k<this->order; k++) {
                b.first += static_cast<Flt> (BezCurve::binomial_lookup(this->order, k))
                    * pow (t_, this->order-k) * pow (t, k) * this->C(k,0);
            }
            b.first += pow (t, this->order) * this->C(this->order,0);
            b.first *= this->scale;
            // y
            b.second = pow(t_, this->order) * this->C(0,1);
            for (unsigned int k=1; k<this->order; k++) {
                b.second += static_cast<Flt> (BezCurve::binomial_lookup(this->order, k))
                    * pow (t_, this->order-k) * pow (t, k) * this->C(k,1);
            }
            b.second += pow(t, this->order) * this->C(this->order,1);
            b.second *= this->scale;

            return BezCoord<Flt>(t, b);
        }

        /*!
         * Compute one point on the curve, starting at the curve point which is found
         * for parameter value t and extending a (Euclidean) distance l along the curve
         * from the starting position.
         *
         * If it is not possible, without exceeding t, to advance a distance l, then set
         * a null BezCoord and return that.
         */
        BezCoord<Flt> computePoint (Flt t, Flt l) const {
            switch (this->order) {
            case 1:
                return this->computePointLinear (t, l);
            case 2:
            case 3:
            default:
                return this->computePointBySearch (t, l);
            }
        }

        /*!
         * Compute the tangent and normal at t.
         */
        pair<BezCoord<Flt>, BezCoord<Flt>> computeTangentNormal (const Flt t) const {
            BezCoord<Flt> tang;
            if (this->C.n_rows == 2/*this->controls.size() == 2*/) {
                // Can't compute tangent using the derivative as derivative would be a
                // curve with a single control point. The tangent to a line is
                // simply the line:
                tang = this->computePoint (t);
            } else {
                BezCurve<Flt> deriv = this->derivative();
                tang = deriv.computePoint (t);
            }
            tang.normalize();
            BezCoord<Flt> norm = tang; // copies the parameter
            // rotate norm:
            norm.setCoord (make_pair(-tang.y(), tang.x()));
            return make_pair (tang, norm);
        }

        /*!
         * For debugging - output, as a string, the BezCoords of this curve, choosing
         * numPoints points evenly spaced in the parameter space t=[0,1].
         */
        string output (unsigned int numPoints) const {
            stringstream ss;
            vector<BezCoord<Flt>> points = this->computePoints (numPoints);
            typename vector<BezCoord<Flt>>::const_iterator i = points.begin();
            while (i != points.end()) {
                if (!i->isNull()) {
                    ss << i->x() << "," << i->y() << endl;
                }
                ++i;
            }
            return ss.str();
        }

        /*!
         * For debugging/file use. Output, as a string, the BezCoords of this curve with
         * the step size step in Cartesian space.
         */
        string output (Flt step) const {
            stringstream ss;
            vector<BezCoord<Flt>> points = this->computePoints (step);
            typename vector<BezCoord<Flt>>::const_iterator i = points.begin();
            while (i != points.end()) {
                if (!i->isNull()) {
                    ss << i->x() << "," << i->y() << endl;
                }
                ++i;
            }
            return ss.str();
        }

        /*!
         * Output the control points.
         */
        string outputControl (void) const {
            stringstream ss;
            ss << this->C;
            return ss.str();
        }

        /*!
         * A setter for the scaling factor.
         */
        void setScale (const Flt s) {
            this->scale = s;
            this->linlengthscaled = this->scale * this->linlength;
        }

        /*!
         * A setter for the length threshold.
         */
        void setLthresh (const Flt l) {
            this->lthresh = l;
        }

        /*!
         * Getters for p0 and p1, the initial and final positions on the curve, either
         * unscaled or scaled by @scale
         */
        //@{
        pair<Flt,Flt> getInitialPointUnscaled (void) const {
            pair<Flt,Flt> ip_unscaled;
            ip_unscaled.first = this->C(0,0);
            ip_unscaled.second = this->C(0,1);
            return ip_unscaled;
        }

        pair<Flt,Flt> getFinalPointUnscaled (void) const {
            pair<Flt,Flt> fp_unscaled;
            fp_unscaled.first = this->C(this->order,0);
            fp_unscaled.second = this->C(this->order,1);
            return fp_unscaled;
        }

        pair<Flt,Flt> getInitialPointScaled (void) const {
            pair<Flt,Flt> ip_scaled;
            ip_scaled.first = this->scale * this->C(0,0);
            ip_scaled.second = this->scale * this->C(0,1);
            return ip_scaled;
        }

        pair<Flt,Flt> getFinalPointScaled (void) const {
            pair<Flt,Flt> fp_scaled;
            fp_scaled.first = this->scale * this->C(this->order,0);
            fp_scaled.second = this->scale * this->C(this->order,1);
            return fp_scaled;
        }

        //! Getter for the control points in vector pair format
        vector<pair<Flt,Flt>> getControls (void) const {
            vector<pair<Flt,Flt>> rtn;
            for (unsigned int r = 0; r<this->C.n_rows; ++r) {
                rtn.push_back (make_pair (this->C(r,0), this->C(r,1)));
            }
            return rtn;
        }

        //@}

        /*!
         * Get the order of the curve
         */
        unsigned int getOrder (void) const {
            return this->order;
        }

    private: // methods

        /*!
         * Perform common initialization tasks.
         */
        void init (void) {
            this->order = this->C.n_rows-1;
            this->linlength = sqrt ((C(order,0)-C(0,0)) * (C(order,0) - C(0,0))
                                    + (C(order,1)-C(0,1)) * (C(order,1) - C(0,1)));
            this->linlengthscaled = this->scale * this->linlength;
            this->matrixSetup();
        }

        //! Set C from the vector for floats vf, which ONLY changes the rows of C from startrow and on.
        void setCFromV (const vector<Flt>& vf, int r) {
            //cout << "Setting C from row " << r << endl;
            for (size_t i = 0; i<vf.size(); i+=2) {
                this->C(r,0) = vf[i];
                this->C(r,1) = vf[i+1];
                //cout << "Set C row " << r << " to ";
                //cout.precision(12);
                //cout << C(r,0) << "," << C(r,1)<<endl;
                ++r;
            }
            //--r;
            //cout << "Last row set in C was " << r << endl;
        }

        /*!
         * Compute an approximation to the distance along the curve, by computing
         * npoints and summing their linear separations.
         */
        Flt computeLength (unsigned int npoints) const {
            vector<BezCoord<Flt>> pts = this->computePoints (npoints);
            Flt dist = static_cast<Flt>(0.0);
            for (size_t i = 1; i<pts.size(); ++i) {
                dist += MathAlgo<Flt>::distance (pts[i-1].getCoord(), pts[i].getCoord());
            }
            return dist;
        }

        /*!
         * Compute one point on the linear curve, distance t along the curve from the
         * starting position.
         */
        BezCoord<Flt> computePointLinear (Flt t) const {
            DBG2 ("computePointLinear (t=" << t << ")");
            this->checkt(t);
            pair<Flt,Flt> b;
            b.first =  ((1-t) * this->C(0,0) + t * this->C(1,0)) * this->scale;
            b.second = ((1-t) * this->C(0,1) + t * this->C(1,1)) * this->scale;
            return BezCoord<Flt>(t, b);
        }

        /*!
         * Compute one point on the linear curve, starting at the curve point which is
         * found for parameter value t and extending a distance l along the curve from
         * the starting position.
         *
         * The key to this is to compute a change in t from the l that you want to move
         * along the line. It's not hard to do the maths for the linear case; see
         * LinearBez1.jpg and LinearBez2.jpg for the sums.
         */
        BezCoord<Flt> computePointLinear (Flt t, Flt l) const {
            DBG2 ("Called computePointLinear(Flt t="<<t<<", Flt l="<<l<<")");
            BezCoord<Flt> b1 = this->computePoint (t);
            BezCoord<Flt> e1 = this->computePoint (static_cast<Flt>(1.0));
            Flt toEnd = b1.distanceTo (e1);
            if (toEnd < l) {
                // Return null coordinate as the result and set remaining to toEnd and
                // the last param to t.
                BezCoord<Flt> rtn (true);
                rtn.setRemaining (toEnd);
                rtn.setParam (t);
                return rtn;
            }
            // Compute new t from l.
            Flt dt = l/this->linlengthscaled;
            t = t+dt;
            return this->computePointLinear (t);
        }

        /*!
         * Compute one point on the quadratic curve, distance t along the curve from the
         * starting position.
         */
        BezCoord<Flt> computePointQuadratic (Flt t) const {
            this->checkt (t);
            pair<Flt,Flt> b;
            Flt t_ = 1-t;
            b.first = (t_ * t_ * this->C(0,0)
                       + 2 * t_ * t * this->C(1,0)
                       + t * t * this->C(2,0)) * this->scale;
            b.second = (t_ * t_ * this->C(0,1)
                        + 2 * t_ * t * this->C(1,1)
                        + t * t * this->C(2,1)) * this->scale;
            return BezCoord<Flt>(t, b);
        }

        /*!
         * Compute one point on the cubic curve, distance t along the curve from the
         * starting position.
         */
        BezCoord<Flt> computePointCubic (Flt t) const {
            this->checkt (t);
            pair<Flt,Flt> b;
            Flt t_ = 1-t;
            b.first = (t_ * t_ * t_ * this->C(0,0)
                       + 3 * t_ * t_ * t * this->C(1,0)
                       + 3 * t_ * t * t * this->C(2,0)
                       + t * t * t * this->C(3,0)) * this->scale;
            b.second = (t_ * t_ * t_ * this->C(0,1)
                        + 3 * t_ * t_ * t * this->C(1,1)
                        + 3 * t_ * t * t * this->C(2,1)
                        + t * t * t * this->C(3,1)) * this->scale;
            return BezCoord<Flt>(t, b);
        }

        /*!
         * Look up the binomial coefficient (n,k) from morph::Pascal.
         */
        static unsigned int binomial_lookup (unsigned int n, unsigned int k) {
            /* To get the values from row n, where n starts at 0 (and ends at N-1), you
               step along a number given by the triangle sequence (n(n+1)/2) and then
               read n+1 values. OR to get n,k, step along a number given by the triangle
               sequence (n(n+1)/2) and then step another k space to the result. */
            unsigned int idx = (n * (n+1) / 2) + k;
            return morph::Pascal[idx];
        }

        /*!
         * A computePoint starting from the point for parameter value t and going to a
         * point which is Euclidean distance l from the starting point.
         *
         * This one uses a binary search to find the next point, and works for quadratic
         * and cubic Bezier curves for which it is difficult to compute the t that would
         * give a Euclidean extension l (it would work for linear curves too).
         */
        BezCoord<Flt> computePointBySearch (Flt t, Flt l) const {
            // Min and max of possible range for dt to make a step of length l in posn space
            Flt dtmin = static_cast<Flt>(0.0);
            Flt dtmax = static_cast<Flt>(1.0) - t;

            // First guess for dt. Arb. units in parameter space.
            Flt dt = dtmin + (dtmax-dtmin)/static_cast<Flt>(2.0);

            BezCoord<Flt> b1 = this->computePoint (t);

            // Find distance from the initial position to the end of the
            // curve. If this is a shorter distance than l, then return.
            BezCoord<Flt> e1 = this->computePoint (static_cast<Flt>(1.0));
            Flt toEnd = b1.distanceTo (e1);
            if (toEnd < l) {
                // Return null coordinate as the result and set remaining to
                // toEnd and the last param to t.
                BezCoord<Flt> rtn (true);
                rtn.setRemaining (toEnd);
                rtn.setParam (t);
                return rtn;
            }

            // On every call, compute a threshold. lthresh is a percentage, so compute
            // the absolute threshold, lt as a percentage of l.
            Flt lt = this->lthresh * static_cast<Flt>(0.01) * l;

            // Do a binary search to find the value of dt which gives a b2 that is l
            // further on
            BezCoord<Flt> b2 (true);
            bool finished = false;
            while (!finished && ((t+dt) <= static_cast<Flt>(1.0))) {

                // Compute position of candidate point dt beyond t in param space
                b2 = this->computePoint (t+dt);
                Flt dl = b1.distanceTo (b2);
                if (abs(l-dl) < lt) {
                    // Stop here.
                    finished = true;
                } else {
                    if (dl > l) {
                        dtmax = dt;
                    } else { // dl < l
                        dtmin = dt;
                    }
                    dt = dtmin + (dtmax-dtmin)/static_cast<Flt>(2.0);
                }
            }

            if (!finished) {
                // Return a null coordinate
                BezCoord<Flt> rtn (true);
                return rtn;
            }

            return b2;
        }

        /*!
         * Like computePointsBySearch, but instead of using the Euclidean distance,
         * space points with x between them in the first coordinate - the horizonal
         * coordinate.
         */
        BezCoord<Flt> computePointBySearchHorz (Flt t, Flt x) const {
            // Min and max of possible range for dt to make a step of length l in posn space
            Flt dtmin = static_cast<Flt>(0.0);
            Flt dtmax = static_cast<Flt>(1.0) - t;

            // First guess for dt. Arb. units in parameter space.
            Flt dt = dtmin + (dtmax-dtmin)/static_cast<Flt>(2.0);

            BezCoord<Flt> b1 = this->computePoint (t);

            // Find distance from the initial position to the end of the curve. If this
            // is a shorter distance than l, then return.
            BezCoord<Flt> e1 = this->computePoint (static_cast<Flt>(1.0));
            Flt toEnd = b1.horzDistanceTo (e1);
            if (toEnd < x) {
                // Return null coordinate as the result and set remaining to toEnd and
                // the last param to t.
                BezCoord<Flt> rtn (true);
                rtn.setRemaining (toEnd);
                rtn.setParam (t);
                return rtn;
            }

            // How close we need to be to the target x for a given choice of dt.
            Flt lt = this->lthresh * static_cast<Flt>(0.01) * x;

            // Do a binary search to find the value of dt
            BezCoord<Flt> b2 (true);
            bool finished = false;
            Flt lastdt = static_cast<Flt>(0.0);
            while (!finished && ((t+dt) <= static_cast<Flt>(1.0)) && lastdt != dt) {

                // Compute position of candidate point dt beyound t in param space
                b2 = this->computePoint (t+dt);
                Flt dx = b1.horzDistanceTo (b2);
                //cout << "t+dt= " << t+dt << ", dx = " << dx << endl;
                if (abs(x-dx) < lt) {
                    // Stop here.
                    finished = true;
                } else {
                    if (dx > x) {
                        dtmax = dt;
                    } else { // dl < l
                        dtmin = dt;
                    }
                    lastdt = dt;
                    dt = dtmin + (dtmax-dtmin)/static_cast<Flt>(2.0);
                }
            }

            if (!finished) {
                // Return a null coordinate
                BezCoord<Flt> rtn (true);
                return rtn;
            }

            return b2;
        }

        /*!
         * Test that t is in range [0,1]. Throw exception otherwise.
         */
        void checkt (Flt t) const {
            if (t < static_cast<Flt>(0.0) || t > static_cast<Flt>(1.0)) {
                throw std::runtime_error ("t out of range [0,1]");
            }
        }

    private: // attributes
        /*!
         * A scaling factor to convert from the SVG drawing units into mm (or
         * whatever). This is used when computing the BezCoords to output.
         */
        Flt scale = static_cast<Flt>(1.0);

        /*!
         * How close we need to be to the target l for a given choice of dt. arb. units
         * in position space (not parameter space).  This is used in computeBySearch and
         * computeBySearchHorz.
         *
         * Should be set as an acceptable percentage error in the target l. So, 1.0
         * would mean that the threshold for finding a suitable dt to advance a distance
         * l along the curve would be l/100 * 1.0.
         */
        Flt lthresh = static_cast<Flt>(1.0);

        /*!
         * The as-the-crow-flies distance from p0 to p1. Use for for BEZLINEAR to avoid
         * repeat computations. See, especially, computePointLinear (Flt t, Flt l) const
         */
        Flt linlength = static_cast<Flt>(0.0);

        /*!
         * Scaled version of linlength
         */
        Flt linlengthscaled = static_cast<Flt>(0.0);

        /*!
         * The order of the Bezier curve. The value of the highest power of t. Thus 3 is
         * a cubic Bezier, 2 is a quadratic Bezier, etc. Note that 0th order Bezier
         * curve does not exist; so the constructor must update this number.
         */
        unsigned int order = 0;

        /*!
         * Matrix representation
         */
        //@{

        /*!
         * Set up M and MC. Called from constructors.  A description of how to write
         * out the matrix comes from Cohen & Riesenfeld (1982) General Matrix
         * Representations...
         */
        void matrixSetup (void) {

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
            int mp = m+1; // order+1
            int r = 0;
            this->M.set_size (mp, mp);
            this->M.zeros();
            for (int i = 0; i < mp; ++i) { // i is column
                for (r = 0; r < mp-i; ++r) { // r is row
                    Flt element = static_cast<Flt>(BezCurve::binomial_lookup (m, i))
                        * static_cast<Flt>(BezCurve::binomial_lookup (m-i, m-i-r))
                        * pow (-static_cast<Flt>(1.0), static_cast<Flt>(m-i-r));
                    // Ensure the matrix is inverted 'm-i', not just 'i'
                    this->M(m-i, r) = element;
                }
            }

            // Compute M * C
            this->MC = this->M * this->C;
        }

        //! The coefficients.
        arma::Mat<Flt> M;

        //! The control points.
        arma::Mat<Flt> C;

        //! M*C
        arma::Mat<Flt> MC;
        //@}
    };

} // namespace morph

#endif // _BEZCURVE_H_
