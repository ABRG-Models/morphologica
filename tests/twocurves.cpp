#include "morph/BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;

/*
 * This test joins two curves together and selects points along each
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
    BezCurve<float> cc1(p1_i, p1_f, p1_c1, p1_c2);

    pair<float,float> p2_f, p2_c1, p2_c2;
    p2_c1.first = 15;
    p2_c1.second = 2;
    p2_c2.first = 16;
    p2_c2.second = 5;
    p2_f.first = 20;
    p2_f.second = 3;

    BezCurve<float> cc2(p1_f, p2_f, p2_c1, p2_c2);

    // Now have two curves, generate points on the curves
    float steplen = 1.0f;

    vector<BezCoord<float>> a = cc1.computePoints (steplen);

    // Look at 'em
    typename vector<BezCoord<float>>::iterator ai = a.begin();
    int ii = 0;
    while (ai != a.end()) {
        if (ai->getNullCoordinate() == false) {
            cout << *(ai) << endl;
        }
        ++ai;
        ++ii;
    }
    --ai; // Step back to final null coordinate
    cout << "Remaining: " << ai->getRemaining() << endl;
    --ai; // Once more to last non-null coordinate
    cout << "Last element: " << ai->getCoord().first << endl;
    pair<float,float> last_of_cc1 = ai->getCoord();

    float firstl = steplen - a.back().getRemaining();
    vector<BezCoord<float>> b = cc2.computePoints (steplen, firstl);

    ai = b.begin();
    ii = 0;
    while (ai != b.end()) {
        if (ai->getNullCoordinate() == false) {
            cout << *(ai) << endl;
        }
        ++ai;
        ++ii;
    }
    --ai; // Step back to final null coordinate
    cout << "Remaining: " << ai->getRemaining() << endl;
    --ai; // Once more to last non-null coordinate
    cout << "Last element: " << ai->getCoord().first << endl;
    pair<float,float> first_of_cc2 = b.front().getCoord();

    // Now determine the Cartesian distance between last of cc1 and
    // first of cc2
    float dx = first_of_cc2.first - last_of_cc1.first;
    float dy = first_of_cc2.second - last_of_cc1.second;

    float d = std::hypot (dx, dy);

    cout << "Distance between adjoining curves: " << d << endl;

    if (steplen - d < 0.02) {
        rtn = 0;
    }

    return rtn;
}
