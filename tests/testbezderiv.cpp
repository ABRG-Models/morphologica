#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include "morph/BezCurvePath.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>
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

#define M_BLUE Scalar(255,0,0,10)
#define M_GREEN Scalar(0,255,0,10)
#define M_RED Scalar(0,0,255,10)
#define M_YELLOW Scalar(0,255,255,10)
#define M_BLACK Scalar(0,0,0)
#define M_WHITE Scalar(255,255,255)
#define M_C1 Scalar(238,121,159) // mediumpurple2
#define M_C2 Scalar(238,58,178) // darkorchid2

void draw (Mat& pImg, BezCurvePath<FLT>& bcp, morph::vvec<morph::vec<FLT, 2>>& v, morph::vvec<morph::vec<FLT, 2>>& w, Scalar linecolour) {

    // Add the control points in similar colours
    list<BezCurve<FLT>> theCurves = bcp.curves;
    size_t j = 0;
    for (auto curv : theCurves) {
        Scalar linecol = j%2 ? M_BLUE : M_GREEN;
        morph::vvec<morph::vec<FLT, 2>> ctrls = curv.getControls();
        for (size_t cc = 0; cc<ctrls.size(); ++cc) {
            Point p1(ctrls[cc][0], ctrls[cc][1]);
            circle (pImg, p1, 5, linecol, 2);
            if (cc==0 || cc==ctrls.size()-1) {
                circle (pImg, p1, 2, M_BLACK, -1);
            } else {
                circle (pImg, p1, 2, M_WHITE, -1);
            }
        }
        Point ps(ctrls[0][0], ctrls[0][1]);
        Point pe(ctrls[1][0], ctrls[1][1]);
        line (pImg, ps, pe, linecolour, 1);
        Point ps2(ctrls[ctrls.size()-2][0], ctrls[ctrls.size()-2][1]);
        Point pe2(ctrls[ctrls.size()-1][0], ctrls[ctrls.size()-1][1]);
        line (pImg, ps2, pe2, linecolour, 1);
        j++;
    }

    // User controls
    for (unsigned int i = 0; i < v.size(); ++i) {
        Point p1(v[i][0], v[i][1]);
        circle (pImg, p1, 2, M_BLACK, -1);
    }

    for (unsigned int i = 0; i < w.size(); ++i) {
        Point p1(w[i][0], w[i][1]);
        circle (pImg, p1, 2, M_BLACK, -1);
    }

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
    for(size_t i=1; i<fitted.size(); i++) {
        line (pImg, fitted[i-1], fitted[i], linecolour, 1);
    }

    if (v.empty()) {
        for (size_t i=0; i<fitted.size(); i++) {
            Point2d normLen = normals[i]*100.0;
#if 0 // Ok on recent OpenCV
            line (pImg, fitted[i], fitted[i]+Point2i(normLen), linecolour, 1);
#else // This form works on the OpenCV on Ubuntu 16
	    Point2i nli;
	    nli.x = (int)normLen.x;
	    nli.y = (int)normLen.y;
            line (pImg, fitted[i], fitted[i]+nli, linecolour, 1);
#endif
        }
    }

    circle (pImg, Point(10,10), 2, M_BLACK, -1);
    circle (pImg, Point(1600,10), 2, M_BLACK, -1);
    circle (pImg, Point(1600,1000), 2, M_BLACK, -1);
    circle (pImg, Point(800,10), 2, M_BLACK, -1);
    circle (pImg, Point(800,1000), 2, M_BLACK, -1);
    circle (pImg, Point(10,1000), 2, M_BLACK, -1);
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
    morph::vvec<morph::vec<FLT, 2>> v = {
        {90,100},
        {140,200},
        {200,380},
        {270,530},
        {350,620},
        {430,730}
    };

    morph::vvec<morph::vec<FLT, 2>> w = {
        v.back(),
        {530,790},
        {610,850},
        {760,840},
        {840,760},
        {980,650}
    };

    // First the analytical fit
    BezCurve<FLT> cv1;
    cv1.fit (v);
    BezCurve<FLT> cv2;
    cv2.fit (w);

    BezCurvePath<FLT> bcp;
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    // Create a frame as the background for our drawing.
    Mat frame = Mat (800, 1000, CV_8UC3, M_WHITE);

    // Draw
    cout << "Draw the two analytical best-fit curves..." << endl;
    draw (frame, bcp, v, w, M_RED);

    cout << "Do the control point-equalizing 0th order optimization..."<< endl;
    bool withopt = false;
    cv2.fit (w, cv1, withopt);

    bcp.removeCurve();
    bcp.removeCurve();
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    //v.clear();
    //w.clear();
    draw (frame, bcp, v, w, M_BLUE);

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

    //v.clear();
    //w.clear();
    draw (frame, bcp, v, w, M_GREEN);
    cout << "Semi-optimised is BLUE; Fully optimized is GREEN" << endl;

    namedWindow( "Curves", WINDOW_AUTOSIZE );// Create a window for display.
    imshow ("Curves", frame);
    if (holdVis == true) {
        // Wait for a key, then exit
        waitKey();
    }

    return rtn;
}
