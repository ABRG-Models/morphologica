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
#include <cstddef>
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
        template <std::size_t N=5, std::size_t K=N>
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

        //! The state has N bits in it. Working with N <= 8 so an unsigned char is
        //! sufficient. In this code, the MSB of state is what I call Gene a.
        typedef unsigned char state_t;

        //! A Boolean gene network class
        template <std::size_t N=5, std::size_t K=5>
        struct GeneNet
        {
            using genosect_t = typename Genosect<K>::type;

            //! Our state's MSB is 1<<N
            static constexpr state_t state_msb = (0x1<<N);

            //! When working with states in a graph of nodes, it may be necessary to use
            //! one bit to refer to the state as being unset; this is the bit to use.
            static constexpr state_t state_t_unset = 0x80;

            //! Probability of flipping each bit of the genome during evolution.
            //float p;

            //! The current state of this gene network. But but... what if I want a 2
            //! context, or 3 context of 1000 context system? Probably want to collect
            //! the states together, but keep this GeneNet class for the
            //! develop/analysis methods.
            //state_t state;

            //! Initialize lo_mask_start. E.g. for N=5 and K=4, this will have the value
            //! 00001111b. These are the bits to take as input.
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

            //! Compile-time function used to initialize state_mask. For N=5, this is 00011111b
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

            //! If true, would randomly wire inputs in setup_inputs()
            static constexpr bool random_wiring = false;

            //! Show debugging for the inputs?
            static constexpr bool debug_inputs = false;

            /*!
             * The common code for develop and develop_async sets up the N inputs. The
             * way in which the inputs are set up will uniquely identify one GeneNet
             * instance from another, even for the same N, K. That can be true even if
             * N==K, though in that case however you wire the inputs, you get
             * effectively the same network; you'll just have different values in the
             * Genome for your solutions.
             */
            static void setup_inputs (state_t& state, std::array<state_t, N>& inputs)
            {
                // A randomized gene network wiring scheme has not yet been written:
                static_assert (random_wiring == false);

                // This is the systematic wiring scheme
                if constexpr (K == N) {
                    // Effectively, there's only one possible wiring diagram; the Grand
                    // Ensemble, for which inputs==state for each of the N genes. This
                    // conforms to the wiring shown in Fig. 1 of "Limit cycle
                    // dynamics..."
                    for (unsigned int i = 0; i < N; ++i) {
                        // For each gene, this line effectively 'rotates the state
                        // around' the correct number of places in the state; hence the
                        // left-shifted part ORed with the right-shifted part.
                        inputs[i] = ((state << i) & state_mask) | (state >> (N-i));
                        if constexpr (debug_inputs == true) {
                            std::cout << " * For Gene " << i << "/" << (char)('a'+i)
                                      << " the input is: " << (GeneNet<N,K>::input_str(inputs[i])) << std::endl;
                        }
                    }
                } else { // Have slightly more computations for K != N, right?
                    // This _should_ cover the copying of inputs for any K < N.
                    for (unsigned int i = 0; i < N; ++i) {
                        // For K=N-1; for gene a, ignore input from a, for gene b ignore input from b, etc
                        // For K=N-2; for gene a, ignore input from a,b, for b, ignore b,c, etc
                        inputs[i] = ((state << i) | (state >> (N-i))) & lo_mask_start;
                        if constexpr (debug_inputs == true) {
                            std::cout << " * For Gene " << i << "/" << (char)('a'+i)
                                      << " the input is: " << (GeneNet<N,K>::input_str(inputs[i])) << std::endl;
                        }
                    }
                    if constexpr (debug_inputs == true) {
                        std::cout << " * For Gene 0/a the input (in table form) is:\n"
                                  << (GeneNet<N,K>::input_table(inputs[0])) << std::endl;
                    }
                }
            }

            //! Show debugging for the develop method?
            static constexpr bool debug_develop = false;

            //! Given a genome, develop this->state.
            static void develop (state_t& state, const Genome<N, K>& genome)
            {
                std::array<state_t, N> inputs;
                GeneNet<N,K>::setup_inputs (state, inputs);

                // Now reset state and compute new values:
                state = 0x0;

                // State a anterior is genome[inps[0]] etc
                for (unsigned int i = 0; i < N; ++i) {
                    genosect_t gs = genome[i];
                    if constexpr (debug_develop == true) {
                        std::cout << "Setting state for gene " << i
                                  << ", with genome section " << i << " which is "
                                  << std::hex << static_cast<unsigned long long int>(gs)
                                  << std::dec << " out of " << genome
                                  << "\ninputs["<<i<<"] is " << GeneNet<N,K>::input_str(inputs[i])
                                  << "\nMoving " << (unsigned int)inputs[i]
                                  << " rows down the gene " << i << " col of the i/o table" << std::endl;
                    }
                    // This line is 'move (inputs[i]) rows down the gene i column of the
                    // input-output table and read the bit'
                    genosect_t inpit = (genosect_t{1} << inputs[i]);
                    state_t num = ((gs & inpit) ? 0x1 : 0x0);
                    if constexpr (debug_develop == true) {
                        std::cout << "leftshift of bit is " << N << "-" << i << "-1=" << (N-i-1)
                                  << " and the bit " << (num?"is":"isn't") << " set" << std::endl;
                    }
                    if (num) {
                        // Then set the relevant bit in state
                        state |= (0x1 << (N-i-1));
                    } else {
                        // Unset the relevant bit in state
                        state &= ~(0x1 << (N-i-1));
                    }
                }
            }

            //! Choose one gene out of N to update at random. Can't be static, uses RNG.
            void develop_async (const Genome<N, K>& genome, state_t& state)
            {
                std::array<state_t, N> inputs;
                GeneNet<N,K>::setup_inputs (inputs);
                // NB: For async, don't reset state.

                // For one gene only
                unsigned int i = this->rng.get();
                genosect_t gs = genome[i];
                genosect_t inpit = (genosect_t{1} << inputs[i]);
                state_t num = ((gs & inpit) ? 0x1 : 0x0);
                if (num) {
                    state |= (0x1 << (N-i-1));
                } else {
                    state &= ~(0x1 << (N-i-1));
                }
            }

            //! Generate string representation of an input, showing the input bits that
            //! are ignored with 'X'
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

            //! Generate a string representation of the input in a table to be
            //! unambiguous about the input bit position names
            static std::string input_table (const state_t& _input)
            {
                std::stringstream ss;
                // Count up from 0 to N, to output a,b,c, etc
                for (unsigned int i = 0; i < N; i++) {
                    ss << (i+1) << " ";
                }
                ss << '\n';
                // Then show the bits
                ss << GeneNet<N,K>::input_str (_input);
                return ss.str();
            }

            /*!
             * Generate a string representation of the state. Something like "1 0 1 1 1"
             * in order MSB to LSB. Thus the value 0x2 in _state gives the string "0 0 0
             * 1 0", which means that Gene 'd' is expressing.
             */
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

            static constexpr bool debug_tables = false;
            static std::string gene_tables (const Genome<N, K>& genome)
            {
                std::stringstream ss;

                // Draw the N Gene tables out - for each gene, what is my output for a
                // given set of gene states?

                std::array<state_t, N> inputs;

                // Gene table headings, all in a row
                for (unsigned int i = 0; i<N; ++i) {
                    ss << "Gene " << (char)('a'+i) << " | ";
                }
                ss << "\n";
                for (unsigned int i = 0; i<N; ++i) { ss << "---------"; }
                ss << "\n";

                // Heading2
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int k = 0; k < K; ++k) {
                        ss << (char)('a'+k) << " ";
                    }
                    ss << " " << (char)('a'+i) << " | ";
                }
                ss << "\n";
                for (unsigned int i = 0; i<N; ++i) { ss << "---------"; }
                ss << "\n";

                // State/output
                for (unsigned int j = 0; j < (1 << K); ++j) {

                    for (unsigned int i = 0; i<N; ++i) {

                        genosect_t gs = genome[i];

                        state_t s = (state_t) (j & state_mask);

                        // output the state as 1010etc
                        ss << GeneNet<N,K>::state_str(s) << " ";

                        // Compute inputs for the current state
                        GeneNet<N,K>::setup_inputs (s, inputs);

                        // This line is 'move (inputs[i]) rows down the gene i column of the
                        // input-output table and read the bit'
                        genosect_t inpit = (genosect_t{1} << inputs[i]);
                        state_t num = ((gs & inpit) ? 0x1 : 0x0);

                        if constexpr (debug_tables == true) {
                            std::cout << "for state " << GeneNet<N,K>::state_str(s)
                                      << "Gene " << (char)('a'+i) << " inputs are "
                                      << GeneNet<N,K>::input_str(inputs[i])
                                      << " and the output is " << (num ? "1" : "0")
                                      << ". This bit would go in state[1<<" << N-i-1 << "]"
                                      << std::endl;
                        }

                        ss << (num ? "1" : "0") << " | ";
                    }
                    ss << std::endl;
                }

                return ss.str();
            }

            //! A setter for the state of this GeneNet.
            //void set (const state_t& st) { this->state = st; }

            //! Accept a string of 1s and 0s and set the state. Characters other than 1 and
            //! 0 are ignored. First 1/0 in the string is the MSB.
            static state_t set (const std::string& statestr)
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
                state_t state = 0;
                for (unsigned int i = 0; i < N; ++i) {
                    unsigned int j = N-i-1;
                    if (sstr[i] == '1') {
                        state |= (0x1UL << j);
                    }
                }
                return state;
            }

            //! Computes the Hamming distance between this->state and target.
            static state_t hamming (state_t state, state_t target)
            {
                // For this very short type, a lookup is probably faster than this intrinsic:
                state_t bits = state ^ target;
                unsigned int hamming = _mm_popcnt_u32 ((unsigned int)bits);
                return static_cast<state_t>(hamming);
            }

        };

    } // namespace bn
} // namespace morph
