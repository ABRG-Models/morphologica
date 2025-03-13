#include "morph/BezCurve.h"
#include "morph/BezCurvePath.h"
#include <utility>
#include <iostream>
#include <fstream>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::BezCurvePath;

int main()
{
    int rtn = -1;

    // Make some control points
    morph::vec<float, 2> i, f, c1, c2;
    i = {1,1};
    c1 = {5,5};
    c2 = {2,-4};
    f = {10,1};
    // Make a cubic curve
    BezCurve<float> cc3(i, f, c1, c2);

    // Make a second quartic curve.
    morph::vvec<morph::vec<float, 2>> quart = {f, {10.0f,10.0f}, {10.0f,0.0f},  {12.0f,-5.0f},  {14.0f,0.0f}};
    BezCurve<float> cc4(quart);

    // Put em in a BezCurvePath
    BezCurvePath<float> bcp;
    bcp.name = "testbezcurves";
    bcp.addCurve (cc3);
    bcp.addCurve (cc4);

    unsigned int nPoints = 201;
    bcp.computePoints (nPoints);
    vector<BezCoord<float>> points = bcp.getPoints();
    vector<BezCoord<float>> tans = bcp.getTangents();

    for (auto p : points) {
        cout << p.x() << "," << p.y() << endl;
    }
    cout << "Tangents" << endl;
    for (auto ta : tans) {
        cout << ta.x() << "," << ta.y() << endl;
    }

    if (points.size() == nPoints) {
        rtn = 0;
    }

    return rtn;
}
