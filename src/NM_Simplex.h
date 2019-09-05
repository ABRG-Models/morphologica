/*
 * A class to implement the Nelder-Mead algorithm. Implemented following the Wikipedia page. Client
 * code should create an instance of the NM_Simplex class, then repeatedly call its public methods
 * until the objects state member is NM_Simplex_State::ReadyToStop. Computation of whatever the
 * objective function is is left entirely to the client code. What the client code should do next is
 * stored in NM_Simplex::state.
 *
 * Author: Seb James
 * Date: September 2019
 */

#ifndef _NM_SIMPLEX_H_
#define _NM_SIMPLEX_H_

#include <vector>
using std::vector;
#include "MathAlgo.h"
using morph::MathAlgo;

namespace morph {

    //! What state is an instance of the NM_Simplex class in?
    enum class NM_Simplex_State {
        // The state is unknown
        Unknown,
        // Compute all vertices, then order them
        NeedToComputeThenOrder,
        // Vertices are all computed, but need to be ordered
        NeedToOrder,
        // Need to compute the value of the reflected point, xr
        NeedToComputeReflection,
        // Need to compute the value of the expanded point, xe
        NeedToComputeExpansion,
        // Need to compute the value of the contracted point, xc
        NeedToComputeContraction,
        // The algorithm has finished and found a location within tolerance
        ReadyToStop
    };

    /*!
     * A class implementing a Nelder Mead simplex of points, and the associated methods for
     * manipulating those points on the way to discovering a minimum of a function.
     */
    template <class Flt>
    class NM_Simplex
    {
    public:
        //! Parameters. Initialised to the standard values given on the NM Wikipedia page.
        //@}
        //! The reflection coefficient
        Flt alpha = 1.0;
        //! The expansion coefficient
        Flt gamma = 2.0;
        //! The contraction coefficient
        Flt rho = 0.5;
        //! The shrink coefficient
        Flt sigma = 0.5;
        //@}

        //! The number of dimensions in the search. There are n+1 vertices in the simplex.
        unsigned int n = 2;

        //! Do we *descend* to the *minimum* metric value/fitness/objective function value? By
        //! default we DO. Set this to false to instead ascend to the maximum metric value.
        bool downhill = true;

        //! Client code should set the termination threshold to be suitable for the problem. When
        //! the standard deviation of the values of the objective function at the vertices of the
        //! simplex drop below this value, the algorithm will be deemed to be finished.
        Flt termination_threshold = 0.0001;

        //! The centroid of all points except vertex n (the last one)
        vector<Flt> x0;

        //! A container to hold the reflected point xr = x0 + alpha(x0 - vertex[vertex_order.back()])
        vector<Flt> xr;
        //! The objective function value of the reflected point
        Flt xr_value;

        //! A container for the expanded point xe
        vector<Flt> xe;
        //! The objective function value of the expanded point
        Flt xe_value;

        //! A container for the contracted point xc (can probably merge with xe)
        vector<Flt> xc;
        //! The objective function value of the contracted point
        Flt xc_value;

        //! The locations of the simplex vertices. A vector of n+1 vertices, each of n coordinates.
        vector<vector<Flt>> vertices;

        //! The objective function value for each vertex.
        vector<Flt> values;

        //! This vector contains the size order of the vector values and can be used to index into
        //! vertices and values in the order of the metric. The first index in this vector indexes
        //! the "best" value in values/vertices. If downhill==true, then the first index indexes the
        //! lowest value in values, otherwise it indexes the highest value in values.
        vector<unsigned int> vertex_order;

        //! This tells client code what it needs to do next. It either needs to order the points or
        //! compute a new objective function value for the reflected point xr;
        NM_Simplex_State state = NM_Simplex_State::Unknown;

