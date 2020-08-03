/*
 * Test a big hex grid with many hexes. Apply boundary as an ellipse.
 */

#include "Visual.h"
#include "VisualDataModel.h"
#include "HexGridVisual.h"
#include "HexGrid.h"
#include "ReadCurves.h"
#include "tools.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>
#include "Scale.h"
#include "Vector.h"

using namespace std;
using morph::Visual;
using morph::VisualDataModel;
using morph::HexGrid;
using morph::HexGridVisual;
using morph::Tools;
using morph::HexDomainShape;
using morph::ReadCurves;
using morph::Scale;
using morph::Vector;

int main()
{
    int rtn = -1;

    Visual v(800,600,"Test window");
    v.zNear = 0.001;

    try {
        HexGrid hg(0.002, 8, 0, HexDomainShape::Boundary);
        hg.setEllipticalBoundary (1.6,2);

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

        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        unsigned int gridId = v.addVisualModel (new HexGridVisual<float>(v.shaderprog, &hg, offset, &data));
        cout << "Added HexGridVisual with gridId " << gridId << endl;

        // Divide existing scale by 10:
        float newGrad = static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->zScale.getParams(0)/10.0;
        // Set this in a new zscale object:
        Scale<float> zscale;
        zscale.setParams (newGrad, 0);
        // And set it back into the visual model:
        static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->setZScale (zscale);

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
