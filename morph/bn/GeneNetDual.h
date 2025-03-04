// A dual-context GeneNet
#pragma once

#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>
#include <set>
#include <cstddef>

namespace morph {
    namespace  bn {
        template <std::size_t N=5, std::size_t K=5>
        struct GeneNetDual : public GeneNet<N, K>
        {
            //! initial_pos is always 0...
            static constexpr state_t initial_pos = 0x0;

            //! ...but depending on N, initial_ant differs
            static constexpr state_t initial_ant_init()
            {
                state_t _initial_ant = 0x0;
                if constexpr (N == 3) {
                    _initial_ant = 0x4;  // 100b;
                } else if constexpr (N == 4) {
                    _initial_ant = 0x8;  // 1000b;
                } else if constexpr (N == 5) {
                    _initial_ant = 0x10; // 10000b;
                } else if constexpr (N == 6) {
                    _initial_ant = 0x20; // 100000b;
                } else if constexpr (N == 7) {
                    _initial_ant = 0x40; // 1000000b;
                }
                return _initial_ant;
            }
            static constexpr state_t initial_ant = GeneNetDual::initial_ant_init();

            state_t state_pos = 0x0;
            state_t state_ant = 0x0;

            state_t target_pos = 0x0;
            state_t target_ant = 0x0;

            //! Output debugging info?
            static constexpr bool debug = false;

            void develop (const Genome<N,K>& genome)
            {
                GeneNet<N,K>::develop (this->state_ant, genome);
                GeneNet<N,K>::develop (this->state_pos, genome);
            }

            //! Set a special 'selected' genome which has fitness 1 according to AttractorScaffolding paper for dual context
            void set_selected (Genome<N,K>& genome)
            {
                if (this->target_pos == 0xa && this->target_ant == 0x15) {
                    if constexpr (N == 6 && K == 6) {
                        genome = {{ 0x2a0b00c8d7cee66fULL, 0x1f27d5082715cd95ULL, 0x9e12d18b6b5add34ULL, 0x7ec6c4222c0dc635ULL, 0x3b72c42b80cf5d5cULL, 0x7221967e8c593e2dULL }};
                    } else if constexpr (N == 5 && K == 5) {
                        genome = {{ 0x8875517a, 0x5c1e87e1, 0x8eef99d4, 0x1a3c467f, 0xdf7235c6 }};
                    } else if constexpr (N == 5 && K == 4) {
                        genome = {{ 0xa3bc, 0x927f, 0x7b84, 0xf57d, 0xecdc }};
                    } else {
                        throw std::runtime_error ("I have no special genome for that combination of N,K.");
                    }
                } else {
                    throw std::runtime_error ("Selected genomes work for target_pos=0xa and target_ant=0x15.");
                }
            }

            /*!
             * Evaluates the fitness of one context (anterior or posterior in the
             * 2-context system).
             */
            double evaluate_one (const Genome<N,K>& genome, state_t state, state_t target)
            {
                double score = 0.0;

                state_t state_last = GeneNet<N,K>::state_t_unset;
                std::set<state_t> visited;
                visited.insert (state); // insert starting state
                for (;;) {
                    state_last = state;
                    GeneNet<N,K>::develop (state, genome);

                    if (visited.count (state)) {

                        // Already visited this state so it's a limit cycle or point attractor

                        if (state == state_last) { // Point attractor

                            score = (state == target) ? 1.0 : score; // score is 0

                        } else { // Limit cycle

                            // Determine the states in the limit cycle by going around it once more.
                            std::set<state_t> lc;
                            unsigned int lc_len = 0;
                            while (lc.count (state) == 0) {
                                // Check if we have one or both target states on this limit cycle
                                lc.insert (state);
                                lc_len++;
                                // ("Limit cycle contains: " << state_str (state));
                                GeneNet<N,K>::develop (state, genome);
                            }

                            // Now have the set lc; can work out its score.

                            // For tabulating the scores
                            std::array<double, N> sc;
                            for (unsigned int j = 0; j < N; ++j) { sc[j] = 0.0; }

                            std::set<state_t>::const_iterator i = lc.begin();
                            while (i != lc.end()) {
                                state_t a = ((*i) ^ ~target) & GeneNet<N,K>::state_mask;
                                for (unsigned int j = 0; j < N; ++j) {
                                    sc[j] += static_cast<double>( (a >> j) & 0x1 );
                                }
                                ++i;
                            }

                            double expnt = N * -1.0;
                            score = std::pow(static_cast<double>(lc_len), expnt);
                            for (unsigned int j = 0; j < N; ++j) {
                                score *= sc[j];
                            }
                        }
                        break;
                    }
                    visited.insert (state);
                }

                return score;
            }

            /*!
             * For the passed-in genome, find its final state, starting from the
             * anterior state initial_ant and the posterior state initial_pos (stored in
             * global variables). Return a fitness specifier for the genome.
             *
             * This function examines the limit cycle that is arrived at from the two
             * initial states. The mean value of each bit in the limit cycle is compared
             * with the target state.
             *
             * The fitness is then computed according to:
             *
             * f = (a0 * a1 * a2 * a3 * a4) * (p0 * p1 * p2 * p3 * p4)
             *
             * a0 is the proportion of time during the limit cycle that bit 0 has the
             * state matching the anterior target
             *
             * Returns fitness in range 0 to 1.0. Note use of double. The fitness values
             * can potentially be very small for a long limit cycle. For example, for a
             * 5 gene LC of size 10, the fitness could be as low as (1/10)^5 * (1/10)^5
             * = 1/10^10, which is heading towards what a single precision float can
             * represent.
             *
             * For further details on this fitness evaluation, see the Nature Scientific
             * Reports paper "Limit cycle dynamics can guide the evolution of gene
             * regulatory networks towards point attractors" (2019)
             */
            double evaluate_fitness (const Genome<N,K>& genome)
            {
                if constexpr (debug == true) {
                    std::cout << "target_ant = " << static_cast<unsigned int>(target_ant) << std::endl;
                    std::cout << "target_pos = " << static_cast<unsigned int>(target_pos) << std::endl;
                }
                double ant_score = this->evaluate_one (genome, initial_ant, this->target_ant);
                double pos_score = this->evaluate_one (genome, initial_pos, this->target_pos);
                if constexpr (debug == true) {
                    std::cout << "score ant = " << ant_score << std::endl;
                    std::cout << "score pos = " << pos_score << std::endl;
                }
                double fitness = ant_score * pos_score;
                if constexpr (debug == true) {
                    if (fitness == 1.0) {
                        std::cout << "F=1 genome found.\n";
                    }
                    std::cout << genome << ", fitness: " << fitness << std::endl;
                }
                return fitness;
            }

            //! Evolve a new genome by repeatedly mutating with bitflip probability p
            Genome<N,K> evolve_new_genome (float p)
            {
                // Holds the genome and a copy of it.
                Genome<N,K> refg;
                Genome<N,K> newg;

                refg.randomize();
                double a = this->evaluate_fitness (refg);

                unsigned int gen = 0;
                // Test fitness to determine whether we should evolve.
                while (a < 1.0) {
                    newg = refg;
                    newg.mutate (p);
                    ++gen;
                    double b = this->evaluate_fitness (newg);
                    if (a > b) {
                        // New fitness <= old fitness
                    } else {
                        // Copy new fitness to ref
                        a = b;
                        // Copy new to reference
                        refg = newg;
                    }
                }
                if constexpr (debug == true) {
                    std::cout << "It took " << gen << " generations to evolve this genome\n";
                }

                return refg;
            }

        };
    }
}
