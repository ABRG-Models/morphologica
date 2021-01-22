#ifndef BOX_H
#define BOX_H 

#include <vector>
#include <iostream>
#include <armadillo>

#include "../core/point.h"

using namespace std;
using namespace arma;

namespace morph{ namespace softmats{


class Box{
public:
    vec min;
    vec max;

    Box();
    Box( vec min, vec max );
    bool inside( Point *p );
    bool collide( Box& b );
    // static
    static void compute( vector<Point *>& points, Box *b );
};

}}

#endif