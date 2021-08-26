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

#include <iostream>
//#define DEBUG 1
#define DBGSTREAM std::cout
#include <morph/MorphDbg.h>

#include <morph/ReadCurves.h>
#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <morph/HexGrid.h>
#include <utility>
#include <vector>
#include <fstream>
#include <math.h>
#include <morph/BezCoord.h>

#include <morph/Visual.h>
#include <morph/HexGridVisual.h>

using namespace std;
using morph::ReadCurves;
using morph::BezCoord;
using std::vector;

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

        // Display with morph::Visual
        morph::Visual v(1600, 1000, "Your SVG defined boundary");
        v.lightingEffects();
        morph::Vector<float, 3> offset = { 0.0f, -0.0f, 0.0f };
        morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset);
        // Set up data for the HexGridVisual and colour hexes according to their state as being boundary/inside/domain, etc
        vector<float> colours (hg.num(), 0.0f);
        static constexpr float cl_boundary_and_in = 0.9f;
        static constexpr float cl_bndryonly = 0.8f;
        static constexpr float cl_domain = 0.5f;
        static constexpr float cl_inside = 0.15f;
        for (auto h : hg.hexen) {
            if (h.boundaryHex() && h.insideBoundary()) {
                // red is boundary hex AND inside boundary
                colours[h.vi] = cl_boundary_and_in;
            } else if (h.boundaryHex()) {
                // orange is boundary ONLY
                colours[h.vi] = cl_bndryonly;
            } else if (h.insideBoundary()) {
                // Inside boundary -  blue
                colours[h.vi] = cl_inside;
            } else {
                // The domain - greenish
                colours[h.vi] = cl_domain;
            }
        }
        hgv->cm.setType (morph::ColourMapType::Jet);
        hgv->setScalarData (&colours);
        hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
        hgv->finalize();
        v.addVisualModel (hgv);
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading " << argv[1] << ": " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
