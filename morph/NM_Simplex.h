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
#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <morph/MathAlgo.h>
#include <morph/vvec.h>

namespace morph {

    //! What state is an instance of the NM_Simplex class in?
    enum class NM_Simplex_State : uint32_t
    {
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

    //! For what reason did we enter NM_Simplex_State?
    enum class NM_Simplex_Stop_Reason : uint32_t
    {
        None,                 // There is currently no reason to stop
        TerminationThreshold, // Normal reason for stopping, we got to the termination_threashd
        TooManyOperations     // We exceeded too_many_operations
    };

    /*!
     * A class implementing a Nelder Mead simplex of points, and the associated methods for
     * manipulating those points on the way to discovering a minimum of a function.
     *
     * This could be re-written with template <typename T, unsigned int N> where N is the
     * dimensionality of the search, and using morph::vec<T, N+1> as the type for
     * vertices.
     */
    template <typename T>
    class NM_Simplex
    {
    public:
        // Parameters. Initialised to the standard values given on the NM Wikipedia page.
        //
        //! The reflection coefficient
        T alpha = T{1};
        //! The expansion coefficient
        T gamma = T{2};
        //! The contraction coefficient
        T rho = T{0.5};
        //! The shrink coefficient
        T sigma = T{0.5};

        //! The number of dimensions in the search. There are n+1 vertices in the simplex.
        unsigned int n = 2U;

        //! Do we *descend* to the *minimum* metric value/fitness/objective function value? By
        //! default we DO. Set this to false to instead ascend to the maximum metric value.
        bool downhill = true;

        //! Increment every time the algorithm performs an operation of some sort. For
        //! this NM algorithm, I increment every time the simplex changes shape.
        unsigned long long int operation_count = 0ULL;

        //! If set >0, then if operation_count exceeds too_many_operations, then
        //! ReadyToStop is set (and a warning emitted). Arriving at
        //! too_many_operations probably means termination_threshold was set too low.
        unsigned long long int too_many_operations = 0ULL;

        //! Client code should set the termination threshold to be suitable for the problem. When
        //! the standard deviation of the values of the objective function at the vertices of the
        //! simplex drop below this value, the algorithm will be deemed to be finished.
        T termination_threshold = T{0.0001};

        //! The centroid of all points except vertex n (the last one)
        morph::vvec<T> x0;

        //! A container to hold the reflected point xr = x0 + alpha(x0 - vertex[vertex_order.back()])
        morph::vvec<T> xr;
        //! The objective function value of the reflected point
        T xr_value;

        //! A container for the expanded point xe
        morph::vvec<T> xe;
        //! The objective function value of the expanded point
        T xe_value;

        //! A container for the contracted point xc (can probably merge with xe)
        morph::vvec<T> xc;
        //! The objective function value of the contracted point
        T xc_value;

        //! The locations of the simplex vertices. A vector of n+1 vertices, each of n coordinates.
        morph::vvec<morph::vvec<T>> vertices;

        //! The objective function value for each vertex.
        morph::vvec<T> values;

        //! This vector contains the size order of the vector values and can be used to index into
        //! vertices and values in the order of the metric. The first index in this vector indexes
        //! the "best" value in values/vertices. If downhill==true, then the first index indexes the
        //! lowest value in values, otherwise it indexes the highest value in values.
        morph::vvec<unsigned int> vertex_order;

        //! This tells client code what it needs to do next. It either needs to order the points or
        //! compute a new objective function value for the reflected point xr;
        NM_Simplex_State state = NM_Simplex_State::Unknown;

        //! Store the reason the algorithm entered the state NM_Simple_State::ReadyToStop
        NM_Simplex_Stop_Reason stopreason = NM_Simplex_Stop_Reason::None;

    public:
        // Constructors

