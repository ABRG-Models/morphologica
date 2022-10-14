/*
 * Test convolution of some data defined on a CartGrid (using CartGrid::convolve)
 */
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/CartGridVisual.h>
#include <morph/CartGrid.h>
#include <morph/ReadCurves.h>
#include <morph/Random.h>
#include <morph/Scale.h>
#include <morph/Vector.h>

int main()
{
    int rtn = 0;

    // This'll be a 256x64 grid. This constructor creates a 'non-centred' cartgrid.
    morph::CartGrid cg(0.01, 0.01, 0.0f, 0.0f, 256*0.01-0.01, 64*0.01-0.01);
    cg.setBoundaryOnOuterEdge();

    // Populate a vector of floats with data
    morph::vVector<float> data (cg.num());
    data.randomize();
    float nonconvolvedSum = data.sum();

    // Create a small CartGrid to contain the convolution kernel
    morph::CartGrid kernel (0.01, 0.01, 0.05, 0.05);

    kernel.setBoundaryOnOuterEdge();
    morph::vVector<float> kdata(kernel.num());

    // Put a Gaussian in the kernel
    // Once-only parts of the calculation of the Gaussian.
    float sigma = 0.025f;
    float one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
    float two_sigma_sq = 2.0f * sigma * sigma;
    // Gaussian dist. result, and a running sum of the results:
    float gauss = 0;
    float sum = 0;
    for (auto& k : kernel.rects) {
        // Gaussian profile based on the rects's distance from centre
        float d_x = k.x;
        float d_y = k.y;
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(d_x * d_x + d_y * d_y) / two_sigma_sq ));
        kdata[k.vi] = gauss;
        sum += gauss;
    }
    // Renormalise
    for (auto& k : kernel.rects) { kdata[k.vi] /= sum; }

    // A vector for the result
    morph::vVector<float> convolved (cg.num(), 0.0f);

    // Call the convolution method from HexGrid:
    cg.convolve (kernel, kdata, data, convolved);

    float convolvedSum = 0.0f;
    for (float& d : convolved) { convolvedSum += d; }

    std::cout << "Unconvolved sum: " << nonconvolvedSum << ", convolved sum: " << convolvedSum << "\n";

    // Visualize the 3 maps
    morph::Visual v(800,600,"Convolution window");

    morph::Vector<float, 3> offset = { 0.0f, 0.0f, 0.0f };
    morph::CartGridVisual<float>* cgv = new morph::CartGridVisual<float>(v.shaderprog, v.tshaderprog, &cg, offset);
    cgv->cartVisMode = morph::CartVisMode::RectInterp;
    cgv->setScalarData (&data);
    cgv->cm.setType (morph::ColourMapType::GreyscaleInv);
    cgv->zScale.setParams (0, 0);
    cgv->addLabel(std::string("Original"), morph::Vector<float, 3>({0.0f,-0.13f,0.0f}),
                  morph::colour::black, morph::VisualFont::DVSans, 0.1f, 48);
    cgv->finalize();
    v.addVisualModel (cgv);

    offset = { 0.0f, -0.3f, 0.0f };
    morph::CartGridVisual<float>* cgvk = new morph::CartGridVisual<float>(v.shaderprog, v.tshaderprog, &kernel, offset);
    cgvk->cartVisMode = morph::CartVisMode::RectInterp;
    cgvk->setScalarData (&kdata);
    cgvk->cm.setType (morph::ColourMapType::GreyscaleInv);
    cgvk->zScale.setParams (0, 0);
    cgvk->addLabel(std::string("Kernel"), morph::Vector<float, 3>({0.0f,-0.13f,0.0f}),
                  morph::colour::black, morph::VisualFont::DVSans, 0.1f, 48);
    cgvk->finalize();
    v.addVisualModel (cgvk);

    offset = { 0.0f, -1.3f, 0.0f };
    morph::CartGridVisual<float>* cgvr = new morph::CartGridVisual<float>(v.shaderprog, v.tshaderprog, &cg, offset);
    cgvr->cartVisMode = morph::CartVisMode::RectInterp;
    cgvr->setScalarData (&convolved);
    cgvr->cm.setType (morph::ColourMapType::GreyscaleInv);
    cgvr->zScale.setParams (0, 0);
    cgvr->addLabel(std::string("Convolved"), morph::Vector<float, 3>({0.0f,-0.13f,0.0f}),
                  morph::colour::black, morph::VisualFont::DVSans, 0.1f, 48);
    cgvr->finalize();
    v.addVisualModel (cgvr);

    v.keepOpen();

    return rtn;
}
