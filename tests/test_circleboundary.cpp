#include <utility>
#include <iostream>
#include <cmath>

#include "morph/Visual.h"
#include "morph/VisualDataModel.h"
#include "morph/HexGridVisual.h"
#include "morph/ColourMap.h"
#include "morph/tools.h"
#include "morph/HexGrid.h"
#include "morph/ReadCurves.h"
#include "morph/Scale.h"
#include "morph/vec.h"

int main (int argc, char** argv)
{
    int rtn = 0;

    morph::Visual v(1024, 768, "Ellipse");
    v.zNear = 0.001;
    v.showCoordArrows = false;

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        if (a1.size() > 0) { holdVis = true; }
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    try {
        morph::HexGrid hg(0.01, 3, 0);
        hg.setCircularBoundary (1);

        std::cout << hg.extent() << std::endl;
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 36624) { rtn = -1; }

        std::vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*sin(hg.d_x[hi]); // Range 0->1
        }
        std::cout << "Created " << data.size() << " floats in data" << std::endl;

        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        auto hgv = std::make_unique<morph::HexGridVisual<float>> (v.shaders, &hg, offset);
        hgv->setScalarData (&data);
        hgv->cm.setType (morph::ColourMapType::Rainbow);
        hgv->zScale.setParams(0,0);
        hgv->finalize();
        v.addVisualModel (hgv);
        v.render();

        if (holdVis == true) { v.keepOpen(); }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << morph::Tools::getPwd() << std::endl;
        rtn = -1;
    }

    return rtn;
}
