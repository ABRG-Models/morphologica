#include "MathAlgo2.h"
#include <iostream>

using namespace morph;
using namespace std;

// Testing MathAlgo with vector/array types.

int main()
{
    int rtn = 0;

    double f = 0.0f;
    vector<double> vf;
    vf.push_back (f);
    f = 2.0f;
    vf.push_back (f);
    f = 1.0f;
    vf.push_back (f);
    cout << "double version" << endl;
    pair<double, double> vfmm = MathAlgo::maxmin (vf);
    cout << "max/min: " << vfmm.first << "/" << vfmm.second << endl;

    array<double, 3> v1 = { 1.0f, 1.0f, 1.0f };
    array<double, 3> v2 = { 0.5f, 2.0f, 1.0f };
    array<double, 3> v3 = { 1.0f, 1.0f, 2.1f };
    vector< array<double, 3> > vv3;
    vv3.push_back (v1);
    vv3.push_back (v2);
    vv3.push_back (v3);
    cout << "array<double,3> version" << endl;
    pair<array<double,3>, array<double,3> > vv3mm = MathAlgo::maxmin (vv3);
    cout << "max/min: (" << vv3mm.first[0] << ","
         << vv3mm.first[1] << "," << vv3mm.first[2] << ")/(" << vv3mm.second[0] << ","
         << vv3mm.second[1] << "," << vv3mm.second[2] << ")" << endl;

    return rtn;
}
