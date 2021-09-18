// Include only morph::randDouble.h from rng.h
#pragma once
#ifdef RANDSINGLE
# undef RANDSINGLE
#endif
#ifdef NO_RANDDOUBLE
# undef NO_RANDDOUBLE
#endif
#include <morph/rng.h>
