/*
 * Another test program. This one is for use as a paper figure.
 */
#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include "morph/BezCurvePath.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>
#include <unistd.h>
#include <vector>
#include <list>

// For drawing the curves
#include <opencv2/opencv.hpp>

#include <chrono>
using namespace std::chrono;

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::BezCurvePath;
using morph::HexGrid;

using namespace cv;

// Colours are BGR(A) format
#define M_BLUE Scalar(255,0,0,10)
#define M_PURPLE Scalar(255,0,200,10)
#define M_GREEN Scalar(0,255,0,10)
#define M_RED Scalar(0,0,255,10)
#define M_YELLOW Scalar(0,255,255,10)
#define M_BLACK Scalar(0,0,0)
#define M_WHITE Scalar(255,255,255)
#define M_C1 Scalar(238,121,159) // mediumpurple2
#define M_C2 Scalar(238,58,178) // darkorchid2

// Colours for two separate Bezier curves
#define M_CURVE1 M_BLUE
#define M_CURVE2 M_C2
// User control points colour
#define M_CTRL M_BLACK
// Fit colour
#define M_FIT M_RED

void draw (Mat& pImg, BezCurvePath<FLT>& bcp, vector<pair<FLT,FLT>>& v, vector<pair<FLT,FLT>>& w, Scalar linecolfit, int sz) {

    unsigned int nFit = 200;
    vector<Point> fitted (nFit);
    vector<Point2d> tangents (nFit);
    vector<Point2d> normals (nFit);
    // Compute the curve for plotting
    bcp.computePoints ((unsigned int)nFit);
    vector<BezCoord<FLT>> coords = bcp.getPoints();
    vector<BezCoord<FLT>> tans = bcp.getTangents();
    vector<BezCoord<FLT>> norms = bcp.getNormals();
    for (unsigned int i = 0; i < nFit; ++i) {
        fitted[i] = Point(coords[i].x(),coords[i].y());
        tangents[i] = Point2d(tans[i].x(),tans[i].y());
        normals[i] = Point2d(norms[i].x(),norms[i].y());
    }

    // This is the fit line.
    for (size_t i=1; i<fitted.size(); i++) {
        line (pImg, fitted[i-1], fitted[i], linecolfit, 2);
    }

    // Add the control points in similar colours
    list<BezCurve<FLT>> theCurves = bcp.curves;
    size_t j = 0;
    for (auto curv : theCurves) {
        Scalar linecol = linecolfit; //j%2 ? linecolour1 : linecolour2;
        vector<pair<FLT,FLT>> ctrls = curv.getControls();
        // Draw the control points
        for (size_t cc = 0; cc<ctrls.size(); ++cc) {
            Point p1(ctrls[cc].first, ctrls[cc].second);
            circle (pImg, p1, sz, linecol, 2);
            if (cc==0 || cc==ctrls.size()-1) {
                circle (pImg, p1, sz-2, M_YELLOW, -1);
            } else {
                circle (pImg, p1, sz-2, M_GREEN, -1);
            }
        }
        // Draw in the lines to the control points
        Point ps(ctrls[0].first, ctrls[0].second);
        Point pe(ctrls[1].first, ctrls[1].second);
        line (pImg, ps, pe, linecol, 1);
        Point ps2(ctrls[ctrls.size()-2].first, ctrls[ctrls.size()-2].second);
        Point pe2(ctrls[ctrls.size()-1].first, ctrls[ctrls.size()-1].second);
        line (pImg, ps2, pe2, linecol, 1);
        j++;
    }

    // The user control points
    for (unsigned int i = 0; i < v.size(); ++i) {
        Point p1(v[i].first, v[i].second);
        circle (pImg, p1, 5, M_CTRL, -1);
    }

    for (unsigned int i = 0; i < w.size(); ++i) {
        Point p1(w[i].first, w[i].second);
        circle (pImg, p1, 5, M_CTRL, -1);
    }
}

int main (int argc, char** argv)
{
    bool holdVis = false;
    if (argc > 1) {
        string a1(argv[1]);
        cout << "a1 is " << a1 << endl;
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << endl;

    int rtn = 0;
    vector<pair<FLT,FLT>> v;
    FLT fac = 2.2;
    FLT xoff = -300.0;
    v.push_back (make_pair (xoff+fac*200,fac*500));
    v.push_back (make_pair (xoff+fac*300,fac*450));
    v.push_back (make_pair (xoff+fac*400,fac*400));
    v.push_back (make_pair (xoff+fac*450,fac*300));

    vector<pair<FLT,FLT>> w;
    w.push_back (v.back());
    w.push_back (make_pair (xoff+fac*500,fac*240));
    w.push_back (make_pair (xoff+fac*580,fac*30));
    w.push_back (make_pair (xoff+fac*630,fac*20));

    // First the analytical fit
    BezCurve<FLT> cv1;
    cv1.fit (v);
    BezCurve<FLT> cv2;
    cv2.fit (w);

    BezCurvePath<FLT> bcp;
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    // Create a frame as the background for our drawing.
    Mat frame = Mat (1200, 1200, CV_8UC3, M_WHITE);

    // Draw
    cout << "Draw the two analytical best-fit curves..." << endl;
    draw (frame, bcp, v, w, M_BLUE, 12);

    cout << "Do the control point-equalizing 0th order optimization..."<< endl;
    bool withopt = false;
    cv2.fit (w, cv1, withopt);

    bcp.removeCurve();
    bcp.removeCurve();
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    draw (frame, bcp, v, w, M_RED, 8);

#if 0
    // Reset and fit with optimzation
    withopt = true;
    // Reset the best fits:
    cv1.fit(v);
    cv2.fit(w);
    // Now do the fully optimized fit
    cv2.fit (w, cv1, withopt);

    bcp.removeCurve();
    bcp.removeCurve();
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    //draw (frame, bcp, v, w, M_FIT);
    //cout << "Semi-optimised is BLUE; Fully optimized is GREEN" << endl;
#endif

    namedWindow( "Curves", WINDOW_AUTOSIZE );// Create a window for display.
    imshow ("Curves", frame);

    if (holdVis == true) {
        // Wait for a key, then exit
        waitKey();
    }

    return rtn;
}
