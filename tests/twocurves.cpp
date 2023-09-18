#include "morph/BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

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
    morph::vec<float, 2> p1_i, p1_f, p1_c1, p1_c2;
    p1_i = {1,1};
    p1_c1 = {5,5};
    p1_c2 = {2,-4};
    p1_f = {10,1};

    // Make a cubic curve
    BezCurve<float> cc1(p1_i, p1_f, p1_c1, p1_c2);

    morph::vec<float, 2> p2_f, p2_c1, p2_c2;
    p2_c1 = {15,2};
    p2_c2 = {16,5};
    p2_f = {20,3};

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
    cout << "Last element: " << ai->coord << endl;
    morph::vec<float, 2> last_of_cc1 = ai->coord;

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
    cout << "Last element: " << ai->coord << endl;

    // Now determine the Cartesian distance between last of cc1 and
    // first of cc2
    float d = (b.front().coord - last_of_cc1).length();
    cout << "Distance between adjoining curves: " << d  << endl;

    if (steplen - d < 0.02) { rtn = 0; }

    return rtn;
}
