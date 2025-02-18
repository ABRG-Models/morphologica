#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <limits>

#include <chrono>
using namespace std::chrono;

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = 0;

    morph::vvec<morph::vec<float, 2>> c = {
        {9.0f,10.0f},
        {19.0f,16.0f},
        {42.0f,33.0f},
        {56.0f,47.0f},
        {75.0f,52.0f},
        {94.0f,59.0f},
        {110.0f,68.0f}
    };

    BezCurve<FLT> cv (c);

    cout << "Defined a " << cv.getOrder() << " nd/rd/th order curve" << endl;

    BezCoord<FLT> bm = cv.computePointMatrix (0.4);
    BezCoord<FLT> bg = cv.computePointGeneral (0.4);
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
        cv.computePointMatrix (t);
    }
    milliseconds m_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds matrix_time = m_af - m_b4;
    cout << "Computed " << (1.0f/tstep) << " matrix bezier points in " << matrix_time.count() << " ms" << endl;

    milliseconds g_b4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    for (float t = 0.0f; t < 1.0f; t+=tstep) {
        cv.computePointGeneral (t);
    }
    milliseconds g_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds general_time = g_af - g_b4;
    cout << "Computed " << (1.0f/tstep) << " bezier points (general method) in " << general_time.count() << " ms" << endl;

    if (cv.getOrder() < 4) {
        milliseconds o_b4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        for (float t = 0.0f; t < 1.0f; t+=tstep) {
            cv.computePoint (t);
        }
        milliseconds o_af = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        milliseconds opt_time = o_af - o_b4;
        cout << "Computed " << (1.0f/tstep) << " bezier points (optimized method) in " << opt_time.count() << " ms" << endl;
    }

    return rtn;
}
