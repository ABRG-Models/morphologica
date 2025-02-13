/*
 * Another test program. This one is for use as a paper figure.
 */
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>
#include <array>
#include <vector>
#include <list>
#include "morph/BezCurve.h"
#include "morph/BezCurvePath.h"
#include "morph/Visual.h"
#include "morph/GraphVisual.h"
#include "morph/vec.h"

// Draw a bezcurve on the graph gv
void draw (morph::GraphVisual<float>* gv,
           morph::BezCurvePath<FLT>& bcp,
           morph::vvec<morph::vec<FLT, 2>>& v,
           std::array<float, 3> linecolfit,
           float sz, bool drawuserctrl = true)
{
    namespace m = morph;

    unsigned int nFit = 200;
    m::vvec<m::vec<float, 2>> fitted (nFit);
    m::vvec<m::vec<float, 2>> tangents (nFit);
    m::vvec<m::vec<float, 2>> normals (nFit);
    // Compute the curve for plotting
    bcp.computePoints ((unsigned int)nFit);
    std::vector<m::BezCoord<FLT>> coords = bcp.getPoints();
    std::vector<m::BezCoord<FLT>> tans = bcp.getTangents();
    std::vector<m::BezCoord<FLT>> norms = bcp.getNormals();
    for (unsigned int i = 0; i < nFit; ++i) {
        fitted[i] = m::vec<double, 2>{coords[i].x(), coords[i].y()}.as_float();
        tangents[i] = m::vec<double, 2>{tans[i].x(), tans[i].y()}.as_float();
        normals[i] = m::vec<double, 2>{norms[i].x(), norms[i].y()}.as_float();
    }

    m::DatasetStyle dsl(m::stylepolicy::lines);
    dsl.linecolour = linecolfit;
    dsl.linewidth = sz/4.0f;
    gv->setdata (fitted, dsl);

    m::DatasetStyle dsm(m::stylepolicy::markers);
    dsm.markercolour = linecolfit;
    dsm.markersize = sz;
    dsm.markerstyle = m::markerstyle::circle;

    m::DatasetStyle dsb(m::stylepolicy::lines);
    dsb.markercolour = linecolfit;
    dsb.linecolour = linecolfit;
    dsb.linewidth = sz/6.0f;
    dsb.markersize = sz;

    // Add the control points in similar colours
    std::list<m::BezCurve<FLT>> theCurves = bcp.curves;
    for (auto curv : theCurves) {
        m::vvec<m::vec<FLT, 2>> ctrlsd = curv.getControls();
        m::vvec<m::vec<float, 2>> ctrls (ctrlsd.size());
        for (size_t i = 0; i < ctrlsd.size(); ++i) { ctrls[i] = ctrlsd[i].as_float(); }
        // Draw the control points
        gv->setdata (ctrls, dsm);

        // Draw in the lines to the control points
        m::vvec<m::vec<float, 2>> pspe = { ctrls[0], ctrls[1] };
        gv->setdata (pspe, dsb);

        m::vvec<m::vec<float, 2>> pspe2 = { ctrls[ctrls.size()-2], ctrls[ctrls.size()-1] };
        gv->setdata (pspe2, dsb);
    }

    if (drawuserctrl) {
        // The user control points
        m::vvec<m::vec<float, 2>> vf (v.size());
        for (size_t i = 0; i < v.size(); ++i) { vf[i] = v[i].as_float(); }
        gv->setdata (vf, dsm);
    }
}

int main (int argc, char** argv)
{
    namespace m = morph;

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        std::cout << "a1 is " << a1 << std::endl;
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    int rtn = 0;
    FLT fac = 3.4;
    FLT xoff = -400.0;

    m::vvec<m::vec<FLT, 2>> v = {
        {xoff+fac*200,fac*500 },
        {xoff+fac*300,fac*450 },
        {xoff+fac*400,fac*400 },
        {xoff+fac*450,fac*300 }
    };

    m::vvec<m::vec<FLT, 2>> w = {
        v.back(),
        {xoff+fac*440,fac*180 },
        {xoff+fac*580,fac*30 },
        {xoff+fac*630,fac*20 }
    };

    // First the analytical fit
    m::BezCurve<FLT> cv1;
    cv1.fit (v);
    m::BezCurve<FLT> cv2;
    cv2.fit (w);

    m::BezCurvePath<FLT> bcp;
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    m::BezCurvePath<FLT> bcp1;
    bcp1.addCurve (cv1);
    m::BezCurvePath<FLT> bcp2;
    bcp2.addCurve (cv2);

    // Create a frame as the background for our drawing.
    m::Visual<> scene(1600, 1000, "Beziers");
    m::vec<float> offset = {-1, -1, 0};
    auto gv = std::make_unique<m::GraphVisual<float>>(offset);
    scene.bindmodel (gv);
    gv->setsize (2,2);
    gv->setlimits (m::range<float>{200,1700}, m::range<float>{0,1700});

    std::cout << "Draw the two analytical best-fit curves..." << std::endl;
    draw (gv.get(), bcp1, v, m::colour::blue, 0.024);
    draw (gv.get(), bcp2, w, m::colour::crimson, 0.024);

    std::cout << "Do the control point-equalizing 0th order optimization..."<< std::endl;
    bool withopt = false;
    cv2.fit (w, cv1, withopt);

    bcp.removeCurve();
    bcp.removeCurve();
    bcp.addCurve (cv1);
    bcp.addCurve (cv2);

    m::vvec<m::vec<FLT, 2>> vw (v);
    vw.insert (vw.end(), w.begin(), w.end());

    draw (gv.get(), bcp, vw, m::colour::darkorchid2, 0.024, false);

    gv->finalize();
    scene.addVisualModel (gv);

    if (holdVis == true) { scene.keepOpen(); }

    return rtn;
}
