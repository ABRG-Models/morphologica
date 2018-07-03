#include "BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;

int main()
{
    int rtn = -1;

    // Make some control points
    pair<float,float> i, f, c1, c2;
    i.first = 1;
    i.second = 1;
    c1.first = 5;
    c1.second = 5;
    c2.first = 2;
    c2.second = -4;
    f.first = 10;
    f.second = 1;

    // Make a cubic curve
    BezCurve cc(i, f, c1, c2);

    // Length of step along curve
    std::ofstream f1;
    f1.open ("tests/curve.csv", std::ios::trunc|std::ios::out);
    f1.precision(12);
    vector<BezCoord> a = cc.computePoints ((unsigned int)100);
    vector<BezCoord>::iterator ai = a.begin();
    int ii = 0;
    while (ai != a.end()) {
        if (ai->getNullCoordinate() == false) {
            f1 << *(ai) << endl;

            // Pick a value out of the list and compare to validate this test
            if (ii == 55) {
                if ((ai->x() - 4.24224996567f) < 0.00000001
                    && (ai->y() - 0.294625133276f) < 0.00000001
                    && (ai->t() - 0.55f) < 0.00000001) {
                    rtn = 0; // return success!
                } else {
                    cerr << "Failed. ai->x(): " << ai->x() << " ai->y(): " << ai->y() << " ai->t(): " << ai->t() <<endl;
                }
            }

        } else {
            //cout << "Remaining: " << ai->getRemaining() << endl;
        }
        ++ai; ++ii;
    }
    f1.close();

    std::ofstream f2;
    f2.open ("tests/ctrl.csv", std::ios::trunc|std::ios::out);
    f2 << i.first << "," << i.second << endl;
    f2 << c1.first << "," << c1.second << endl;
    f2 << c2.first << "," << c2.second << endl;
    f2 << f.first << "," << f.second << endl;
    f2.close();

    return rtn;
}
