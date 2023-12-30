// Include only morph::randSingle.h from rng.h
#pragma once
#ifdef NO_RANDSINGLE
# undef NO_RANDSINGLE
#endif
#define NO_RANDDOUBLE 1 // disable randDouble
#include <morph/rng.h>
