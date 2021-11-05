#include <utility>
#include <iostream>
#include <cmath>

#include "morph/Visual.h"
using morph::Visual;
#include "morph/VisualDataModel.h"
using morph::VisualDataModel;
#include "morph/HexGridVisual.h"
using morph::HexGridVisual;
#include "morph/ColourMap.h"
using morph::ColourMapType;
#include "morph/tools.h"
using morph::Tools;
#include "morph/HexGrid.h"
using morph::HexGrid;
using morph::HexDomainShape;
#include "morph/ReadCurves.h"
#include "morph/Vector.h"
using morph::Vector;

using namespace std;

int main (int argc, char** argv)
{
    int rtn = 0;

    Visual v(1024, 768, "Ellipse");
    v.zNear = 0.001;
    v.showCoordArrows = false;

    bool holdVis = false;
    if (argc > 1) {
        string a1(argv[1]);
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << endl;

    try {
        HexGrid hg(0.01, 3, 0, HexDomainShape::Boundary);
        hg.setEllipticalBoundary (1, 0.7);

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 25717) {
            rtn = -1;
        }

        vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*sin(hg.d_x[hi]); // Range 0->1
        }
        cout << "Created " << data.size() << " floats in data" << endl;

        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        unsigned int id = v.addVisualModel (new HexGridVisual<float> (v.shaderprog, v.tshaderprog, &hg, offset, &data, ColourMapType::Magma));
        // Problem with doing it at this point is that the HexGrid was already initialised...
        static_cast<VisualDataModel<float>*>(v.getVisualModel(id))->zScale.setParams (0.0, 0.0);
        // ...so reinit
        static_cast<VisualDataModel<float>*>(v.getVisualModel(id))->reinit();
        v.render();

        if (holdVis == true) {
            while (v.readyToFinish == false) {
                glfwWaitEventsTimeout (0.018);
                v.render();
            }
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }

    return rtn;
}
