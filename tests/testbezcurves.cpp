#include "morph/BezCurve.h"
#include "morph/BezCurvePath.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::BezCurvePath;

int main()
{
    int rtn = -1;

    // Make some control points
    pair<float,float> i, f, c1, c2;
    i.first = 1.0f;
    i.second = 1.0f;
    c1.first = 5.0f;
    c1.second = 5.0f;
    c2.first = 2.0f;
    c2.second = -4.0f;
    f.first = 10.0f;
    f.second = 1.0f;
    // Make a cubic curve
    BezCurve<float> cc3(i, f, c1, c2);

    // Make a second quartic curve.
    vector<pair<float, float>> quart;
    quart.push_back (f);
    quart.push_back (make_pair(10.0f,10.0f));
    quart.push_back (make_pair(10.0f,0.0f));
    quart.push_back (make_pair(12.0f,-5.0f));
    quart.push_back (make_pair(14.0f,0.0f));
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
