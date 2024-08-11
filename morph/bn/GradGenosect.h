#pragma once

#include <cstddef>

namespace morph {
    namespace  bn {
        //! template metafunction to select size type for a GradGenome's genosect_t
        template <std::size_t N> struct GradGenosect {}; // Default should fail to compile
        template<> struct GradGenosect<1> { typedef unsigned char type; };
        template<> struct GradGenosect<2> { typedef unsigned char type; };
        template<> struct GradGenosect<3> { typedef unsigned char type; };
        template<> struct GradGenosect<4> { typedef unsigned char type; };

        template<> struct GradGenosect<5> { typedef unsigned short type; };
        template<> struct GradGenosect<6> { typedef unsigned short type; };
        template<> struct GradGenosect<7> { typedef unsigned short type; };
        template<> struct GradGenosect<8> { typedef unsigned short type; };

        template<> struct GradGenosect<9>  { typedef unsigned int type; };
        template<> struct GradGenosect<10> { typedef unsigned int type; };
        template<> struct GradGenosect<11> { typedef unsigned int type; };
        template<> struct GradGenosect<12> { typedef unsigned int type; };
        template<> struct GradGenosect<13> { typedef unsigned int type; };
        template<> struct GradGenosect<14> { typedef unsigned int type; };
        template<> struct GradGenosect<15> { typedef unsigned int type; };
        template<> struct GradGenosect<16> { typedef unsigned int type; };

        template<> struct GradGenosect<17> { typedef unsigned long long int type; };
        template<> struct GradGenosect<18> { typedef unsigned long long int type; };
        template<> struct GradGenosect<19> { typedef unsigned long long int type; };
        template<> struct GradGenosect<20> { typedef unsigned long long int type; };
        template<> struct GradGenosect<21> { typedef unsigned long long int type; };
        template<> struct GradGenosect<22> { typedef unsigned long long int type; };
        template<> struct GradGenosect<23> { typedef unsigned long long int type; };
        template<> struct GradGenosect<24> { typedef unsigned long long int type; };
        template<> struct GradGenosect<25> { typedef unsigned long long int type; };
        template<> struct GradGenosect<26> { typedef unsigned long long int type; };
        template<> struct GradGenosect<27> { typedef unsigned long long int type; };
        template<> struct GradGenosect<28> { typedef unsigned long long int type; };
        template<> struct GradGenosect<29> { typedef unsigned long long int type; };
        template<> struct GradGenosect<30> { typedef unsigned long long int type; };
        template<> struct GradGenosect<31> { typedef unsigned long long int type; };
        template<> struct GradGenosect<32> { typedef unsigned long long int type; };
    }
}
