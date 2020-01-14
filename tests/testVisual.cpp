#include "Visual.h"
#include "HexGrid.h"
#include "ReadCurves.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
using morph::Visual;
using morph::HexGrid;
using morph::Tools;
using morph::HexDomainShape;
using morph::ReadCurves;

int main()
{
    int rtn = -1;

    Visual v(800,600,"Test window");

    try {
        string pwd = Tools::getPwd();
        string curvepath = "./tests/trial.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "./../tests/trial.svg";
        }
        ReadCurves r(curvepath);

        HexGrid hg(0.01, 3, 0, HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;

        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 1604) {
            rtn = -1;
        }

        vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*std::sin(10*hg.d_x[hi]); // Range 0->1
        }
        cout << "Created " << data.size() << " floats in data" << endl;

        array<float, 3> offset = { 0.0, 0.0, 0.0 };
        array<float, 4> scale = { 0.1, 0.0, 1.0, 0.0};
        unsigned int gridId = v.addHexGridVisual (&hg, offset, data, scale);
        cout << "Added HexGridVisual with gridId " << gridId << endl;
        v.render();

        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }


    return rtn;
}
