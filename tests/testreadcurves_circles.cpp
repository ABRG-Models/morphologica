#include "ReadCurves.h"
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include "BezCoord.h"
#include "BezCurvePath.h"

using namespace std;
using morph::ReadCurves;
using morph::BezCoord;
using morph::BezCurvePath;
using std::vector;

#define DEBUG 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

int main()
{
    int rtn = -1;

    try {
        ReadCurves r("../../tests/whiskerbarrels_withcentres.svg");
        //r.save (0.001f);
        BezCurvePath<float> bcp = r.getCorticalPath();
        bcp.computePoints (0.01f);
        vector<BezCoord<float>> pts = bcp.getPoints();
        auto i = pts.begin();
        while (i != pts.end()) {
            cout << *i << endl;
            ++i;
        }

        cout.precision(12);
        cout << "pts[23] =  " << pts[23].t()
             << " " << pts[23].x()
             << " " << pts[23].y()
             << endl;
        if ((fabs(pts[23].t() - 0.110523112118) < 0.000001f)
            && (fabs(pts[23].x() - 0.74002712965) < 0.000001f)
            && (fabs(pts[23].y() - 0.393309623003) < 0.000001f)) {
            cout << "rtn IS 0" << endl;
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
