/*
 * Simulated Annealing (or quenching). Usage similar to NM_Simplex. Client code should
 * create an instance of the anneal class, then repeatedly call its public methods until
 * the objects state member is Anneal_State::ReadyToStop. Computation of whatever the
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
#include <morph/Vector.h>
#include <morph/Random.h>

namespace morph {

    //! What state is an instance of the Anneal class in?
    enum class Anneal_State
    {
        // The state is unknown
        Unknown,
        // Need to call the init() function to setup paramters etc
        NeedToInit,
        // Need to perform a step of the annealing algo
        NeedToStep,
        // Need to do compute the objective of the candidate
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

        //! The temperature or control parameter
        T temp = T{1};

        //! Number of candidates that are improved (descents, if downhill is true)
        unsigned long long int num_improved = 0;
        //! Number of candidates that are worse (if downhill is true)
        unsigned long long int num_worse = 0;
        //! Record statistics on the number of acceptances of worse candidates
        unsigned long long int num_worse_accepted = 0;

        //! Random number generator (uniform, range 0-1)
        morph::RandUniform<T> rnd_u;

        //! Parameter ranges - defining a part of R^n to search
        morph::vVector<morph::Vector<T,2>> ranges;

        // Multiplier on the range.
        T range_mult = T{1};

        //! A vector of random number generators.
        morph::vVector<morph::RandNormal<T>*> generators;

        //! Best parameters so far
        morph::vVector<T> x_best;
        //! Value of obj fn for best parameters
        T f_x_best = T{0};

        //! Candidate parameter values
        morph::vVector<T> x_cand;
        //! Value of obj fn for candidate parameters
        T f_x_cand = T{0};

        //! Current parameters
        morph::vVector<T> x;
        //! Value of obj fn for current parameters
        T f_x = T{0};

        //! The state tells client code what it needs to do next.
        Anneal_State state = Anneal_State::Unknown;

        //! General constructor for n dimensions with initial params
        Anneal (const morph::vVector<T>& initial_params,
                const morph::vVector<morph::Vector<T,2>>& param_ranges, const bool dh = true)
        {
            this->n = initial_params.size();
            this->ranges = param_ranges;
            this->downhill = dh;
            this->init();
            this->x_cand = initial_params;
            this->x_best = initial_params;
            this->x = initial_params;
            this->state = Anneal_State::NeedToCompute;
        }

        //! Deconstructor cleans up the RandNormal generators
        ~Anneal() { for (auto& g : this->generators) { delete g; } }

        //! Reset the statistics on the number of objective functions accepted etc
        void reset_stats() { num_improved = 0; num_worse = 0; num_worse_accepted = 0; }

        //! The cooling schedule function
        T U() { return (T{1} - (++operation_count) / static_cast<T>(num_operations)); }

        // Advance the simulated annealing algorithm by one step
        void step()
        {
            if (this->temp <= T{0}) {
                this->state = Anneal_State::ReadyToStop;
                return;
            }

            // Evaluate candidate; if it's the best, then update best
            if (this->downhill == true) {
                this->x_best = this->f_x_cand < this->f_x_best ? this->x_cand : this->x_best;
                this->f_x_best = this->f_x_cand < this->f_x_best ? this->f_x_cand : this->f_x_best;
            } else {
                this->x_best = this->f_x_cand > this->f_x_best ? this->x_cand : this->x_best;
                this->f_x_best = this->f_x_cand > this->f_x_best ? this->f_x_cand : this->f_x_best;
            }

            // set temperature
            this->temp = this->U();

            // Do we accept candidate?
            if (this->accept()) {
                this->x = this->x_cand;
                this->f_x = this->f_x_cand;
            }

            // Choose new candidate parameters
            this->generate_candidate();

            // Tell client code it needs to compute the objective for the new candidate
            this->state = Anneal_State::NeedToCompute;
        }

        void set_f_x_cand (T f_c)
        {
            this->f_x_cand = f_c;
            this->state = Anneal_State::NeedToStep;
        }

    protected:

        //! The neighbour or candidate generating function
        virtual void generate_candidate()
        {
            morph::vVector<T> delta(this->x_cand.size());
            for (unsigned int i = 0; i < delta.size(); ++i) { delta[i] = (this->generators[i])->get(); }
            this->x_cand = this->x + delta * this->range_mult;
            // Ensure we don't exceed the ranges
            for (unsigned int i = 0; i < this->x_cand.size(); ++i) {
                x_cand[i] = x_cand[i] > this->ranges[i][1] ? this->ranges[i][1] : x_cand[i];
                x_cand[i] = x_cand[i] < this->ranges[i][0] ? this->ranges[i][0] : x_cand[i];
            }
        }

        //! The acceptance function (Metropolis et al style)
        virtual bool accept()
        {
            bool rtn = false;
            if (this->f_x_cand < this->f_x) {
                ++this->num_improved;
                rtn = true;
            } else {
                ++this->num_worse;
                T p = std::exp (-(this->f_x_cand - this->f_x)/this->temp);
                rtn = p > this->rnd_u.get() ? ++this->num_worse_accepted, true : false;
            }
            return rtn;
        }

        //! Resize vectors, allocate RNGs
        void init()
        {
            this->f_x_best = (this->downhill == true) ? std::numeric_limits<T>::max() : std::numeric_limits<T>::lowest();
            this->f_x = f_x_best;
            this->f_x_cand = f_x_best;
            this->x.resize (this->n, 0.0);
            this->x_cand.resize (this->n, 0.0);
            this->x_best.resize (this->n, 0.0);
            this->generators.resize (this->n);
            unsigned int i = 0;
            for (auto r : this->ranges) {
                // Range is r[0] to r[1]
                T sd = std::sqrt(r[1]-r[0]); // Fix this
                this->generators[i++] = new morph::RandNormal<T>(T{0}, sd);
            }
        }
    };

} // namespace morph