        //! Default constructor
        NM_Simplex() { this->allocate(); }
        //! General constructor for n+1 vertices in n dimensions. The inner vector
        //! should be of size n, the outer vector of size n+1. Thus, for a simplex
        //! triangle flipping on a 2D surface, you'd have 3 vertices with 2 coordinates
        //! each.
        NM_Simplex (const morph::vvec<morph::vvec<T>>& initial_vertices)
        {
            // dimensionality, n, is the number of simplex vertices minus one
            // if (initial_vertices.size() < 2) { /* Error! */ }
            this->n = initial_vertices.size() - 1;
            this->allocate();
            unsigned int i = 0U;
            for (morph::vvec<T>& v : this->vertices) {
                v = initial_vertices[i++];
            }
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }
        //! Special constructor for 2 vertices in 1 dimension
        NM_Simplex (const T& v0, const T& v1)
        {
            this->n = 1;
            this->allocate();
            this->vertices[0][0] = v0;
            this->vertices[1][0] = v1;
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }
        //! Special constructor for 3 vertices in 2 dimensions
        NM_Simplex (const morph::vec<T, 2>& v0,
                    const morph::vec<T, 2>& v1, const morph::vec<T, 2>& v2)
        {
            this->n = 2;
            this->allocate();
            this->vertices[0][0] = v0[0];
            this->vertices[0][1] = v0[1];
            this->vertices[1][0] = v1[0];
            this->vertices[1][1] = v1[1];
            this->vertices[2][0] = v2[0];
            this->vertices[2][1] = v2[1];
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }
        //! General constructor for n dimensional simplex
        NM_Simplex (const unsigned int _n): n(_n) { this->allocate(); }

        //! Reset the algorithm ready to go again
        void reset (const morph::vvec<morph::vvec<T>>& initial_vertices)
        {
            this->stopreason = NM_Simplex_Stop_Reason::None;
            this->operation_count = 0ULL;
            this->n = initial_vertices.size() - 1;
            this->allocate();
            unsigned int i = 0U;
            for (morph::vvec<T>& v : this->vertices) { v = initial_vertices[i++]; }
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }

        //! Return the location of the best approximation, given the values of the vertices.
        morph::vvec<T> best_vertex() { return this->vertices[this->vertex_order[0]]; }
        //! Return the value of the best approximation, given the values of the vertices.
        T best_value() { return this->values[this->vertex_order[0]]; }

        //! Order the vertices.
        void order()
        {
            // Order the vertices so that the first vertex is the best and the last one is the worst
            if (this->downhill) {
                // Best is lowest
                MathAlgo::bubble_sort_lo_to_hi<T> (this->values, this->vertex_order);
            } else {
                MathAlgo::bubble_sort_hi_to_lo<T> (this->values, this->vertex_order);
            }

            // if ready to stop, set state and return (we order before testing if we stop, as the
            // returning of the best value relies on the vertices being ordered).
            T sd = MathAlgo::compute_sd<T> (this->values);
            if (sd < this->termination_threshold) {
                this->state = NM_Simplex_State::ReadyToStop;
                this->stopreason = NM_Simplex_Stop_Reason::TerminationThreshold;
                return;
            } else if (this->too_many_operations > 0ULL
                       && this->operation_count > this->too_many_operations) {
                // If this is emitted, check your termination_threshold
                std::cerr << "Warning (NM_Simplex): Reached too_many_operations. "
                          << "Setting state 'ReadyToStop'. Check termination_threshold, which was: "
                          << this->termination_threshold << ". SD of simplex vertices was "
                          << sd <<" (i.e. >=termination_threshold)." << std::endl;
                this->state = NM_Simplex_State::ReadyToStop;
                this->stopreason = NM_Simplex_Stop_Reason::TooManyOperations;
                return;
            }

            this->compute_x0();
            this->reflect();
        }

    private:
        //! Find the reflected point, xr, which is the reflection of the worst point about the
        //! centroid of the simplex.
        void reflect()
        {
            this->operation_count++;
            unsigned int worst = this->vertex_order[this->n];
            this->xr = this->x0 + (this->x0 - this->vertices[worst]) * this->alpha;
            this->state = NM_Simplex_State::NeedToComputeReflection;
        }

