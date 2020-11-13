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

namespace morph {
    namespace  bn {

//! N_Genes is the number of genes. Better as constexpr probably.
#ifndef N_Genes
# error 'Number of genes parameter N_Genes must be defined'
#endif


/*!
 * Here you can set (in the notation used by Stuart Kaufmann) whether
 * k=n or k=n-1. We investigated both, but the paper covers the case
 * where k=n. In the code, N_Ins is equivalent to Kaufmann's 'k';
 * N_Genes is his 'n'.
 */
#ifndef k_equals_n_minus_1
# define N_Ins_EQUALS_N_Genes 1
#endif

#ifdef N_Ins_EQUALS_N_Genes       // k = n
# define ExtraOffset  (1)
# define N_Ins        (N_Genes)
#else                             // k = n-1
# define ExtraOffset  (0)
# define N_Ins        (N_Genes-1)
# define N_minus_k    (1)
#endif

/*!
 * The genome has a section for each gene. The length of the
 * section of each gene is 2^N_Ins. 32 bit width sections are
 * enough for N_Genes <= 6. 64 bit width sections ok for N_Genes <= 7.
 */
#if N_Genes == 7 // or N_Genes == 6 and N_Ins_EQUALS_N_Genes
        typedef unsigned long long int genosect_t;
# define GENOSECT_ONE 0x1ULL
#elif N_Genes == 6
# if defined N_Ins_EQUALS_N_Genes
        typedef unsigned long long int genosect_t;
#  define GENOSECT_ONE 0x1ULL
# else
        typedef unsigned int genosect_t;
#  define GENOSECT_ONE 0x1UL
# endif
#else
        typedef unsigned int genosect_t;
# define GENOSECT_ONE 0x1UL
#endif

/*!
 * Width of the genome section. Used here to create the right width
 * bitsets for debugging.
 */
#define Genosect_Width (1 << N_Ins)

        //! The state has N_Genes bits in it. Working with N_Genes <= 8, so:
        typedef unsigned char state_t;

/*!
 * When right-shifting the hi_mask, we need to set the top bit to 1,
 * because right-shifting an unsigned integer number always zero-fills
 * by default.
 */
#define state_t_top_bit 0x80


        //! A Boolean gene network class
        struct GeneNet
        {
            //! Probability of flipping each bit of the genome during evolution.
            float pOn;

#if 0
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
            static constexpr unsigned char lo_mask_start = Genome::lo_mask_init();

            //! Compile-time function used to initialize hi_mask_start
            static constexpr unsigned char hi_mask_init()
            {
                unsigned char _hi_mask_start = 0xff & (0xff << N);
                return _hi_mask_start;
            }
            static constexpr unsigned char hi_mask_start = Genome::hi_mask_init();
#endif
            /*
             * Starting values of the masks used when computing inputs from a
             * state. When computing input for a gene at position i, the hi_mask is used
             * to obtain inputs for positions > i, the low mask for position < i. The hi
             * bits are then shifted right once to make up an input containing N_Genes-1
             * bits. Must be set up using masks_init().
             */
            unsigned char lo_mask_start;
            unsigned char hi_mask_start;

            //! The mask used to get the significant bits of genome section. Set up: masks_init().
            genosect_t genosect_mask;

            //! The mask used to get the significant bits of a state. Set up using masks_init().
            state_t state_mask;


            //! Initialise the masks based on the value of N_Genes
            void masks_init()
            {
#ifdef DEBUG2
                // 2 ^ (N_Genes - 1) == 1 << (N_Genes-1) == 1 << N_Ins
                unsigned int l_genome = N_Genes * (1 << N_Ins);
                DBG2 ("N_Genes: " << N_Genes << " N_Ins: " << N_Ins << " length of genome: " << l_genome);
#endif
                // Set up globals. Set N_Ins bits to the high position for the lo_mask
                this->lo_mask_start = 0x0;
                for (unsigned int i = 0; i < N_Ins; ++i) {
                    this->lo_mask_start |= 0x1 << i;
                }
                this->hi_mask_start = 0xff & (0xff << N_Genes);

                this->genosect_mask = 0x0;
                for (unsigned int i = 0; i < (1<<N_Ins); ++i) { // 1<<N is the same as 2^N
                    this->genosect_mask |= (0x1 << i);
                }
                DBG2 ("genosect_mask: 0x" << std::hex << this->genosect_mask << std::dec); // 65535 is 16 bits

                this->state_mask = 0x0;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    this->state_mask |= (0x1 << i);
                }
            }

