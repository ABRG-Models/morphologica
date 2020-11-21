#pragma once

namespace morph {
    namespace  bn {
        /*!
         * Genosect is a template metafunction, with several specializations. It has one
         * attribute, type, which client code should use as the correct type for the
         * Genome's array. Each Genome section in the Genome's array has 2^K bits.
         */
        template <size_t K = 0> struct Genosect {}; // Default should fail to compile
        template<> struct Genosect<1> { typedef unsigned char type; };
        template<> struct Genosect<2> { typedef unsigned char type; };
        template<> struct Genosect<3> { typedef unsigned char type; };
        template<> struct Genosect<4> { typedef unsigned short type; };
        template<> struct Genosect<5> { typedef unsigned int type; };
        template<> struct Genosect<6> { typedef unsigned long long int type; };
    }
}
