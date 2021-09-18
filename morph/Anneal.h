/*
 * Simulated Annealing. Usage similar to NM_Simplex. Client code should create an
 * instance of the anneal class, then repeatedly call its public methods until the
 * objects state member is Anneal_State::ReadyToStop. Computation of whatever the
 * objective function is is left entirely to the client code. What the client code
 * should do next is stored in Anneal::state.
 *
 * Author: Seb James
 * Date: September 2021
 */
#pragma once

#include <utility>
#include <vector>
#include <iostream>
#include <morph/MathAlgo.h>
#include <morph/vVector.h>

namespace morph {

    //! What state is an instance of the Anneal class in?
    enum class Anneal_State
    {
        // The state is unknown
        Unknown,
        // Need to do something
        NeedToComputeCandidate,
        // The algorithm has finished and found a location within tolerance
        ReadyToStop
    };

    /*!
     * A class implementing the simulated annealing optimization process. The number of
     * parameters, n, is set at runtime, by design.
     */
    template <typename T>
    class Anneal
    {
    public:
        //! The number of dimensions in the parameter search space.
        unsigned int n = 2;

        //! Do we *descend* to the *minimum* metric value/fitness/objective function value? By
        //! default we DO. Set this to false to instead ascend to the maximum metric value.
        bool downhill = true;

        //! Increment every time the algorithm performs an operation of some sort. For
        //! this NM algorithm, I increment every time the simplex changes shape.
        unsigned long long int operation_count = 0;

        //! If set >0, then if operation_count exceeds too_many_operations, then
        //! ReadyToStop is set (and a warning emitted).
        unsigned long long int too_many_operations = 0;

        // Annealing variables here

        //! The temperature
        T T;

        //! Current paramters
        morph::vVector<T> current;
        //! Value of objective function for current parameters
        T current_value;

        //! Best parameters
        morph::vVector<T> best;
        //! Value of obj fn for best parameters
        T best_value;

        //! Candidate parameter values
        morph::vVector<T> cand;
        //! Value of obj fn for candidate parameters
        T cand_value;

        //! The state tells client code what it needs to do next.
        Anneal_State state = Anneal_State::Unknown;

    public:
        // Constructors

        //! Default constructor
        Anneal() { this->allocate(); }
        //! General constructor for n dimensions with initial values
        Anneal (const morph::vVector<T>& initial_values)
        {
            this->n = initial_values.size();
            this->allocate();
            unsigned int i = 0;
            for (morph::vVector<T>& v : this->vertices) {
                v = initial_vertices[i++];
            }
            this->state = Anneal_State::StartingState;
        }

        //! Return the parameters of the best approximation, given the values of the vertices.
        morph::vVector<T> best_approximation() { return this->best_param; }
        //! Return the value of the best approximation, given the values of the vertices.
        T best_value() { return this->best_param_value; }

        void func()
        {
            T w = std::exp(-(fxcand - fxcur)/this->T);
            T s = this->rng.get();
            if (w>s) {
                this->state = Anneal::accept_candidate;
            }
        }

    private:
        //! Resize the various vectors based on the value of n.
        void allocate()
        {
            this->vertices.resize (this->n+1);
            for (morph::vVector<T>& v : this->vertices) { v.resize (this->n, 0.0); }
            this->x0.resize (this->n, 0.0);
            this->xr.resize (this->n, 0.0);
            this->xe.resize (this->n, 0.0);
            this->xc.resize (this->n, 0.0);
            this->values.resize (this->n+1, 0.0);
            this->vertex_order.resize (this->n+1, 0);
            unsigned int i = 0;
            for (unsigned int& vo : this->vertex_order) { vo = i++; }
        }
    };

} // namespace morph