            void compute_next_common (const state_t& state, std::array<state_t, N_Genes>& inputs)
            {
#ifndef N_Ins_EQUALS_N_Genes
                state_t lo_mask = lo_mask_start;
                state_t hi_mask = hi_mask_start;
#endif

                for (unsigned int i = 0; i < N_Genes; ++i) {

#ifdef N_Ins_EQUALS_N_Genes
                    inputs[i] = ((state << i) & state_mask) | (state >> (N_Genes-i));
#else
                    inputs[i] = (state & lo_mask) | ((state & hi_mask) >> N_minus_k);
                    hi_mask = (hi_mask >> 1) | 0x80;
                    lo_mask >>= 1;
#endif
                }
            }

            //! Choose one gene out of N_Genes to update at random.
            void compute_next_async (const std::array<genosect_t, N_Genes>& genome, state_t& state)
            {
                std::array<state_t, N_Genes> inputs;
                compute_next_common (state, inputs);
                //state = 0x0; // Don't reset state.

                // For one gene only
                unsigned int i = floor(randDouble()*N_Genes);
                DBG2 ("Setting state for gene " << i);
                genosect_t gs = genome[i];
                genosect_t inpit = (0x1 << inputs[i]);
                state_t num = ((gs & inpit) ? 0x1 : 0x0);
                if (num) {
                    state |= (0x1 << (N_Ins-(i+ExtraOffset)));
                } else {
                    state &= ~(0x1 << (N_Ins-(i+ExtraOffset)));
                }
            }

            /*!
             * Given a state for N_Genes, and a genome, compute the next
             * state. This is "develop" rather than "evolve".
             */
            void compute_next (const std::array<genosect_t, N_Genes>& genome, state_t& state)
            {
                std::array<state_t, N_Genes> inputs;
                compute_next_common (state, inputs);

                // Now reset state and compute new values:
                state = 0x0;

                // State a anterior is genome[inps[0]] etc
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    DBG2 ("Setting state for gene " << i);
                    genosect_t gs = genome[i];
                    genosect_t inpit = (0x1 << inputs[i]);
                    state_t num = ((gs & inpit) ? 0x1 : 0x0);
                    if (num) {
                        state |= (0x1 << (N_Ins-(i+ExtraOffset)));
                    } else {
                        state &= ~(0x1 << (N_Ins-(i+ExtraOffset)));
                    }
                }
            }

            /*!
             * Generate a string representation of the state. Something like "1 0 1
             * 1 1" or "0 0 1 1 0".
             */
            std::string state_str (const state_t& state)
            {
                std::stringstream ss;
                // Count down from N_Genes, to output bits in order MSB to LSB.
                for (unsigned int i = N_Genes; i > 0; --i) {
                    unsigned int j = i-1;
                    ss << ((state & (0x1<<j)) >> j) << " ";
                }
                return ss.str();
            }

            /*!
             * Accept a list of 1 and 0 and convert this into a state_t. Characters
             * other than 1 and 0 are ignored.
             */
            state_t str2state (const std::string& statestr)
            {
                // Collect only 1 and 0 chars, in order
                std::string sstr("");
                for (unsigned int i = 0; i < statestr.size(); ++i) {
                    if (statestr[i] == '1' || statestr[i] == '0') {
                        sstr += statestr[i];
                    }
                }

                // Check length of string of 1s and 0s is not longer than N_Genes
                if (sstr.size() != N_Genes) {
                    std::stringstream ee;
                    ee << "Wrong number of 1s and 0s (should be " << N_Genes << ")";
                    throw std::runtime_error (ee.str());
                }

                // Build up the return state
                state_t thestate = (state_t)0x0;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    unsigned int j = N_Genes-i-1;
                    if (sstr[i] == '1') {
                        thestate |= (0x1UL << j);
                    }
                }

