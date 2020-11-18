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
            static constexpr state_t state_msb = 0x80;

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

            //! Compile-time function used to initialize lo_mask_start. E.g. for N=5 and
            //! K=4, this will have the value 00001111b. These are the bits to take as
            //! input.
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

            //! Compile-time function used to initialize hi_mask_start. E.g. for N=5 and
            //! K=4, this will have the value 11100000b
            static constexpr unsigned char hi_mask_init()
            {
                unsigned char _hi_mask_start = 0xff & (0xff << N);
                return _hi_mask_start;
            }
            static constexpr unsigned char hi_mask_start = GeneNet::hi_mask_init();

            //! Compile-time function used to initialize state_mask. For N=5, this is
            //! 00011111b
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

            //! Constructor sets up rng
            GeneNet() { rng.setparams (0, N-1); }

            //! Random number generated with N outcomes
            morph::RandUniform<unsigned int> rng;

            //! If compiled with this set true, then rather than using a sequential
            //! scheme to decide which inputs should be ignored by each of the N genes,
            //! these should be chosen randomly.
            static constexpr bool random_wiring = false;

            //! Common code for develop and develop_async. The way in which the inputs
            //! are set up will uniquely identify one GeneNet instance from another,
            //! even for the same N, K. That can be true even if N==K, though in that
            //! case however you wire the inputs, you get effectively the same network;
            //! you'll just have different values in the Genome for your solutions.
            void setup_inputs (std::array<state_t, N>& inputs)
            {
                if constexpr (K == N) {
                    // Effectively, there's only one possible wiring diagram; the Grand
                    // Ensemble, for which inputs==state for each of the N genes. This
                    // conforms to the wiring shown in Fig. 1 of "Limit cycle
                    // dynamics..."
                    for (unsigned int i = 0; i < N; ++i) {
                        // For each gene, this line effectively 'rotates the state
                        // around' the correct number of places in the state; hence the
                        // left-shifted part ORed with the right-shifted part.
                        inputs[i] = ((this->state << i) & state_mask) | (this->state >> (N-i));
                        std::cout << " * For Gene " << i << " the input is: " << (this->input_str(inputs[i])) << std::endl;
                    }
                } else { // Have slightly more computations for K != N, right?
                    // This _should_ cover the copying of inputs for any K < N.
                    state_t lo_mask = lo_mask_start;
                    state_t hi_mask = hi_mask_start;
                    //std::cout << "Current state is " << this->state_str(this->state) << std::endl;
                    for (unsigned int i = 0; i < N; ++i) {
                        // Thinkme: Hmm. Here, why does the right shift not cycle with i?? That's a mistake.
                        // For K=N-1; for gene a, ignore input from a, for gene b ignore input from b, etc
                        inputs[i] = (this->state & lo_mask) | ((this->state & hi_mask) >> (N-K));
                        std::cout << " * For Gene " << i << " the input is: " << (this->input_str(inputs[i])) << std::endl;
                        hi_mask = (hi_mask >> 1) | state_msb;
                        lo_mask >>= 1;
                    }
                }
            }

            //! Given a genome, develop this->state.
            void develop (const Genome<N, K>& genome)
            {
                std::array<state_t, N> inputs;
                this->setup_inputs (inputs);

                // Now reset state and compute new values:
                this->state = 0x0;

                // State a anterior is genome[inps[0]] etc
                for (unsigned int i = 0; i < N; ++i) {
                    genosect_t gs = genome[i];
                    // std::cout << "Setting state for gene " << i
                    //           << ", with genome section " << i << " which is "
                    //           << std::hex << gs << std::dec << " out of " << genome << std::endl;
                    // This line is 'move (inputs[i]) rows down the gene i column of the input-output table and read the bit'
                    // std::cout << "inputs["<<i<<"] is " << this->input_str(inputs[i]) << std::endl;
                    // std::cout << "Moving " << (unsigned int)inputs[i] << " rows down the gene " << i << " col of the i/o table\n";
                    genosect_t inpit = (genosect_t{1} << inputs[i]);
                    state_t num = ((gs & inpit) ? 0x1 : 0x0);
                    // unsigned int leftshift = N-i-1;
                    // std::cout << "leftshift of bit/unbit is " << N << "-" << i << "-1=" << leftshift << std::endl;
                    if (num) {
                        // Then set the relevant bit in state
                        this->state |= (0x1 << (N-i-1));
                    } else {
                        // Unset the relevant bit in state
                        this->state &= ~(0x1 << (N-i-1));
                    }
                }
            }

            //! Choose one gene out of N to update at random.
            void develop_async (const Genome<N, K>& genome)
            {
                std::array<state_t, N> inputs;
                this->setup_inputs (inputs);
                // NB: For async, don't reset state.

                // For one gene only
                unsigned int i = this->rng.get();
                genosect_t gs = genome[i];
                genosect_t inpit = (genosect_t{1} << inputs[i]);
                state_t num = ((gs & inpit) ? 0x1 : 0x0);
                if (num) {
                    this->state |= (0x1 << (N-i-1));
                } else {
                    this->state &= ~(0x1 << (N-i-1));
                }
            }

            //! Generate string representation of an input, showing the input bits that are ignored with 'X'
            static std::string input_str (const state_t& _input)
            {
                std::stringstream ss;
                // Count down from N, to output bits in order MSB to LSB.
                for (unsigned int i = N; i > 0; --i) {
                    if (i > K) {
                        // Ignored input bit
                        ss << "X ";
                    } else {
                        unsigned int j = i-1;
                        ss << ((_input & (0x1<<j)) >> j) << " ";
                    }
                }
                return ss.str();
            }

            //! Generate a string representation of the state. Something like "1 0 1 1
            //! 1" in order MSB to LSB. Thus the value 0x2 in _state gives the string "0
            //! 0 0 1 0", which means that Gene 'd' is expressing.
            static std::string state_str (const state_t& _state)
            {
                std::stringstream ss;
                // Count down from N, to output bits in order MSB to LSB. MSB is Gene a,
                // next is b and (if N==5) LSB is e.
                for (unsigned int i = N; i > 0; --i) {
                    unsigned int j = i-1;
                    ss << ((_state & (0x1<<j)) >> j) << " ";
                }
                return ss.str();
            }

            //! Generate a string representation of the state in a table to be
            //! unambiguous about the gene names
            static std::string state_table (const state_t& _state)
            {
                std::stringstream ss;
                // Count up from 0 to N, to output a,b,c, etc
                for (unsigned int i = 0; i < N; i++) {
                    ss << (char)('a'+i) << " ";
                }
                ss << '\n';
                // Then call state_str
                ss << GeneNet<N,K>::state_str (_state);
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
