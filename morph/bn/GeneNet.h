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

        //! I'd like a Genome class.
        struct Genome
        {
            // In GeneNet, a Genome is an array<genosect_t, N_Genes>
        };

        //! A Boolean gene network class
        struct GeneNet
        {
            //! Probability of flipping each bit of the genome during evolution.
            float pOn;

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

#if N_Genes == 6
            std::array<genosect_t, N_Genes> selected_genome()
            {
                array<genosect_t, N_Genes> genome = {{ 0x2a0b00c8d7cee66fULL, 0x1f27d5082715cd95ULL, 0x9e12d18b6b5add34ULL, 0x7ec6c4222c0dc635ULL, 0x3b72c42b80cf5d5cULL, 0x7221967e8c593e2dULL }};
                return genome;
            }
#elif N_Genes == 5
# if defined N_Ins_EQUALS_N_Genes
            /*!
             * A selected genome for k=n
             *
             * This is the genome given in Fig 1 of the paper for N_Genes=5 and
             * also the one we'll use in the paper as the bold evolution and for
             * which we show the state transitions.
             */
            std::array<genosect_t, N_Genes>
            selected_genome()
            {
                std::array<genosect_t, N_Genes> genome = {{ 0x8875517a, 0x5c1e87e1, 0x8eef99d4, 0x1a3c467f, 0xdf7235c6 }};
                return genome;
            }
# else
            /*!
             * A genome for k=n-1
             */
            std::array<genosect_t, N_Genes>
            selected_genome()
            {
                std::array<genosect_t, N_Genes> genome = {{ 0xa3bc, 0x927f, 0x7b84, 0xf57d, 0xecdc }};
                return genome;
            }
# endif
#endif

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

            /*!
             * Output the genome in a string containing hex representations of the
             * N_Genes genosects.
             */
            std::string genome_id (const std::array<genosect_t, N_Genes>& genome)
            {
                stringstream ss;
                ss << hex;
                bool first = true;
                for (unsigned int i = 0; i<N_Genes; ++i) {
                    if (first) {
                        first = false;
                        ss << (genome[i] & genosect_mask);
                    } else {
                        ss << "-" << (genome[i] & genosect_mask);
                    }
                }
                ss << dec;
                return ss.str();
            }

            //! A debugging aid to display the genome in a little table.
            void
            show_genome (const std::array<genosect_t, N_Genes>& genome)
            {
                std::cout << "Genome:" << std::endl;
                bool first = true;
                for (unsigned int i = 0; i<N_Genes; ++i) {
                    if (first) {
                        first = false;
                        std::cout << (char)('a'+i);
                    } else {
                        std::cout << "     " << (char)('a'+i);
                    }
                }
                std::cout << std::endl << std::hex;
                first = true;
                for (unsigned int i = 0; i<N_Genes; ++i) {
                    if (first) {
                        first = false;
                        std::cout << "0x" << genome[i];
                    } else {
                        std::cout << " 0x" << genome[i];
                    }
                }
                std::cout << std::dec << std::endl;
                std::cout << "Genome table:" << std::endl;
                std::cout << "input  output" << std::endl;
                for (unsigned int i = N_Ins; i > 0; --i) {
                    std::cout << (i-1);
                }
                std::cout << "   ";
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    std::cout << i << " ";
                }
                std::cout << "<-- for input, bit posn; for output, array index";
#if N_Genes == 5
# if defined N_Ins_EQUALS_N_Genes
                std::cout << std::endl << "----------------" << std::endl;
                std::cout << "12345   abcde <-- 1,2,3,4,5 is i ii iii iv v in Fig 1." << std::endl;
# else
                std::cout << std::endl << "-----------------" << std::endl;
                std::cout << "1234   abcde <-- 1,2,3,4 is i ii iii iv in Fig 1." << std::endl;
# endif
#else
                for (unsigned int i = 0; i<N_Ins; ++i) { std::cout << i; }
                std::cout << "  ";
                for (unsigned int i = 0; i<N_Genes; ++i) { std::cout << " " << (char)('a'+i); }
                std::cout << std::endl;
#endif
                std::cout << "----------------" << std::endl;

                for (unsigned int j = 0; j < (1 << N_Ins); ++j) {
                    std::cout << bitset<N_Ins>(j) << "   ";
                    for (unsigned int i = 0; i < N_Genes; ++i) {
                        genosect_t mask = GENOSECT_ONE << j;
                        std::cout << ((genome[i]&mask) >> j);
                    }
                    std::cout << std::endl;
                }
            }

            //! Set the genome g to zero.
            void zero_genome (std::array<genosect_t, N_Genes>& g)
            {
#pragma omp simd
                for (unsigned int i = 0; i < N_Genes; ++i) { g[i] = 0; }
            }


            //! Copy the contents of @from to @to
            void copy_genome (const std::array<genosect_t, N_Genes>& from, std::array<genosect_t, N_Genes>& to)
            {
#pragma omp simd
                for (unsigned int i = 0; i < N_Genes; ++i) { to[i] = from[i]; }
            }

            /*!
             * This one will, rather than flipping each bit with a certain
             * probability, instead flip bits_to_flip bits, selected randomly.
             */
            void evolve_genome (std::array<genosect_t, N_Genes>& genome, unsigned int bits_to_flip)
            {
#ifdef DEBUG
                unsigned int numflipped = 0;
#endif

                unsigned int genosect_w = (GENOSECT_ONE << N_Ins);
                unsigned int lgenome = N_Genes * genosect_w;

                // Init a list containing all the indices, lgenome long. As bits
                // are flipped, we'll remove from this list, ensuring that the
                // random selection of the remaining bits remains fair.
                list<unsigned int> idices;
                for (unsigned int b = 0; b < lgenome; ++b) {
                    idices.push_back (b);
                }
                for (unsigned int b = 0; b < bits_to_flip; ++b) {
                    unsigned int r = static_cast<unsigned int>(floor(randDouble() * (double)lgenome));
                    // Catch the edge case (where randDouble() returned exactly 1.0)
                    if (r == lgenome) { --r; }

                    // The bit to flip is *i, after these two lines:
                    list<unsigned int>::iterator i = idices.begin();
                    for (unsigned int rr = 0; rr < r; ++rr, ++i) {}

                    unsigned int j = *i;
                    // Find out which genome sect j is in.
                    unsigned int gi = j / genosect_w;
                    DBG ("Genome section gi= " << gi << " which is an offset of " << (gi*genosect_w));
                    genosect_t gsect = genome[gi];

                    j -= (gi*genosect_w);
#ifdef DEBUG
                    ++numflipped;
                    DBG ("Flipping bit " << j+(gi*genosect_w));
#endif
                    gsect ^= (GENOSECT_ONE << j);
                    genome[gi] = gsect;

                    --lgenome;
                }
                DBG ("Num flipped: " << numflipped);
            }

            /*!
             * The evolution function.
             */
            void evolve_genome (array<genosect_t, N_Genes>& genome)
            {
#ifdef DEBUG
                unsigned int numflipped = 0;
#endif
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    genosect_t gsect = genome[i];
                    for (unsigned int j = 0; j < (1<<N_Ins); ++j) {
                        if (randDouble() < this->pOn) {
                            // Flip bit j
#ifdef DEBUG
                            ++numflipped;
#endif
                            gsect ^= (GENOSECT_ONE << j);
                        }
                    }
                    genome[i] = gsect;
                }
                DBG ("Num flipped: " << numflipped);
            }

            //! Flip one bit in the passed in genome at index flipidx
            void bitflip_genome (std::array<genosect_t, N_Genes>& genome,
                                 unsigned int theGenosect, unsigned int extra)
            {
                genome[theGenosect] ^= (GENOSECT_ONE << extra);
            }

            /*!
             * A version of evolve_genome which adds to a count of the number of flips
             * made in each genosect. Was used for code verification.
             */
            void
            evolve_genome (std::array<genosect_t, N_Genes>& genome,
                           std::array<unsigned long long int, N_Genes>& flipcount)
            {
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    genosect_t gsect = genome[i];
                    for (unsigned int j = 0; j < (1<<N_Ins); ++j) {
                        if (randFloat() < pOn) {
                            // Flip bit j
                            ++flipcount[i];
                            gsect ^= (GENOSECT_ONE << j);
                        }
                    }
                    genome[i] = gsect;
                }
            }

            //! Populate the passed in genome with random bits.
            void random_genome (std::array<genosect_t, N_Genes>& genome)
            {
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    // Replace with use of a RandUniform<>:
                    genome[i] = ((genosect_t) SHR3((&rd))) & genosect_mask;
                }
            }

            //! Generate a random genome and return it.
            std::array<genosect_t, N_Genes> random_genome()
            {
                array<genosect_t, N_Genes> genome;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    // Replace with RandUniform<>
                    genome[i] = ((genosect_t) SHR3((&rd))) & genosect_mask;
                }

                return genome;
            }

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

            //! Compute Hamming distance between two genomes.
            unsigned int
            compute_hamming (const std::array<genosect_t, N_Genes>& g1,
                             const std::array<genosect_t, N_Genes>& g2)
            {
                unsigned int hamming = 0;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    genosect_t bits = g1[i] ^ g2[i]; // XOR
                    hamming += _mm_popcnt_u32 ((unsigned int)bits);
                }
                return hamming;
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

            /*!
             * Is the function defined by the genosect_t @gs a canalysing function?
             *
             * If not, return (unsigned int)0, otherwise return the number of bits for
             * which the function is canalysing - this may be called canalysing "depth".
             */
            unsigned int isCanalyzing (const genosect_t& gs)
            {
                std::bitset<N_Ins> acanal_set;
                std::bitset<N_Ins> acanal_unset;
                std::array<int, N_Ins> setbitivalue;
                std::array<int, N_Ins> unsetbitivalue;
                unsigned int canal = 0;

                for (unsigned int i = 0; i < N_Ins; ++i) {
                    acanal_set[i] = false;
                    acanal_unset[i] = false;
                    setbitivalue[i] = -1;
                    unsetbitivalue[i] = -1;
                }

                // Test each bit. If bit's state always leads to same result in gs, then
                // canalysing is true for that bit.
                for (unsigned int i = 0; i < N_Ins; ++i) {

                    // Test bit i. First assume it IS canalysing for this bit
                    acanal_set.set(i);
                    acanal_unset.set(i);

                    // Iterate over the rows in the truth table
                    for (unsigned int j = 0; j < (0x1 << N_Ins); ++j) {

                        // if (bit i in row j is on)
                        if ((j & (1UL<<i)) == (1UL<<i)) {
                            if (setbitivalue[i] == -1) {
                                // -1 means we haven't yet recorded an output from gs, so record it:
                                setbitivalue[i] = (int)(1UL&(gs>>j));
                            } else {
                                if (setbitivalue[i] != (int)(1UL&(gs>>j))) {
                                    // Cannot be canalysing for bit i set to 1.
                                    acanal_set.reset(i);
                                }
                            }
                        } else {
                            // (bit i in row j is off)
                            if (unsetbitivalue[i] == -1) {
                                // Haven't yet recorded an output from gs, so record it:
                                unsetbitivalue[i] = (int)(1UL&(gs>>j));
                            } else {
                                if (unsetbitivalue[i] != (int)(1UL&(gs>>j))) {
                                    // Cannot be canalysing for bit i set to 0
                                    acanal_unset.reset(i) = false;
                                }
                            }
                        }
                    }
                }

                // Count up
                for (unsigned int i = 0; i < N_Ins; ++i) {
                    if (acanal_set.test(i) == true) {
                        canal++;
                        DBG2 ("Bit " << i << "=1 produces a consistent output value");
                    }
                    if (acanal_unset.test(i) == true) {
                        canal++;
                        DBG2 ("Bit " << i << "=0 produces a consistent output value");
                    }
                }

                return canal;
            }

            /*!
             * Test each section of the genosect and determine how many of the truth
             * tables are canalysing functions. Return the number of truth tables that
             * are canalysing.
             */
            unsigned int canalyzingness (const std::array<genosect_t, N_Genes>& g1)
            {
                unsigned int canal = 0;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    DBG2 ("=== isCanalysing? genome section " << i << " ===");
                    unsigned int _canal = isCanalyzing (g1[i]);
                    DBG2 ("Section " << i << " has canalysing value: " << _canal);
                    canal += _canal;
                }
                return canal;
            }

            //! Compute the bias; the proportion of set bits in the genome.
            double bias (const std::array<genosect_t, N_Genes>& g1)
            {
                unsigned int bits = 0;
                for (unsigned int i = 0; i < N_Genes; ++i) {
                    for (unsigned int j = 0; j < (0x1 << N_Genes); ++j) {
                        bits += ((g1[i] >> j) & 0x1) ? 1 : 0;
                    }
                }
                return (double)bits/(double)(N_Genes*(1<<N_Genes));
            }
        };

    } // namespace bn
} // namespace morph