                return thestate;
            }

            //! Output a text representation of the state to stdout.
            void show_state (const state_t& state)
            {
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    std::cout << (char)((char)'a'+(char)i) << " ";
                }
                std::cout << std::endl << state_str (state) << std::endl;
            }

            /*!
             * Produce the string of 1 and 0 chars to match the format Dan provides
             * genomes in.
             */
            std::string genome2str (std::array<genosect_t, N_Genes>& genome)
            {
                std::stringstream rtn;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    for (unsigned long long int j = 0; j < (1 << N_Ins); ++j) {
                        genosect_t mask = GENOSECT_ONE << j;
                        rtn << ((genome[i]&mask) >> j);
                    }
                }
                return rtn.str();
            }

            /*!
             * Take a string of 1s and 0s and convert this into an array<genosect_t,
             * N_Genes>.
             */
            std::array<genosect_t, N_Genes> str2genome (const std::string& s)
            {
                std::array<genosect_t, N_Genes> g;
                // Init g
                for (unsigned int i = 0; i < N_Genes; ++i) { g[i] = 0x0; }

                // Check length of string.
                unsigned int l = s.length();
                unsigned int l_genosect = GENOSECT_ONE << N_Ins;
                unsigned int l_genome = N_Genes * l_genosect;
                if (l != l_genome) {
                    return g;
                } // else ("String has " << l_genome << " bit chars as required...");

                for (unsigned int i = 0; i < N_Genes; ++i) {
                    for (unsigned int j = 0; j < l_genosect; ++j) {
                        bool high = (s[j + i*l_genosect] == '1');
                        if (high) {
                            g[i] |= GENOSECT_ONE << j;
                        } // else do nothing.
                    }
                }

                return g;
            }

            /*!
             * Convert a vector of booleans (Stuart's format for the genome) into
             * an array of genosect_ts.
             */
            std::array<genosect_t, N_Genes> vecbool2genome (const std::vector<bool>& vb)
            {
                std::array<genosect_t, N_Genes> g;
                // Init g
                for (unsigned int i = 0; i < N_Genes; ++i) { g[i] = 0x0; }

                // Check length of vector of bools
                unsigned int l = vb.size();
                unsigned int l_genosect = GENOSECT_ONE << N_Ins;
                unsigned int l_genome = N_Genes * l_genosect;
                if (l == l_genome) {
                    DBG ("Vector has " << l_genome << " bit bools as required...");
                } else {
                    return g;
                }

                for (unsigned int i = 0; i < N_Genes; ++i) {
                    for (unsigned int j = 0; j < l_genosect; ++j) {
                        if (vb[j + i*l_genosect]) {
                            g[i] |= GENOSECT_ONE << j;
                        } // else do nothing.
                    }
                }
                return g;
            }

            //! The state has N bits in it. Working with N <= 8, so:
            typedef unsigned char state_t;

#if 0
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
            static constexpr state_t state_mask = Genome::state_mask_init();
