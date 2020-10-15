#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <iostream>
#include <morph/display.h>
#include <unistd.h>

using namespace morph;
using namespace std;

int main()
{
    if (XOpenDisplay(NULL) == (Display*)0) {
        cout << "No display, can't run test. Return 0\n";
        return 0;
    }

    int rtn = 0;
    unsigned int hexnum = 0;

    cout << "Start " << Tools::timeNow() << endl;
    // Create and then write a HexGrid
    try {
        string pwd = Tools::getPwd();
        string curvepath = "../../tests/trial.svg";

        ReadCurves r(curvepath);

        HexGrid hg(0.01, 3, 0, HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;

        hexnum = hg.num();
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        hg.save("../trialhexgrid.h5");

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }
    cout << "Generated " << Tools::timeNow() << endl;
    // Now read it back
    try {
        HexGrid hg("../trialhexgrid.h5");

        cout << "Read " << Tools::timeNow() << endl;

        // Make sure read-in grid has same number of hexes as the generated one.
        if (hexnum != hg.num()) {
            rtn = -1;
        }

        vector<double> fix(3, 0.0);
        vector<double> eye(3, 0.0);
        vector<double> rot(3, 0.0);
        double rhoInit = 1.7;
        morph::Gdisplay disp(800, 600, 0, 0, "A boundary", rhoInit, 0.0, 0.0);
        disp.resetDisplay (fix, eye, rot);

        // plot stuff here.
        array<float,3> cl_a = morph::ColourMap<float>::jetcolour (0.78);
        array<float,3> cl_b = morph::ColourMap<float>::jetcolour (0.58);
        array<float,3> offset = {{0, 0, 0}};
        for (auto h : hg.hexen) {
            if (h.boundaryHex()) {
                disp.drawHex (h.position(), (h.d/2.0f), cl_a);
            } else {
                disp.drawHex (h.position(), offset, (h.d/2.0f), cl_b);
            }
        }

        usleep (100000);
        disp.redrawDisplay();

        unsigned int sleep_seconds = 1;
        cout << "Sleep " << sleep_seconds << " s before closing display..." << endl;
        while (sleep_seconds--) {
            usleep (1000000); // one second
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }
    return rtn;
}
