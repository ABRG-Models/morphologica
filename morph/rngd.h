// Include only morph::randDouble.h from rng.h
#pragma once
#define NO_RANDSINGLE 1 // disable randSingle
#ifdef NO_RANDDOUBLE
# undef NO_RANDDOUBLE
#endif
#include <morph/rng.h>