#endif

            /*!
             * Computes the Hamming distance between state and target. This is
             * equal to the number of bit positions at which state and target
             * differ.
             */
            state_t compute_hamming (state_t state, state_t target)
            {
                // For this very short type, a lookup is probably faster than this intrinsic:
                state_t bits = state ^ target;
                unsigned int hamming = _mm_popcnt_u32 ((unsigned int)bits);
                return static_cast<state_t>(hamming);
            }


            /*!
             * This function obtained from
             * https://compprog.wordpress.com/2007/10/17/generating-combinations-1/
             *
             * next_combination (int comb[], int k, int n)
             *  Generates the next combination of n elements as k after comb
             *
             *  I.e. produces n choose k combinations.
             *
             *  comb => the previous combination ( use (0, 1, 2, ..., k) for first)
             *  k => the size of the subsets to generate
             *  n => the size of the original set
             *
             *  Returns: 1 if a valid combination was found
             *           0, otherwise
             */
            static int next_combination (int comb[], int k, int n)
            {
                int i = k - 1;
                ++comb[i];
                while ((i >= 0) && (comb[i] >= n - k + 1 + i)) {
                    --i;
                    ++comb[i];
                }

                if (comb[0] > n - k) { /* Combination (n-k, n-k+1, ..., n) reached */
                    /* Peculiar bug occurs here on returning when called within combos.cpp */
                    return 0; /* No more combinations can be generated */
                }

                /* comb now looks like (..., x, n, n, n, ..., n).
                   Turn it into (..., x, x + 1, x + 2, ...) */
                for (i = i + 1; i < k; ++i) {
                    comb[i] = comb[i - 1] + 1;
                }

                return 1;
            }

            //! Prints out a combination like {1, 2} for debugging
            static void printc (int comb[], int k)
            {
                printf ("{");
                int i;
                for (i = 0; i < k; ++i) { printf("%d, ", comb[i] + 1); }
                printf ("\b\b}\n");
            }

            //! Returns a string of len 1s and 0s in the unsigned int ui.
            std::string uint_str (const unsigned int& ui, int len)
            {
                std::stringstream ss;
                // Count down from N_Genes, to output bits in order MSB to LSB.
                for (unsigned int i = len; i > 0; --i) {
                    unsigned int j = i-1;
                    ss << ((ui & (0x1<<j)) >> j);
                }
                return ss.str();
            }

            /*!
             * Prints out a combination in binary format, like: {001, 010}
             * etc. Could count zeros in cols and print out here.
             */
            static void printc_binary (int comb[], int k, int w)
            {
                for (int i = 0; i < k; ++i) {
                    std::cout << uint_str (comb[i], w) << "\n";
                }
                std::cout << "---\n";
            }

            /*!
             * This counts the zeros in the column(s) indicated by show_zeros_mask
             * and returns 1 if there were all zeros on those column(s)
             *
             * comb The array of possible combinations generated by calls to
             * next_combination
             *
             * l is the number of states considered - this is the limit cycle
             * length in my application.
             *
             * ng bit width of the numbers - this is number of genes.
             *
             * show_zeros_mask - a bit mask for the cols for which we should count
             * zero columns.
             *
             * m is the column number for which I want to count the possible
             * combinations for which a preceding zero column would make the score
             * zero.
             *
             * prec_zcs is incremented if the state set under consideration has
             * zero columns in any column up to col m-1.
             */
            int
            printc_binary (int comb[], int l, int ng,
                           int show_zeros_mask, float& score, int m, int& prec_zcs, int& zcs)
            {
                int rtn = 0;
                int zeros = 0;
                float cols[ng];
                for (unsigned int c = 0; c < (unsigned int)ng; ++c) {
                    cols[c] = 0;
                }

                for (int s = 0; s < l; ++s) {
                    for (unsigned int c = 0; c < (unsigned int)ng; ++c) {
                        cols[c] += (comb[s] & (1<<c)) >> c;
                    }
                    if (show_zeros_mask
                        && ((comb[s] ^ show_zeros_mask) & show_zeros_mask) == show_zeros_mask) {
                        zeros++;
                    }
                    std::cout << uint_str (comb[s], ng) << "\n";
                }

                if (zeros == l) {
                    std::cout << "ZCol," << uint_str(show_zeros_mask, ng) << ",";
                    for (int s = 0; s < l; ++s) {
                        for (unsigned int c = 0; c < (unsigned int)ng; ++c) {
                            std::cout << ((comb[s] & (1<<c)) >> c);
                        }
                        std::cout << ",";
                    }
                    std::cout << std::endl;
                    rtn = 1;
                }

                // Record scores for each column. Increment a count if we had zeros
                // in any column up to m.
                score = 1.0f;
                bool incremented_prec_zcs = false;
                for (unsigned int c = 0; c < (unsigned int)ng; ++c) {
                    cols[c] = cols[c]/(float)l;
                    //std::cout << "score = " << score << " * " << cols[c] << std::endl;
                    score = score * cols[c];
                    if (c < (unsigned int)m && cols[c] == 0.0f && !incremented_prec_zcs) {
                        prec_zcs++;
                        incremented_prec_zcs = true;
                    }
                    if (c == (unsigned int)m && cols[c] == 0.0f && !incremented_prec_zcs) {
                        zcs++;
                    }
                }
                std::cout << "score: " << score << std::endl;
                std::cout << "---\n";
                return rtn;
            }

        };

    } // namespace bn
} // namespace morph
