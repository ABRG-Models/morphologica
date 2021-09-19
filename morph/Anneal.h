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
#include <morph/Random.h>

namespace morph {

    //! What state is an instance of the Anneal class in?
    enum class Anneal_State
    {
        // The state is unknown
        Unknown,
        // Need to do something
        NeedToCompute,
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

        //! How many annealing steps to make as we go from T=1 to T=0
        unsigned long long int num_operations = 0;

        //! The temperature
        T temp = T{1};

        //! Random number generator
        morph::RandUniform* rng;

        //! Best parameters
        morph::vVector<T> best;
        //! Value of obj fn for best parameters
        T best_value = T{0};

        //! Candidate parameter values
        morph::vVector<T> cand;
        //! Value of obj fn for candidate parameters
        T cand_value = T{0};

        //! The state tells client code what it needs to do next.
        Anneal_State state = Anneal_State::Unknown;

    public:
        //! Default constructor
        Anneal() { this->allocate(); }
        //! General constructor for n dimensions with initial params
        Anneal (const morph::vVector<T>& initial_params)
        {
            this->n = initial_params.size();
            this->allocate();
            this->cand = initial_params;
            this->state = Anneal_State::NeedToCompute;
        }

        // Advance the simulated annealing algorithm by one step
        void step()
        {
            if (this->temp == T{0}) {
                this->state = Anneal_State::ReadyToStop;
                return;
            }
            // Evaluate candidate; if it's the best, then update best
            if (this->downhill == true) {
                this->best = this->cand_value < this->best_value ? this->cand : this->best;
                this->best_value = this->cand_value < this->best_value ? this->cand_value : this->best_value;
            } else {
                this->best = this->cand_value > this->best_value ? this->cand : this->best;
                this->best_value = this->cand_value > this->best_value ? this->cand_value : this->best_value;
            }
            // Decrement temperature
            this->temp = T{1} - (++operation_count) / static_cast<T>(num_operations);
            // WRITEME: Choose new candidate parameters
            // Tell client code it needs to compute the objective
            this->state = Anneal_State::NeedToCompute;
        }

    private:
        //! Resize the various vectors based on the value of n.
        void allocate()
        {
            this->cand.resize (this->n, 0.0);
            this->best.resize (this->n, 0.0);
        }
    };

} // namespace morph
