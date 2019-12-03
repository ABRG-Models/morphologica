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
        ReadCurves r("../../tests/whiskerbarrels_withcentres.svg");
        //r.save (0.001f);
        vector<BezCoord> pts = r.getCorticalPath().getPoints (0.01f);
        auto i = pts.begin();
        while (i != pts.end()) {
            cout << *i << endl;
            ++i;
        }
        // Change as it's on a different outline 0.110460862517 0.739935457706 0.393380910158

        cout.precision(12);
        cout << "pts[23] =  " << pts[23].t()
             << " " << pts[23].x()
             << " " << pts[23].y()
             << endl;
        cout << "pts[23] =  " << fabs(pts[23].t() - 0.110460862517)
             << " " << fabs(pts[23].x() - 0.739935457706)
             << " " << fabs(pts[23].y() - 0.393380910158)
             << endl;
        if ((fabs(pts[23].t() - 0.110460862517) < 0.000001f)
            && (fabs(pts[23].x() - 0.739935457706) < 0.000001f)
            && (fabs(pts[23].y() - 0.393380910158) < 0.000001f)) {
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
