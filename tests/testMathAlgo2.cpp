#include "MathAlgo.h"
#include <iostream>

using namespace morph;
using namespace std;

// Testing MathAlgo with vector/array types.

int main()
{
    int rtn = 0;

    array<float, 3> v3 = { 1.0f, 1.0f, 1.0f };
    vector<array<float, 3>> vv3;
    vv3.push_back (v3);
    v3[0] = 0.4f;
    vv3.push_back (v3);
    v3[2] = 2.4f;
    pair<array<float,3>, array<float,3> > vv3 = MathAlgo<array<float, 3>>::maxmin (vf);

    return rtn;
}
