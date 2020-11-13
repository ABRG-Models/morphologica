/*!
 * Snazzy templated Genome class derived from std::array.
 *
 * Author: Seb James
 * Date: Nov 2020
 */

#pragma once

#include <array>
#include <bitset>
#include <iostream>
#include <sstream>
#include <immintrin.h> // Using intrinsics for computing Hamming distances
#include <morph/Random.h>

namespace morph {
    namespace  bn {

        //! The state has N bits in it. Working with N <= 8, so:
        typedef unsigned char state_t;

        // In GeneNet, a Genome is an array<T, N_Genes> where the logic above determines what T should be
        template <typename T, size_t N, size_t K> struct Genome;
        template <typename T, size_t N, size_t K> std::ostream& operator<< (std::ostream&, const Genome<T, N, K>&);

        /*!
         * The Genome class is derived from an std::array.
         */
        template <typename T=unsigned int, size_t N=5, size_t K=5>
        struct Genome : public std::array<T, N> // if derived from array, can't have constructor
        {
            //! Compile-time function used to initialize lo_mask_start
            static constexpr unsigned char lo_mask_init()
            {
                // Set up globals. Set N_Ins bits to the high position for the lo_mask
                unsigned char _lo_mask_start = 0x0;
                for (unsigned int i = 0; i < K; ++i) {
                    _lo_mask_start |= 0x1 << i;
                }
                return _lo_mask_start;
            }
            //! Compile-time function used to initialize hi_mask_start
            static constexpr unsigned char hi_mask_init()
            {
                unsigned char _hi_mask_start = 0xff & (0xff << N);
                return _hi_mask_start;
            }
            //! Compile-time function used to initialize genosect_mask
            static constexpr T genosect_mask_init()
            {
                T _genosect_mask = 0x0;
                for (unsigned int i = 0; i < (1<<K); ++i) { // 1<<N is the same as 2^N
                    _genosect_mask |= (0x1 << i);
                }
                return _genosect_mask;
            }
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
#endif

            static constexpr unsigned char lo_mask_start = Genome::lo_mask_init();
            static constexpr unsigned char hi_mask_start = Genome::hi_mask_init();
            //! The mask used to get the significant bits of genome section.
            static constexpr T genosect_mask = Genome::genosect_mask_init();
            //! The mask used to get the significant bits of a state.
            //static constexpr state_t state_mask = Genome::state_mask_init();

            //! String output
            std::string str() const
            {
                std::stringstream ss;
                ss << std::hex;
                bool first = true;
                for (unsigned int i = 0; i<N; ++i) {
                    if (first) {
                        first = false;
                        ss << ((*this)[i] & this->genosect_mask);
                    } else {
                        ss << "-" << ((*this)[i] & this->genosect_mask);
                    }
                }
                ss << std::dec;
                return ss.str();
            }
            //! Alias for str
            std::string genome_id() const { return this->str(); }

            //! Compute Hamming distance between this genome and another (g2).
            unsigned int hamming (const morph::bn::Genome<T,N,K>& g2)
            {
                unsigned int hamming = 0;
                for (unsigned int i = 0; i < N; ++i) {
                    T bits = (*this)[i] ^ g2[i]; // XOR
                    hamming += _mm_popcnt_u32 ((unsigned int)bits);
                }
                return hamming;
            }
            /*!
             * Is the function defined by the T @gs a canalysing function?
             *
             * If not, return (unsigned int)0, otherwise return the number of bits for
             * which the function is canalysing - this may be called canalysing "depth".
             */
            unsigned int isCanalyzing (const T& gs)
            {
                std::bitset<K> acanal_set;
                std::bitset<K> acanal_unset;
                std::array<int, K> setbitivalue;
                std::array<int, K> unsetbitivalue;
                unsigned int canal = 0;

                for (unsigned int i = 0; i < K; ++i) {
                    acanal_set[i] = false;
                    acanal_unset[i] = false;
                    setbitivalue[i] = -1;
                    unsetbitivalue[i] = -1;
                }

                // Test each bit. If bit's state always leads to same result in gs, then
                // canalysing is true for that bit.
                for (unsigned int i = 0; i < K; ++i) {

                    // Test bit i. First assume it IS canalysing for this bit
                    acanal_set.set(i);
                    acanal_unset.set(i);

                    // Iterate over the rows in the truth table
                    for (unsigned int j = 0; j < (0x1 << K); ++j) {

                        // if (bit i in row j is on)
                        if ((j & (T{1}<<i)) == (T{1}<<i)) {
                            if (setbitivalue[i] == -1) {
                                // -1 means we haven't yet recorded an output from gs, so record it:
                                setbitivalue[i] = (int)(T{1}&(gs>>j));
                            } else {
                                if (setbitivalue[i] != (int)(T{1}&(gs>>j))) {
                                    // Cannot be canalysing for bit i set to 1.
                                    acanal_set.reset(i);
                                }
                            }
                        } else {
                            // (bit i in row j is off)
                            if (unsetbitivalue[i] == -1) {
                                // Haven't yet recorded an output from gs, so record it:
                                unsetbitivalue[i] = (int)(T{1}&(gs>>j));
                            } else {
                                if (unsetbitivalue[i] != (int)(T{1}&(gs>>j))) {
                                    // Cannot be canalysing for bit i set to 0
                                    acanal_unset.reset(i) = false;
                                }
                            }
                        }
                    }
                }

                // Count up
                for (unsigned int i = 0; i < K; ++i) {
                    if (acanal_set.test(i) == true) {
                        canal++;
                        //DBG2 ("Bit " << i << "=1 produces a consistent output value");
                    }
                    if (acanal_unset.test(i) == true) {
                        canal++;
                        //DBG2 ("Bit " << i << "=0 produces a consistent output value");
                    }
                }

                return canal;
            }

            /*!
             * Test each section of the genosect and determine how many of the truth
             * tables are canalysing functions. Return the number of truth tables that
             * are canalysing.
             */
            unsigned int canalyzingness()
            {
                unsigned int canal = 0;
                for (unsigned int i = 0; i < N; ++i) {
                    canal += this->isCanalyzing ((*this)[i]);
                }
                return canal;
            }

            //! Compute the bias; the proportion of set bits in the genome.
            double bias()
            {
                unsigned int bits = 0;
                for (unsigned int i = 0; i < N; ++i) {
                    for (unsigned int j = 0; j < (0x1 << N); ++j) {
                        bits += (((*this)[i] >> j) & 0x1) ? 1 : 0;
                    }
                }
                return (double)bits/(double)(N*(1<<N));
            }

            //! This Genome has its own random number generator
            morph::RandUniform<T> rng;
            void randomize()
            {
                for (unsigned int i = 0; i < N; ++i) {
                    (*this)[i] = this->rng.get() & genosect_mask;
                }
            }

            //! Overload the stream output operator
            friend std::ostream& operator<< <T, N, K> (std::ostream& os, const Genome<T, N, K>& v);
        };

        template <typename T=unsigned int, size_t N=5, size_t K=5>
        std::ostream& operator<< (std::ostream& os, const Genome<T, N, K>& g)
        {
            os << g.str();
            return os;
        }
    } // bn
} // morph
