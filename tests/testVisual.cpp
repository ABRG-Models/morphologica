#include "Visual.h"
#include "HexGrid.h"
#include "ReadCurves.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

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

        HexGrid hg(0.02, 1, 0, HexDomainShape::Boundary);
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

        // Make some dummy data
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 1.0 * hg.d_x[hi];
        }
        cout << "Created " << data.size() << " floats in data" << endl;

        array<float, 3> offset = { 0.0, 0.0, 0.0 };
        unsigned int gridId = v.addHexGridVisual (&hg, data, offset);
        cout << "Added HexGridVisual with gridId " << gridId << endl;

        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout(2.5);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }


    return rtn;
}
