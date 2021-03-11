#pragma once
#include "point.h"

namespace morph{ namespace softmats{
/**
 * Edge data structure
 * 
 * @author Alejandro Jimenez Rodriguez
 */
    typedef struct Edge{
        Point* p1;
        Point* p2;
    }Edge;
}}