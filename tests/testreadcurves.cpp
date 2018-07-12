#include "ReadCurves.h"
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include "BezCoord.h"

using namespace std;
using morph::ReadCurves;
using morph::BezCoord;
using std::vector;

#define DEBUG 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

int main()
{
    int rtn = -1;

    try {
        ReadCurves r("../../tests/trial.svg");
        //r.save (0.001f);
        vector<BezCoord> pts = r.getCorticalPath().getPoints (0.01f);
        auto i = pts.begin();
        while (i != pts.end()) {
            cout << *i << endl;
            ++i;
        }
        // 0.329062,0.849467,1.00663
        cout.precision(12);
        cout << "pts[23] =  " << pts[23].t()
             << " " << pts[23].x()
             << " " << pts[23].y()
             << endl;
        cout << "pts[23] =  " << fabs(pts[23].t() - 0.32906216383)
             << " " << fabs(pts[23].x() - 0.849467217922)
             << " " << fabs(pts[23].y()- 1.00663292408)
             << endl;
        if ((fabs(pts[23].t() - 0.32906216383) < 0.000001f)
            && (fabs(pts[23].x() - 0.849467217922) < 0.000001f)
            && (fabs(pts[23].y()- 1.00663292408) < 0.000001f)) {
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