    public:
        //! With the objective function value for the reflected point xr passed in, apply the
        //! reflection and decide whether to replace, expand or contract.
        void apply_reflection (const T _xr_value)
        {
            this->xr_value = _xr_value;

            if (this->downhill
                && this->xr_value < this->values[vertex_order[n-1]]
                && this->xr_value >= this->values[vertex_order[0]]) {
                // reflected is better (<) than 2nd worst but not better than the best, so replace
                // the worst point in the simplex with the relected point.
                this->values[vertex_order[n]] = this->xr_value;
                this->vertices[vertex_order[n]] = this->xr;
                this->state = NM_Simplex_State::NeedToOrder;

            } else if (this->downhill && this->xr_value < this->values[vertex_order[0]]) {
                // reflected is better (<) than best point so far; expand the reflected point to try
                // to get an EVEN better result
                this->expand();

            } else if (this->downhill == false
                       && this->xr_value > this->values[vertex_order[n-1]]
                       && this->xr_value <= this->values[vertex_order[0]]) {
                // reflected is better (>) than 2nd worst but not better than the best, so replace
                // the worst point in the simplex with the relected point.
                this->values[vertex_order[n]] = this->xr_value;
                this->vertices[vertex_order[n]] = this->xr;
                this->state = NM_Simplex_State::NeedToOrder;

            } else if (this->downhill == false && this->xr_value > this->values[vertex_order[0]]) {
                // reflected is better (>) than best point so far; expand
                this->expand();

            } else {
                // reflected is worse than (or equal to) the 2nd worst, so contract the worst point
                // towards the centroid
                this->contract();
            }
        }

    private:
        //! Compute the expanded point and then set the state to tell the client code that it needs
        //! to compute the objective function for the expanded point.
        void expand()
        {
            this->operation_count++;
            this->xe = this->x0 + (this->xr - this->x0) * this->gamma;
            this->state = NM_Simplex_State::NeedToComputeExpansion;
        }

    public:
        //! After computing the objective function for the expanded point, client code needs to call
        //! this function.
        void apply_expansion (const T _xe_value)
        {
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
        void contract()
        {
            this->operation_count++;
            unsigned int worst = this->vertex_order[this->n];
            this->xc = this->x0 + (this->vertices[worst] - this->x0) * this->rho;
            this->state = NM_Simplex_State::NeedToComputeContraction;
        }

    public:
        void apply_contraction (const T _xc_value)
        {
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
        void shrink()
        {
            this->operation_count++;
            for (unsigned int i = 1; i <= this->n; ++i) {
                this->vertices[i] = this->vertices[0] + (this->vertices[i] - this->vertices[0]) * this->sigma;
            }
            this->state = NM_Simplex_State::NeedToComputeThenOrder;
        }

        //! Compute x0, the centroid of all points except vertex n, or, put another way, the
        //! centroid of the best side.
        void compute_x0()
        {
            this->x0.zero();
            // For each simplex vertex except the worst vertex
            for (unsigned int i = 0; i<this->n; ++i) { // *excluding* i==n
                this->x0 += this->vertices[this->vertex_order[i]];
            }
            this->x0 /= static_cast<T>(this->n);
        }

        //! Resize the various vectors based on the value of n.
        void allocate()
        {
            this->vertices.resize (this->n+1);
            for (morph::vvec<T>& v : this->vertices) { v.resize (this->n, T{0}); }
            this->x0.resize (this->n, T{0});
            this->xr.resize (this->n, T{0});
            this->xe.resize (this->n, T{0});
            this->xc.resize (this->n, T{0});
            this->values.resize (this->n+1, T{0});
            this->vertex_order.resize (this->n+1, 0U);
            unsigned int i = 0;
            for (unsigned int& vo : this->vertex_order) { vo = i++; }
        }
    };

} // namespace morph
