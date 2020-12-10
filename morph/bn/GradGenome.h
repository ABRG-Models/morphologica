/*!
 * A gradient-climbing genome, with bits for the inter-gene interactions (but NOT self-self
 * interactions) of 'climbs gradient of gene j' and 'descents gradient of gene j'.
 *
 * Author: Seb James
 * Date: Dec 2020
 */

#pragma once

#include <array>
#include <list>
#include <bitset>
#include <iostream>
#include <sstream>
#include <immintrin.h>
#include <morph/tools.h>
#include <morph/bn/Random.h>
#include <morph/bn/GradGenosect.h>

namespace morph {
    namespace  bn {

        // A Genome is (derived from) an array<Genosect<K>::type, N>.
        template <size_t N> struct GradGenome;
        template <size_t N> std::ostream& operator<< (std::ostream&, const GradGenome<N>&);

        /*!
         * The GradGenome class
         *
         * A GradGenome isn't like the Genome, in that it doesn't have an associated
         * GeneNet. It incorporates its own method for client code to query if gene i
         * climbs gradient of gene j etc.
         *
         * Bit arrangement:
         *
         * A is in (*this)[0]| B (*this)[1]      | C (*this)[2]
         * -----------------------------------------------------------------------
         * Au Ad Bu Bd Cu Cd | Au Ad Bu Bd Cu Cd | Au Ad Bu Bd Cu Cd |
         * -----------------------------------------------------------------------
         * MSB           LSB | MSB           LSB |
         *
         * If the MSB of (*this)[1] is set, then B climbs UP the gradient of A (unless
         * the next-most significant bit is ALSO set, in which case it neither climbs
         * nor descends the gradient of A)
         *
         * \tparam T The width of each 'genosect'. This should be an unsigned integral
         * type; unsigned char, unsigned int or unsigned long long int.
         *
         * \tparam N The number of genes in the network. Each gene interacts with the
         * other N genes so there are 2*N*N bits total in the GradGenome.
         */
        template <size_t N=5>
        struct GradGenome : public std::array<typename GradGenosect<N>::type, N>
        {
            using genosect_t = typename GradGenosect<N>::type;

            //! initialize GradGenome<>::genosect_mask
            static constexpr genosect_t genosect_mask_init()
            {
                genosect_t _genosect_mask = 0x0;
                for (unsigned int i = 0; i < (2*N); ++i) {
                    _genosect_mask |= (genosect_t{1} << i);
                }
                return _genosect_mask;
            }
            static constexpr genosect_t genosect_mask = GradGenome<N>::genosect_mask_init();

            //! Each genosect can be up to 64 bits, so no more than 32 genes
            static constexpr bool checkTemplateParams()
            {
                static_assert(N < 32);
                return N<32 ? true : false;
            }

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

            std::string table() const
            {
                std::stringstream ss;
                ss << "Gradient climb/descend genome table:\n";
                bool first = true;
                // 'i' header line
                for (unsigned int i = 0; i<N; ++i) {
                    if (first) {
                        first = false;
                        for (unsigned int j = 0; j<2*N; ++j) { ss << (char)('a'+i); }
                    } else {
                        ss << " ";
                        for (unsigned int j = 0; j<2*N; ++j) { ss << (char)('a'+i); }
                    }
                }
                ss << std::endl;

                // climb/descend bit indicator line
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int j = 0; j<N; ++j) {
                        ss << "CD";
                    }
                    ss << " ";
                }
                ss << std::endl;

