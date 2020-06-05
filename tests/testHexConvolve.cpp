/*
 * Test convolution of some data defined on a HexGrid.
 */
#include "Visual.h"
#include "VisualDataModel.h"
#include "HexGridVisual.h"
#include "HexGrid.h"
#include "ReadCurves.h"
#include "tools.h"
#include "Random.h"
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

    Visual v(800,600,"Convolution window");
    v.zNear = 0.001;

    try {
        string pwd = Tools::getPwd();
        string curvepath = "./tests/trial.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "./../tests/trial.svg";
        }
        ReadCurves r(curvepath);

        HexGrid hg(0.01, 3, 0, HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        // Populate a vector of floats with data
        vector<float> data (hg.num(), 0.0f);
        //morph::RandNormal<float> rng (0.1f, 0.05f);
        morph::RandUniform<float> rng;
        for (auto& d : data) {
            d = rng.get();
        }

        // Create a circular HexGrid to contain the Gaussian convolution kernel
        HexGrid kernel(0.01, 1, 0, HexDomainShape::Boundary);
        kernel.setCircularBoundary (0.1);
        vector<float> kerneldata (kernel.num(), 0.0f);
        // Once-only parts of the calculation of the Gaussian.
        float sigma = 0.02f;
        float one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
        float two_sigma_sq = 2.0f * sigma * sigma;
        // Gaussian dist. result, and a running sum of the results:
        float gauss = 0;
        float sum = 0;
        for (auto& k : kernel.hexen) {
            // Gaussian profile based on the hex's distance from centre, which is
            // already computed in each Hex as Hex::r
            gauss = (one_over_sigma_root_2_pi * std::exp ( -(k.r*k.r) / two_sigma_sq ));
            kerneldata[k.vi] = gauss;
            sum += gauss;
        }
        // Renormalise
        for (auto& k : kernel.hexen) { kerneldata[k.vi] /= sum; }

        // A vector for the result
        vector<float> convolved (hg.num(), 0.0f);

        // Call the convolution method from HexGrid:
        hg.convolve (kernel, kerneldata, data, convolved);

        // Visualize the 3 maps
        cout << "visual...\n";
        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        unsigned int gridId = v.addVisualModel (new HexGridVisual<float>(v.shaderprog, &hg, offset, &data));
        offset[1] += 0.6f;
        unsigned int gridId1 = v.addVisualModel (new HexGridVisual<float>(v.shaderprog, &kernel, offset, &kerneldata));
        offset[1] -= 0.6f;
        offset[0] += 1.0f;
        unsigned int gridId2 = v.addVisualModel (new HexGridVisual<float>(v.shaderprog, &hg, offset, &convolved));

        // Divide existing scale by 10:
        float newGrad = static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->zScale.getParams(0)/10.0;
        // Set this in a new zscale object:
        Scale<float> zscale;
        zscale.setParams (newGrad, 0);
        // And set it back into the visual model:
        static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->setZScale (zscale);
        static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId2))->setZScale (zscale);

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
