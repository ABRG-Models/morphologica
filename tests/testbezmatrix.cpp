#include "HexGrid.h"
#include "BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>

#include <chrono>
using namespace std::chrono;

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = -1;

    pair<float,float> v1 = make_pair (-0.28f, 0.0f);
    pair<float,float> v2 = make_pair (0.28f, 0.0f);
    pair<float,float> v3 = make_pair (0.28f, 0.45f);
    pair<float,float> v4 = make_pair (-0.28f, 0.45f);
    vector<pair<float,float>> c;
    c.push_back (v1);
    c.push_back (v2);
    c.push_back (v3);
    c.push_back (v4);

    BezCurve cv (c);

    cout << "Defined a " << cv.getOrder() << " nd/rd/th order curve" << endl;

    BezCoord bm = cv.computePointMatrix (0.4);
    BezCoord bg = cv.computePointGeneral (0.4);
    cout << "matrix method: " << bm << endl;
    cout << "general method: " << bg << endl;

    float xdiff = bm.x() - bg.x();
    float ydiff = bm.y() - bg.y();
    cout << "x points differ by: " << xdiff << endl;
    cout << "y points differ by: " << ydiff << endl;

    if (xdiff < numeric_limits<float>::epsilon() && ydiff < numeric_limits<float>::epsilon()) {
        cout << "General & matrix methods compute same point" << endl;
    } else {
        ++rtn;
    }

    // Profile matrix and general methods
    float tstep = 0.00001f;
    milliseconds m_b4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    for (float t = 0.0f; t < 1.0f; t+=tstep) {
        BezCoord bm = cv.computePointMatrix (t);
    }
    milliseconds m_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds matrix_time = m_af - m_b4;
    cout << "Computed " << (1.0f/tstep) << " matrix bezier points in " << matrix_time.count() << " ms" << endl;

    milliseconds g_b4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    for (float t = 0.0f; t < 1.0f; t+=tstep) {
        BezCoord bm = cv.computePointGeneral (t);
    }
    milliseconds g_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds general_time = g_af - g_b4;
    cout << "Computed " << (1.0f/tstep) << " bezier points (general method) in " << general_time.count() << " ms" << endl;

    milliseconds o_b4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    for (float t = 0.0f; t < 1.0f; t+=tstep) {
        BezCoord bm = cv.computePoint (t);
    }
    milliseconds o_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds opt_time = o_af - o_b4;
    cout << "Computed " << (1.0f/tstep) << " bezier points (optimized cubic method) in " << opt_time.count() << " ms" << endl;

    return rtn;
}
