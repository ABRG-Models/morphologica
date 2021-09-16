// Two singleton random number generator classes for randSingle and randDouble
//
// This code makes use of RandUniform<> from Random.h, giving easy to use
// morph::randSingle() and morph::randDouble() implementations that are relatively fast
// (each creates a single instance of RandUniform<> in memory) and provide good quality
// pseudo random numbers based on the mt19937 algorithm.
//
// Author: Seb James, August 2021.

#pragma once
#include <morph/Random.h>

// #define RANDSINGLE if you want a single precision rng
#ifdef RANDSINGLE
namespace morph {
    // Uniform random number generator, singles
    class srng
    {
    private:
        srng() {};
        ~srng() {};
        static srng* pInstance_srng;
    public:
        static srng* i()
        {
            if (srng::pInstance_srng == 0) { srng::pInstance_srng = new srng; }
            return srng::pInstance_srng;
        }
        float get() { return this->rng.get(); }
        morph::RandUniform<float> rng;
    };
    srng* srng::pInstance_srng = 0;
    static float randSingle() { return morph::srng::i()->get(); }
}
#endif

// #define NO_RANDDOUBLE if you DON't want a double precision rng
#ifndef NO_RANDDOUBLE
namespace morph {
    // Uniform random number generator, doubles
    class drng
    {
    private:
        drng() {};
        ~drng() {};
        static drng* pInstance_drng;
    public:
        // The instance public function.
        static drng* i()
        {
            if (drng::pInstance_drng == 0) { drng::pInstance_drng = new drng; }
            return drng::pInstance_drng;
        }
        double get() { return this->rng.get(); }
        morph::RandUniform<double> rng;
    };
    // Globally initialise drng instance pointer to NULL.
    drng* drng::pInstance_drng = 0;
    static double randDouble() { return morph::drng::i()->get(); }
}
#endif
