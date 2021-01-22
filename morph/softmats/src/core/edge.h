#pragma once
#include "point.h"

namespace morph{ namespace softmats{
    typedef struct Edge{
        Point* p1;
        Point* p2;
    }Edge;
}}