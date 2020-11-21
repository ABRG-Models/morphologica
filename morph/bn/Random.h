/*
 * A singleton class to hold random number generators for use in Boolean gene networks.
 */

#pragma once

#include <array>
#include <morph/Random.h>
#include <morph/bn/Genosect.h>

namespace morph {
    namespace  bn {

        //! Access random number generators for boolean gene networks
        template<size_t N=5, size_t K=5>
        class Random
        {
            using genosect_t = typename Genosect<K>::type;

            //! Private constructor/destructor
            Random() {};
            ~Random() {};
            //! A pointer returned to the single instance of this class
            static Random* pInstance;

        public:
            //! The instance public function. Uses the very short name 'i' to keep code tidy.
            static Random* i()
            {
                if (Random::pInstance == 0) {
                    Random::pInstance = new morph::bn::Random;
                    //Random::i()->init(); // If required
                }
                return Random::pInstance;
            }

            static constexpr size_t gw = N*(1<<K);
            std::array<float, gw> rnums; // Hmm, now our array contains an array. I want
                                         // a singleton rng class which also has this
                                         // array allocated.

            //! Populate rnums with gw new random numbers
            void fill_rnums() { this->frng.get<gw> (rnums); }

            //! A random number generator of width genosect_t.
            morph::RandUniform<genosect_t> genosect_rng;

            //! A floating point random number generator
            morph::RandUniform<float> frng;
        };
    }
}
