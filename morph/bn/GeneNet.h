/*!
 * Boolean gene network class. Based on code from AttractorScaffolding (2018).
 *
 * Author: Seb James
 * Date: Nov 2020
 */

#pragma once

#include <bitset>
#include <array>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <bitset>
#include <list>
#include <math.h>
#include <immintrin.h> // Using intrinsics for computing Hamming distances
#include <morph/bn/Genome.h>

namespace morph {
    namespace  bn {

#if 0
        /*!
         * This came from basins.h. I think some features should go in GeneNet, or it
         * should derive from GeneNet.
         *
         * A class to hold information about one network and its comparison
         * with any other networks.
         */
        template <size_t N=5, size_t K=N>
        struct NetInfo
        {
            NetInfo(AllBasins<N,K>& ab_, unsigned int gen, double fitn)
            {
                this->update (ab_, gen, fitn);
            }
            void update (AllBasins<N,K>& ab_, unsigned int gen, double fitn)
            {
                this->ab = ab_;
                this->generation = gen;
                this->fitness = fitn;
            }
            //! Contains the genome, and information about the attractors in the network.
            AllBasins<N,K> ab;
            //! The evolutionary generation at which this network evolved.
            unsigned int generation;
            //! The fitness of the network
            double fitness = 0.0;
            //! How much the fitness changed since the last genome
            double deltaF = 0.0;
            //! How many transitions (in ab) have changed since the last network.
            unsigned int numChangedTransitions = 0;
        };
#endif

        //! A Boolean gene network class
        template <size_t N=5, size_t K=5>
        struct GeneNet
        {
            using genosect_t = typename Genosect<K>::type;

            //! The state has N bits in it. Working with N <= 8, so:
            using state_t = unsigned char;

            //! Probability of flipping each bit of the genome during evolution.
            float p;

            //! The current state of this gene network
            state_t state;

            /*
             * Starting values of the masks used when computing inputs from a
             * state. When computing input for a gene at position i, the hi_mask is used
             * to obtain inputs for positions > i, the low mask for position < i. The hi
             * bits are then shifted right once to make up an input containing N-1
             * bits. Must be set up using masks_init().
             */

            //! Compile-time function used to initialize lo_mask_start
            static constexpr unsigned char lo_mask_init()
            {
                // Set up globals. Set K bits to the high position for the lo_mask
                unsigned char _lo_mask_start = 0x0;
                for (unsigned int i = 0; i < K; ++i) {
                    _lo_mask_start |= 0x1 << i;
                }
                return _lo_mask_start;
            }
            static constexpr unsigned char lo_mask_start = GeneNet::lo_mask_init();

            //! Compile-time function used to initialize hi_mask_start
            static constexpr unsigned char hi_mask_init()
            {
                unsigned char _hi_mask_start = 0xff & (0xff << N);
                return _hi_mask_start;
            }
            static constexpr unsigned char hi_mask_start = GeneNet::hi_mask_init();

            //! Compile-time function used to initialize state_mask. Does this belong in GeneNet?
            static constexpr state_t state_mask_init()
            {
                state_t _state_mask = 0x0;
                for (unsigned int i = 0; i < N; ++i) {
                    _state_mask |= (0x1 << i);
                }
                return _state_mask;
            }
            //! The mask used to get the significant bits of a state.
            static constexpr state_t state_mask = GeneNet::state_mask_init();

            //! Common code for develop and develop_async
            void setup_inputs (std::array<state_t, N>& inputs)
            {
                if constexpr (N == K) {
                    for (unsigned int i = 0; i < N; ++i) {
                        inputs[i] = ((this->state << i) & state_mask) | (this->state >> (N-i));
                    }
                } else {
                    state_t lo_mask = lo_mask_start;
                    state_t hi_mask = hi_mask_start;
                    for (unsigned int i = 0; i < N; ++i) {
                        inputs[i] = (this->state & lo_mask) | ((this->state & hi_mask) >> (N-K));
                        hi_mask = (hi_mask >> 1) | 0x80;
                        lo_mask >>= 1;
                    }
                }
            }

            //! Constructor sets up rng
            GeneNet() { rng.setparams (0, N-1); }

            //! Random number generated with N outcomes
            morph::RandUniform<unsigned int> rng;

            static constexpr size_t extraoffset = (K==N?1:0);

            //! Choose one gene out of N to update at random.
            void develop_async (const Genome<N, K>& genome)
            {
                std::array<state_t, N> inputs;
                this->setup_inputs (inputs);
                //state = 0x0; // Don't reset state.

                // For one gene only
                //unsigned int i = floor(this->frng.get()*N);
                unsigned int i = this->rng.get();
                std::cout << "Setting state for gene " << i << std::endl;
                typename Genosect<K>::type gs = genome[i];
                typename Genosect<K>::type inpit = (0x1 << inputs[i]);
                state_t num = ((gs & inpit) ? 0x1 : 0x0);
                if (num) {
                    this->state |= (0x1 << (K-(i+this->extraoffset)));
                } else {
                    this->state &= ~(0x1 << (K-(i+this->extraoffset)));
                }
            }

            //! Given a genome, develope this->state.
            void develop (const Genome<N, K>& genome)
            {
                std::array<state_t, N> inputs;
                this->setup_inputs (state, inputs);

                // Now reset state and compute new values:
                this->state = 0x0;

                // State a anterior is genome[inps[0]] etc
                for (unsigned int i = 0; i < N; ++i) {
                    typename Genosect<K>::type gs = genome[i];
                    typename Genosect<K>::type inpit = (0x1 << inputs[i]);
                    state_t num = ((gs & inpit) ? 0x1 : 0x0);
                    if (num) {
                        this->state |= (0x1 << (K-(i+this->extraoffset)));
                    } else {
                        this->state &= ~(0x1 << (K-(i+this->extraoffset)));
                    }
                }
            }

            //! Generate a string representation of the state. Something like "1 0 1 1 1"
            std::string state_str (const state_t& state) const
            {
                std::stringstream ss;
                // Count down from N, to output bits in order MSB to LSB.
                for (unsigned int i = N; i > 0; --i) {
                    unsigned int j = i-1;
                    ss << ((this->state & (0x1<<j)) >> j) << " ";
                }
                return ss.str();
            }

            void set (const state_t& st) { this->state = st; }

            //! Accept a list of 1 and 0 and set the state. Characters other than 1 and
            //! 0 are ignored.
            void set (const std::string& statestr)
            {
                // Collect only 1 and 0 chars, in order
                std::string sstr("");
                for (unsigned int i = 0; i < statestr.size(); ++i) {
                    if (statestr[i] == '1' || statestr[i] == '0') {
                        sstr += statestr[i];
                    }
                }

                // Check length of string of 1s and 0s is not longer than N
                if (sstr.size() != N) {
                    std::stringstream ee;
                    ee << "Wrong number of 1s and 0s (should be " << N << ")";
                    throw std::runtime_error (ee.str());
                }

                // Build up the return state
                this->state = 0;
                for (unsigned int i = 0; i < N; ++i) {
                    unsigned int j = N-i-1;
                    if (sstr[i] == '1') {
                        this->state |= (0x1UL << j);
                    }
                }
            }

            //! Computes the Hamming distance between this->state and target.
            state_t hamming (state_t target)
            {
                // For this very short type, a lookup is probably faster than this intrinsic:
                state_t bits = this->state ^ target;
                unsigned int hamming = _mm_popcnt_u32 ((unsigned int)bits);
                return static_cast<state_t>(hamming);
            }

        };

    } // namespace bn
} // namespace morph