    public:
        //! Constructors
        //@{
        //! Default constructor
        NM_Simplex (void) { this->allocate(); }
        //! General constructor for n+1 vertices in n dimensions
        NM_Simplex (const vector<vector<Flt>>& initial_vertices) {
            // dimensionality, n, is the number of simlex vertices minus one
            // if (initial_vertices.size() < 2) { /* Error! */ }
            this->n = initial_vertices.size() - 1;
            this->allocate();
            unsigned int i = 0;
            for (vector<Flt>& v : this->vertices) {
                v = initial_vertices[i++];
            }
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }
        //! General constructor for n dimensional simplex
        NM_Simplex (const unsigned int _n): n(_n) {
            // if (this->n < 1) { /* Error! */ }
            this->allocate();
        }
        //@}

        //! Order the vertices.
        void order (void) {

            // if ready to stop, set state and return (no need to order yet)
            //Flt sd = this->compute_sd_values(); // SD of this->values
            Flt sd = MathAlgo<Flt>::compute_sd (this->values);
            if (sd < this->termination_threshold) {
                this->state = NM_Simplex_State::ReadyToStop;
                return;
            }

            // Order the vertices so that the first vertex is the best and the last one is the worst
            if (this->downhill) {
                // Best is lowest
                MathAlgo<float>::bubble_sort_lo_to_hi (this->values, this->vertex_order);
            } else {
                MathAlgo<float>::bubble_sort_hi_to_lo (this->values, this->vertex_order);
            }
            cout << "After order, the order of values is " << endl;
            for (auto va : values) {
                cout << ", " << va;
            }
            cout << endl;
           cout << "...and the order of vertex_order is " << endl;
            for (auto vo : vertex_order) {
                cout << ", " << vo;
            }
            cout << endl;

            this->reflect();
        }

    private:
        //! Find the reflected point, xr.
        void reflect (void) {
            unsigned int worst = this->vertex_order[this->n];
            for (unsigned int j = 0; j < this->n; ++j) {
                this->xr[j] = this->x0[j] + this->alpha * (this->x0[j] - this->vertices[worst][j]);
                cout << "Set xr[" << j << "] to x0[j]=" << x0[j] << " + " << alpha << " * " << x0[j] << " - " << vertices[worst][j] << " (the worst vertex)" << endl;
            }
            this->state = NM_Simplex_State::NeedToComputeReflection;
        }

    public:
        //! With the objective function value for the reflected point xr passed in, apply the
        //! reflection and decide whether to replace, expand or contract.
        void apply_reflection (const Flt _xr_value) {

            this->xr_value = _xr_value;

            if (this->downhill) {
                cout << "Downhill.." << endl;
            }

            if (this->downhill
                && this->xr_value < this->values[vertex_order[n-1]]
                && this->xr_value >= this->values[vertex_order[0]]) {
                cout << "reflected (" << xr_value << ") is better (<) than 2nd worst (" << values[vertex_order[n-1]]
                     << ") but not better than the best (" << values[vertex_order[0]] << ")" << endl;
                cout << "replace worst point (" << values[vertex_order[n]]  << ") with reflected point (" << xr_value << ")" << endl;
                this->values[vertex_order[n]] = this->xr_value;
                this->vertices[vertex_order[n]] = this->xr;
                this->state = NM_Simplex_State::NeedToOrder;

                // Replace vertices[n] with xr and then next step is order().
            } else if (this->downhill
                       && this->xr_value < this->values[vertex_order[0]]) {
                cout << "reflected is better (<) than best point so far; expand" << endl;
                this->expand();

            } else if (this->downhill == false
                       && this->xr_value > this->values[vertex_order[n-1]]
                       && this->xr_value <= this->values[vertex_order[0]]) {
                // reflected is better (>) than 2nd worst but not better than the best
                this->values[vertex_order[n]] = this->xr_value;
                this->vertices[vertex_order[n]] = this->xr;
                this->state = NM_Simplex_State::NeedToOrder;

            } else if (this->downhill == false
                       && this->xr_value > this->values[vertex_order[0]]) {
                // reflected is better (>) than best point so far; expand
                this->expand();

            } else {
                // reflected is worse than (or equal to) the 2nd worst, so contract
                this->contract();
            }
        }

