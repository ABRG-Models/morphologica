/*
 * Read the given svg file (first argument on command line), then
 * create a HexGrid and show the boundary.
 *
 * Useful to demonstrate that the morphologica code can read your
 * Adobe Illustrator or Inkscape generated SVG file.
 *
 * Author: Seb James <seb.james@sheffield.ac.uk>
 *
 * Date: June 2019
 */

#include "ReadCurves.h"
#include "display.h"
#include "tools.h"
#include "HexGrid.h"
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

//#define DEBUG 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

int main(int argc, char** argv)
{
    int rtn = -1;

    if (argc < 2 && argc > 0) {
        cerr << "Usage: " << argv[0]
             << " ./path/to/curves.svg [domain-span (mm default:3)] [hexdia (mm default:0.01)]" << endl;
        return rtn;
    }
    float gridspan = 3.0f;
    if (argc > 2) {
        gridspan = atof (argv[2]);
        cout << "User supplied grid width: " << gridspan << " mm" << endl;
    }
    float hexdia = 0.01f;
    if (argc > 3) {
        hexdia = atof (argv[3]);
        cout << "User supplied hex size: " << hexdia << " mm" << endl;
        if (hexdia < 0.003f) {
            cerr << "Very small hex dia - memory use may be large." << endl;
        }
    }

    try {
        // Read the curves
        ReadCurves r(argv[1]);
        // Create a HexGrid
        morph::HexGrid hg(hexdia, gridspan, 0, morph::HexDomainShape::Boundary);
        // Apply the curves as a boundary
        cout << "Number of hexes before setting boundary: " << hg.num() << endl;
        hg.setBoundary (r.getCorticalPath());

        cout << "HexGrid extent:" << endl;
        cout << "  x range: " << hg.getXmin() << " to " << hg.getXmax() << endl;
        cout << "  y range: " << hg.getXmin(90) << " to " << hg.getXmax(90) << endl;
        cout << "Scaling is " << r.getScale_mmpersvg() << " mm per SVG unit, or "
             << r.getScale_svgpermm() << " units/mm" << endl;
        cout << "Number of hexes within the boundary: " << hg.num() << endl;

        vector<double> fix(3, 0.0);
        vector<double> eye(3, 0.0);
        vector<double> rot(3, 0.0);
        double rhoInit = (double)gridspan;
        morph::Gdisplay disp(960, 900, 0, 0, argv[0], rhoInit, 0.0, 0.0);
        disp.resetDisplay (fix, eye, rot);
        disp.redrawDisplay();

        // plot stuff here.
        array<float,3> cl_boundary_and_in = morph::Tools::getJetColorF (0.9);
        array<float,3> cl_bndryonly = morph::Tools::getJetColorF (0.8);
        array<float,3> cl_domain = morph::Tools::getJetColorF (0.5);
        array<float,3> cl_inside = morph::Tools::getJetColorF (0.15);
        array<float,3> offset = {{0, 0, 0}};
        for (auto h : hg.hexen) {
            if (h.boundaryHex() && h.insideBoundary()) {
                // red is boundary hex AND inside boundary
                disp.drawHex (h.position(), (h.d/2.0f), cl_boundary_and_in);
            } else if (h.boundaryHex()) {
                // orange is boundary ONLY
                disp.drawHex (h.position(), (h.d/2.0f), cl_bndryonly);
            } else if (h.insideBoundary()) {
                // Inside boundary -  blue
                disp.drawHex (h.position(), (h.d/2.0f), cl_inside);
            } else {
                // The domain - greenish
                disp.drawHex (h.position(), offset, (h.d/2.0f), cl_domain);
            }
        }
        disp.redrawDisplay();

        // Draw small hex at boundary centroid
        array<float,3> cl_aa = morph::Tools::getJetColorF (0.98);
        array<float,3> c;
        c[2] = 0;
        c[0] = hg.boundaryCentroid.first;
        c[1] = hg.boundaryCentroid.second;
        disp.drawHex (c, offset, (hg.hexen.begin()->d/2.0f), cl_aa);
        cout << "boundaryCentroid x,y: " << c[0] << "," << c[1] << endl;

        disp.redrawDisplay();

        // red hex at zero
        array<float,3> pos = { { 0, 0, 0} };
        disp.drawHex (pos, 0.05, cl_aa);
        disp.redrawDisplay();

        cout << "press a key(rtn) to exit" << endl;
        int a;
        cin >> a;

        disp.closeDisplay();

    } catch (const exception& e) {
        cerr << "Caught exception reading " << argv[1] << ": " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
