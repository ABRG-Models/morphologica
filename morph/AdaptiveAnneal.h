/*
 * Simulated Annealing - The Adaptive Annealing Algo.
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
#include <morph/Anneal.h> // For morph::Anneal_State

namespace morph {

    /*!
     * A class implementing the Adaptive Simlulated Annealing Algorithm
     */
    template <typename T>
    class AdaptiveAnneal
    {
    public:
        //! The number of dimensions in the parameter search space. Set by constructor.
        unsigned int D = 0;

        //! Do we *descend* to the *minimum* metric value/fitness/objective function value? By
        //! default we DO. Set this to false to instead ascend to the maximum metric value.
        bool downhill = true;

        //! k is the symbol Lester uses for the step count
        unsigned long long int k = 1;

        //! How many annealing steps to make as a maximum? Set to exp(n)
        unsigned long long int k_f = 1000;

        //! The temperatures
        morph::vVector<T> temp;
        //! Initial temperatures
        morph::vVector<T> temp_0;
        //! Final temperatures
        morph::vVector<T> temp_f;

        //! Lester's Temperature_Ratio_Scale (same name in the ASA C
        //! code). m=-log(temperature_ratio_scale). This is the first parameter to tune
        //! and pay attention to.
        T temperature_ratio_scale = T{1e-5};
        //! Internal ASA parameter, m=-log(temperature_ratio_scale)
        morph::vVector<T> m;

        //! Lester's Temperature_Anneal_Scale in ASA C code. n=log(temperature_anneal_scale)
        T temperature_anneal_scale = T{100};
        //! Internal ASA parameter, n=log(temperature_anneal_scale)
        morph::vVector<T> n;

        //! Internal control parameter, c = m * exp(-n/D)
        morph::vVector<T> c;

        //! Lester's Cost_Parameter_Scale_Ratio (used to compute temp_cost)
        T cost_parameter_scale_ratio = T{1};
        morph::vVector<T> c_cost;
        morph::vVector<T> temp_cost_0;
        //! Temperature used in the acceptance function. k_cost is the number of
        //! accepted points, num_accepted
        morph::vVector<T> temp_cost;

        // Statistical records
        //! Number of candidates that are improved (descents, if downhill is true)
        unsigned long long int num_improved = 0;
        //! Number of candidates that are worse (if downhill is true)
        unsigned long long int num_worse = 0;
        //! Record statistics on the number of acceptances of worse candidates
        unsigned long long int num_worse_accepted = 0;
        //! Number of accepted parameter sets
        unsigned long long int num_accepted = 0;

        //! All accepted paramters? All parameters computed?
        morph::vVector<morph::vVector<T>> param_hist;
        //! For each entry in param_hist, record also its objective function value
        morph::vVector<T> f_param_hist;

        //! Parameter ranges - defining a part of R^n to search - [Ai, Bi]
        morph::vVector<T> range_min; // A
        morph::vVector<T> range_max; // B
        morph::vVector<T> rdelta;    // = range_max - range_min;
        morph::vVector<T> rmeans;    // = (range_max + range_min)/T{2};

        //! Initial parameters. Saved as prelim() needs to return to these.
        morph::vVector<T> x_init;

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

        //! Reannealing sensitivies
        morph::vVector<T> s;
        morph::vVector<T> s_max;
        morph::vVector<T> partials;

        //! The state tells client code what it needs to do next.
        Anneal_State state = Anneal_State::Unknown;

        //! General constructor for n dimensions with initial params
        AdaptiveAnneal (const morph::vVector<T>& initial_params,
                        const morph::vVector<morph::Vector<T,2>>& param_ranges, const bool dh = true)
        {
            this->D = initial_params.size();
            this->range_min.resize (this->D);
            this->range_max.resize (this->D);
            unsigned int i = 0;
            for (auto pr : param_ranges) {
                this->range_min[i] = pr[0];
                this->range_max[i] = pr[1];
                ++i;
            }
            this->rdelta = this->range_max - this->range_min;
            this->rmeans = (this->range_max + this->range_min)/T{2};

            this->downhill = dh;
            this->x_init = initial_params;
            this->x_cand = initial_params;
            this->x_best = initial_params;
            this->x = initial_params;

            // Before ::init() is called, user may need to manually change some
            // parameters, like temperature_ratio_scale.
            this->state = Anneal_State::NeedToInit;
        }

        //! After setting parameters, the user must call init
        void init()
        {
            // Set up the parameter/cost value members
            this->f_x_best = (this->downhill == true) ? std::numeric_limits<T>::max() : std::numeric_limits<T>::lowest();
            this->f_x = f_x_best;
            this->f_x_cand = f_x_best;
            this->x.resize (this->D, T{0});
            this->x_cand.resize (this->D, T{0});
            this->x_best.resize (this->D, T{0});

            // Initial and current temperatures
            this->temp_0.resize (this->D, T{1});
            this->temp.resize (this->D, T{1});

            // Sensitivies containers
            this->s.resize (this->D, T{1});
            this->s_max.resize (this->D, T{1});
            this->partials.resize (this->D, T{1});

            // The m and n parameters
            this->m.resize (this->D);
            this->m.set_from (-std::log(this->temperature_ratio_scale));

            this->n.resize (this->D);
            this->n.set_from (std::log(this->temperature_anneal_scale));

            // Work out expected final temp
            this->temp_f = this->temp_0 * (-this->m).exp();

            this->k_f = static_cast<unsigned long long int>(std::exp (this->n.mean()));

            // Set the 'control parameter', c, from n and m
            this->c.resize (this->D, T{1});
            this->c = this->m * (-this->n/this->D).exp();

            this->c_cost = this->c * cost_parameter_scale_ratio;
            this->temp_cost_0 = this->c_cost;
            this->temp_cost = this->c_cost;

            this->state = Anneal_State::NeedToCompute; // or NeedToStep
        }

        //! Reset the statistics on the number of objective functions accepted etc
        void reset_stats() { num_improved = 0; num_worse = 0; num_worse_accepted = 0; num_accepted = 0; }

        // Advance the simulated annealing algorithm by one step
        void step()
        {
            if (this->stop_check()) { return; }
            this->cooling_schedule();
            this->acceptance_check();
            this->generate_next();
            ++this->k;
            this->reanneal();
            this->state = Anneal_State::NeedToCompute;
        }

        void set_f_x_cand (T f_c)
        {
            this->f_x_cand = f_c;
            this->state = Anneal_State::NeedToStep;
        }

    protected:
        morph::RandUniform<T> rng_u;

        //! A function to generate a new set of parameters
        void generate_next()
        {
            bool generated = false;
            unsigned int num_attempts = 0;
            while (!generated) {
                morph::vVector<T> u(this->D);
                u.randomize();
                morph::vVector<T> u2 = ((u*T{2}) - T{1}).abs();
                morph::vVector<T> sigu = (u-T{0.5}).signum();
                morph::vVector<T> y = sigu * this->temp * ( ((T{1}/this->temp)+T{1}).pow(u2) - T{1} );
                this->x_cand = this->x + y;
                ++num_attempts;
                if (this->x_cand <= this->range_max && this->x_cand >= this->range_min) {
                    generated = true;
                } // else we'll re-generate u and y
            }
        }

        //! Carry out a reannealing. Need to study ASA code to replicate.
        void reanneal()
        {
            bool reanneal_required = false;
            if (reanneal_required) {
                this->s = -this->rdelta * this->partials; // (A-B) * dL/dalph
                std::cout << "old k: " << k << std::endl;
                this->k = static_cast<unsigned long long int>(std::pow((((temp_0/temp)*(s.max()/s)).log()/this->c).mean(), D));
                std::cout << "new k: " << k << std::endl;
            }
        }

        //! The algorithm's stopping condition
        bool stop_check()
        {
#if 0
            if ((this->temp < this->temp_f) || (this->k > this->k_f)) {
                return true;
            }
#endif
            return false;
        }

        //! The cooling schedule function
        void cooling_schedule()
        {
            this->temp = this->temp_0 * (-this->c * std::pow(this->k, T{1}/D)).exp();
            this->temp_cost = this->temp_cost_0 * (-this->c_cost * std::pow(this->num_accepted, T{1}/D)).exp();
            //std::cout << "(cooling_schedule) temp = " << temp << " temp_cost = " << temp_cost <<  std::endl;
        }

        //! The acceptance function
        bool acceptance_check()
        {
            bool candidate_is_better = false;
            if ((this->downhill == true && this->f_x_cand < this->f_x)
                || (this->downhill == false && this->f_x_cand > this->f_x)) {
                // In this case, the objective function of the candidate has improved
                candidate_is_better = true;
                ++this->num_improved;
            } else {
                ++this->num_worse;
            }

            T p = std::exp(-(this->f_x_cand - this->f_x)/(std::numeric_limits<T>::epsilon()+this->temp_cost.mean()));
            T u = this->rng_u.get();
            bool accepted = p > u ? true : false;

            if (candidate_is_better==false && accepted==true) { ++this->num_worse_accepted; }

            if (accepted) {
                std::cout << "dfx = " << (this->f_x_cand-this->f_x) << std::endl;
                std::cout << "dx = " << (this->x_cand-this->x) << std::endl;
                this->partials = (this->f_x_cand-this->f_x)/(this->x_cand-this->x);
                std::cout << "partials = " << this->partials << std::endl;
                this->x = this->x_cand;
                this->f_x = this->f_x_cand;
                this->x_best = this->f_x_cand < this->f_x_best ? this->x_cand : this->x_best;
                this->f_x_best = this->f_x_cand < this->f_x_best ? this->f_x_cand : this->f_x_best;
                this->num_accepted++;
            }

            std::cout << "Candidate is " << (candidate_is_better ? "B  ": "W/S") <<  ", p = " << p
                      << ", accepted? " << (accepted ? "Y":"N") << " k_cost=" << num_accepted << std::endl;
            return accepted;
        }

    };

} // namespace morph