    private:
        //! Compute the expanded point and then set the state to tell the client code that it needs
        //! to compute the objective function for the expanded point.
        void expand (void) {
            for (unsigned int j = 0; j < this->n; ++j) {
                this->xe[j] = this->x0[j] + this->gamma * (this->xr[j] - this->x0[j]);
            }
            this->state = NM_Simplex_State::NeedToComputeExpansion;
        }

    public:
        //! After computing the objective function for the expanded point, client code needs to call
        //! this function.
        void apply_expansion (const Flt _xe_value) {

            this->xe_value = _xe_value;

            if ((this->downhill && this->xe_value < this->xr_value)
                || (this->downhill == false && this->xe_value > this->xr_value)) {
                // expanded is better
                this->values[vertex_order[this->n]] = this->xe_value;
                this->vertices[vertex_order[this->n]] = this->xe;
                this->state = NM_Simplex_State::NeedToOrder;
            } else {
                // expanded is not better, use reflected value
                this->values[vertex_order[this->n]] = this->xr_value;
                this->vertices[vertex_order[this->n]] = this->xr;
                this->state = NM_Simplex_State::NeedToOrder;
            }
        }

    private:
        void contract (void) {
            unsigned int worst = this->vertex_order[this->n];
            for (unsigned int j = 0; j < this->n; ++j) {
                this->xc[j] = this->x0[j] + this->rho * (this->vertices[worst][j] - this->x0[j]);
            }
            this->state = NM_Simplex_State::NeedToComputeContraction;
        }

    public:
        void apply_contraction (const Flt _xc_value) {
            this->xc_value = _xc_value;
            unsigned int worst = this->vertex_order[this->n];
            if ((this->downhill && this->xc_value < this->values[worst])
                || (this->downhill == false && this->xc_value > this->values[worst])) {
                // contracted is better than worst
                this->values[vertex_order[this->n]] = this->xc_value;
                this->vertices[vertex_order[this->n]] = this->xc;
                this->state = NM_Simplex_State::NeedToOrder;
            } else {
                this->shrink();
            }
        }

    private:
        void shrink (void) {
            for (unsigned int i = 1; i <= this->n; ++i) {
                for (unsigned int j = 0; j < this->n; ++j) {
                    this->vertices[i][j] = this->vertices[0][j]
                        + this->sigma * (this->vertices[i][j] - this->vertices[0][j]);
                }
            }
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }

        //! Compute x0, the centroid of all points except vertex n
        void compute_x0 (void) {
            // Zero x0
            for (Flt& x0i : this->x0) { x0i = 0.0; }
            // For each simplex vertex:
            for (unsigned int i = 0; i<=this->n; ++i) {
                for (unsigned int j = 0; j<this->n; ++j) {
                    this->x0[j] += this->vertices[i][j];
                }
            }
            for (unsigned int j = 0; j<this->n; ++j) {
                this->x0[j] /= static_cast<Flt>(this->n);
            }
        }

        //! Resize the various vectors based on the value of n.
        void allocate (void) {
            this->vertices.resize (this->n+1);
            for (vector<Flt>& v : this->vertices) {
                v.resize (this->n, 0.0);
            }
            this->x0.resize (this->n, 0.0);
            this->xr.resize (this->n, 0.0);
            this->xe.resize (this->n, 0.0);
            this->xc.resize (this->n, 0.0);
            this->values.resize (this->n+1, 0.0);
            this->vertex_order.resize (this->n+1, 0);
            unsigned int i = 0;
            for (unsigned int& vo : this->vertex_order) {
                vo = i++;
            }
        }
    };

} // namespace morph

#endif // _NM_SIMPLEX_H_
