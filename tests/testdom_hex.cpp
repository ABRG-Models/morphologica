#include "display.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <unistd.h>

#include "HexGrid.h"
#include "ReadCurves.h"

using namespace std;

int main()
{
    int rtn = 0;
    try {
        string pwd = morph::Tools::getPwd();
        string curvepath = "../tests/trialmod.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "../../tests/trialmod.svg";
        }
        morph::ReadCurves r(curvepath);

        morph::HexGrid hg(0.01, 1.2, 0, morph::HexDomainShape::Hexagon);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 11347) {
            rtn = -1;
        }

        vector<double> fix(3, 0.0);
        vector<double> eye(3, 0.0);
        vector<double> rot(3, 0.0);
        double rhoInit = 1.7;
        morph::Gdisplay disp(960, 700, 0, 0, "A boundary", rhoInit, 0.0, 0.0);
        disp.resetDisplay (fix, eye, rot);

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

        // Draw small hex at boundary centroid
        array<float,3> cl_aa = morph::Tools::getJetColorF (0.98);
        array<float,3> c;
        c[2] = 0;
        c[0] = hg.boundaryCentroid.first;
        c[1] = hg.boundaryCentroid.second;
        cout << "d/2: " << hg.hexen.begin()->d/4.0f << endl;
        disp.drawHex (c, offset, (hg.hexen.begin()->d/2.0f), cl_aa);
        cout << "boundaryCentroid x,y: " << c[0] << "," << c[1] << endl;

        // red hex at zero
        array<float,3> pos = { { 0, 0, 0} };
        disp.drawHex (pos, 0.05, cl_aa);

        usleep (100000);
        disp.redrawDisplay();

        unsigned int sleep_seconds = 1;
        cout << "Sleep " << sleep_seconds << " s before closing display..." << endl;
        while (sleep_seconds--) {
            usleep (1000000); // one second
        }

        disp.closeDisplay();

    } catch (const exception& e) {
        cerr << "Caught exception reading svg: " << e.what() << endl;
        cerr << "Current working directory: " << morph::Tools::getPwd() << endl;
        rtn = -1;
    }

    return rtn;
}
