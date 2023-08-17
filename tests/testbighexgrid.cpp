/*
 * Test a big hex grid with many hexes. Apply boundary as an ellipse.
 */

#include "morph/Visual.h"
#include "morph/VisualDataModel.h"
#include "morph/HexGridVisual.h"
#include "morph/HexGrid.h"
#include "morph/ReadCurves.h"
#include "morph/tools.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>
#include "morph/Scale.h"
#include "morph/vec.h"

int main()
{
    int rtn = -1;

    morph::Visual v(800,600,"Test window");
    v.zNear = 0.001;

    try {
        morph::HexGrid hg(0.002, 8, 0);
        hg.setEllipticalBoundary (1.6,2);

        std::cout << hg.extent() << std::endl;

        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 1604) { rtn = -1; }

        std::vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*std::sin(10*hg.d_x[hi]); // Range 0->1
        }
        std::cout << "Created " << data.size() << " floats in data" << std::endl;

        morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        auto hgv = std::make_unique<morph::HexGridVisual<float>> (&hg, offset);
        v.bindmodel (hgv);
        hgv->hexVisMode = morph::HexVisMode::Triangles; // Triangles faster to render than the default hexes
        hgv->setScalarData (&data);
        hgv->zScale.setParams (0.1f, 0.0f);
        hgv->finalize();
        v.addVisualModel (hgv);

        v.render();

        while (v.readyToFinish == false) { v.keepOpen(); }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << morph::Tools::getPwd() << std::endl;
        rtn = -1;
    }


    return rtn;
}