                // 'j' header line
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int j = 0; j<N; ++j) {
                        ss << (char)('a'+j) << (char)('a'+j);
                    }
                    ss << " ";
                }
                ss << std::endl;

                // for each i, output the genosect
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int j = 0; j<(2*N); ++j) {
                        ss << (((*this)[i] >> ((2*N)-j-1)) & 0x1);
                    }
                    ss << " ";
                    // which gives climb/descend info
                }
                ss << std::endl;

                ss << "Gradient climb/descend short table:\n";
                first = true;
                // 'i' header line
                for (unsigned int i = 0; i<N; ++i) {
                    if (first) {
                        first = false;
                        for (unsigned int j = 0; j<N; ++j) { ss << (char)('a'+i); }
                    } else {
                        ss << " ";
                        for (unsigned int j = 0; j<N; ++j) { ss << (char)('a'+i); }
                    }
                }
                ss << std::endl;

                // climb/descend
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int j = 0; j<N; ++j) {
                        if (i_climbs_j(i,j)) {
                            ss << "C";
                        } else {
                            if (i_descends_j(i,j)) {
                                ss << "D";
                            } else {
                                ss << "-";
                            }
                        }
                    }
                    ss << " ";
                }
                ss << std::endl;

                // 'j' header line
                for (unsigned int i = 0; i<N; ++i) {
                    for (unsigned int j = 0; j<N; ++j) {
                        ss << (char)('a'+j);
                    }
                    ss << " ";
                }
                ss << std::endl;


                return ss.str();
            }

            //! Set the genome from the hex string format generated by str()
            void set (const std::string& hexstr)
            {
                // 1 split string by the '-' character
                std::vector<std::string> parts = morph::Tools::stringToVector (hexstr, "-");

                // 2 Make sure there are N sections
                if (parts.size() != N) {
                    throw std::runtime_error ("Can't set this genome from that string, wrong number of genome sections");
                }

                // 3 Convert each as hex into genosect_t things
                size_t i = 0;
                for (auto p : parts) {
                    (*this)[i] = std::stoi (p, 0, 16) & this->genosect_mask;
                    i++;
                }
            }

            //! Set the genome to zero.
            void zero() { for (unsigned int i = 0; i < N; ++i) { (*this)[i] = 0; } }

            //! Genome width
            static constexpr size_t width = 2 * N * N;

            //! Mutate this genome with bit flip probability p
            void mutate (const float& p)
            {
                // Number of frng calls is N * 2^K (160 for N=5,K=5). That's a lot of
                // randomness for each bit.
                Random<N,N>* prng = Random<N,N>::i();
                prng->fill_grad_rnums();
                typename std::array<float, width>::iterator riter = prng->grad_rnums.begin();
                for (unsigned int i = 0; i < N; ++i) {
                    genosect_t gsect = (*this)[i];
                    for (unsigned int j = 0; j < (2*N); ++j) {
                        if (*riter++ < p) {
                            // Flip bit j
                            gsect ^= (genosect_t{1} << j);
                        }
                    }
                    (*this)[i] = gsect;
                }
            }

            void randomize()
            {
                for (unsigned int i = 0; i < N; ++i) {
                    (*this)[i] = Random<N,N>::i()->genosect_rng.get() & genosect_mask;
                }
            }

            static constexpr bool debug_logic = false;

            //! Return true if gene_i climbs the gradient of gene_j
            bool i_climbs_j (size_t gene_i, size_t gene_j) const
            {
                if constexpr (debug_logic) {
                    std::cout << "i="<<gene_i<<" climbs j="<<gene_j<<"?" << std::endl
                              << "*this[i] = " << (unsigned int)(*this)[gene_i] << std::endl
                              << "0x3 << ((N-gene_j-1)*2) = " << (unsigned int)(0x3 << ((N-gene_j-1)*2)) << std::endl
                              << "(*this)[gene_i] & 0x3 << (N-gene_j-1*2) = " << (unsigned int) ((*this)[gene_i] & (0x3 << ((N-gene_j-1)*2))) << std::endl;
                }
                // The shift for Gene j within the genosect for Gene i.
                size_t _shift = (N-gene_j-1) * 2;
                return ((((*this)[gene_i] & (0x3 << _shift)) >> _shift) == 0x2) ? true : false;
            }

            //! Return true if gene_i descends the gradient of gene_j
            bool i_descends_j (size_t gene_i, size_t gene_j) const
            {
                size_t _shift = (N-gene_j-1) * 2;
                return ((((*this)[gene_i] & (0x3 << _shift)) >> _shift) == 0x1) ? true : false;
            }

            //! Overload the stream output operator
            friend std::ostream& operator<< <N> (std::ostream& os, const GradGenome<N>& v);
        };

        template <size_t N=5>
        std::ostream& operator<< (std::ostream& os, const GradGenome<N>& g)
        {
            os << g.str();
            return os;
        }
    } // bn
} // morph
