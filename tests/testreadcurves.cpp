#include <iostream>
#define DEBUG 1
#define DBGSTREAM std::cout
#include "morph/MorphDbg.h"

#include "morph/ReadCurves.h"
#include <utility>
#include <vector>
#include <fstream>
#include <cmath>
#include "morph/BezCoord.h"
#include "morph/BezCurvePath.h"

using namespace std;
using morph::ReadCurves;
using morph::BezCoord;
using morph::BezCurvePath;
using std::vector;

int main()
{
    int rtn = -1;

    try {
        ReadCurves r("../../tests/trial.svg");
        BezCurvePath<float> bcp = r.getCorticalPath();
        bcp.computePoints (0.01f);
        vector<BezCoord<float>> pts = bcp.getPoints();
        cout << "Got " << pts.size() << " points with getPoints()" << endl;
        auto i = pts.begin();
        while (i != pts.end()) {
            cout << *i << endl;
            ++i;
        }
        // 0.329310834408 0.849295854568 1.00672543049
        cout.precision(12);
        cout << "pts[23] =  " << pts[23].t()
             << " " << pts[23].x()
             << " " << pts[23].y()
             << endl;
        if ((std::abs(pts[23].t() - 0.329311) < 0.00001f)
            && (std::abs(pts[23].x() - 0.849296) < 0.00001f)
            && (std::abs(pts[23].y()- 1.00673) < 0.00001f)) {
            cout << "Matches expectation; rtn IS 0" << endl;
            rtn = 0;
        } else {
            cout << "rtn not 0" << endl;
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
