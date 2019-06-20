#include "display.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <unistd.h>

#include "HexGrid.h"
#include "ReadCurves.h"

using namespace morph;
using namespace std;

int main()
{
    int rtn = 0;
    try {
        string pwd = Tools::getPwd();
        string curvepath = "../tests/trialmod.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "../../tests/trialmod.svg";
        }
        ReadCurves r(curvepath);

        HexGrid hg(0.02, 7, 0, HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 2088) {
            rtn = -1;
        }

        vector<double> fix(3, 0.0);
        vector<double> eye(3, 0.0);
        vector<double> rot(3, 0.0);
        double rhoInit = 1.7;
        morph::Gdisplay disp(800, 600, 0, 0, "A boundary", rhoInit, 0.0, 0.0);
        disp.resetDisplay (fix, eye, rot);
        disp.redrawDisplay();

        // plot stuff here.
        array<float,3> cl_a = morph::Tools::getJetColorF (0.78);
        array<float,3> cl_b = morph::Tools::getJetColorF (0.58);
        array<float,3> offset = {{0, 0, 0}};
        for (auto h : hg.hexen) {
            if (h.boundaryHex) {
                disp.drawHex (h.position(), (h.d/2.0f), cl_a);
            } else {
                disp.drawHex (h.position(), offset, (h.d/2.0f), cl_b);
            }
        }
        disp.redrawDisplay();


#if 0
        // Offset centroid
        hg.offsetCentroid(); // FAILS

        // Redraw
        cl_a = morph::Tools::getJetColorF (0.08);
        cl_b = morph::Tools::getJetColorF (0.28);
        for (auto h : hg.hexen) {
            h.z-=0.1;
            if (h.boundaryHex) {
                disp.drawHex (h.position(), (h.d/2.0f), cl_a);
            } else {
                disp.drawHex (h.position(), offset, (h.d/2.0f), cl_b);
            }
        }
        disp.redrawDisplay();
#endif

        // Draw small hex at boundary centroid
        array<float,3> c;
        c[2] = 0;
        c[0] = hg.boundaryCentroid.first;
        c[1] = hg.boundaryCentroid.second;
        cout << "d/2: " << hg.hexen.begin()->d/4.0f << endl;
        disp.drawHex (c, offset, (hg.hexen.begin()->d/2.0f), cl_a);
        cout << "boundaryCentroid x,y: " << c[0] << "," << c[1] << endl;

        disp.redrawDisplay();

        // red hex at zero
        array<float,3> cl_aa = morph::Tools::getJetColorF (0.98);
        array<float,3> pos = { { 0, 0, 0} };
        disp.drawHex (pos, 0.05, cl_aa);
        disp.redrawDisplay();

        unsigned int sleep_seconds = 3;
        cout << "Sleep " << sleep_seconds << " s before closing display..." << endl;
        while (sleep_seconds--) {
            usleep (1000000); // one second
        }

        disp.closeDisplay();

    } catch (const exception& e) {
        cerr << "Caught exception reading svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }

    return rtn;
}
