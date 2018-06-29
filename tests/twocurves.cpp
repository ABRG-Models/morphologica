#include "BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;

/*
 * This test joins two curves together and selects points along eac
 * curve, making sure to keep them evenly spaced.
 */

int main()
{
    int rtn = -1;

    // Make some control points
    pair<float,float> p1_i, p1_f, p1_c1, p1_c2;
    p1_i.first = 1;
    p1_i.second = 1;
    p1_c1.first = 5;
    p1_c1.second = 5;
    p1_c2.first = 2;
    p1_c2.second = -4;
    p1_f.first = 10;
    p1_f.second = 1;

    // Make a cubic curve
    BezCurve cc1(p1_i, p1_f, p1_c1, p1_c2);

    pair<float,float> p2_f, p2_c1, p2_c2;
    p2_c1.first = 15;
    p2_c1.second = 2;
    p2_c2.first = 16;
    p2_c2.second = 5;
    p2_f.first = 20;
    p2_f.second = 3;

    BezCurve cc2(p1_f, p2_f, p2_c1, p2_c2);

    // Now have two curves, generate points on the curves
    vector<BezCoord> a = cc1.computePoints (float(1.0));

    // Look at 'em
    vector<BezCoord>::iterator ai = a.begin();
    int ii = 0;
    while (ai != a.end()) {
        if (ai->getNullCoordinate() == false) {
            cout << *(ai) << endl;
        }
        ++ai;
        ++ii;
    }
    cout << "Remaining: " << a.back().getRemaining() << endl;


    vector<BezCoord> b = cc2.computePoints (float(1.0));

    ai = b.begin();
    ii = 0;
    while (ai != b.end()) {
        if (ai->getNullCoordinate() == false) {
            cout << *(ai) << endl;
        }
        ++ai;
        ++ii;
    }
    cout << "Remaining: " << b.back().getRemaining() << endl;

    return rtn;
}
