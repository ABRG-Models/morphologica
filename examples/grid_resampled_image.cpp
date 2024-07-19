#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>
#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>

int main()
{
    morph::Visual v(1400, 1300, "Demo of Grid showing a resampled image");
    v.setSceneTrans (morph::vec<float,3>({-2.60691f, 1.39885f, -11.1f}));

    morph::vec<float, 2> dx = { 0.02f, 0.02f };
    morph::vec<float, 2> dx2 = { 0.04f, 0.04f };
    // Top left to bottom right order matches image loaded by loadpng and avoids the need for a
    // vec<bool, 2> flip arg to morph::loadpng.
    morph::Grid g1(256U, 65U, dx);
    std::cout << "g1 extents: (xmin,xmax,ymin,ymax): " << g1.extents() << std::endl;

    // Resample onto a lower res grid
    morph::Grid g2(128U, 32U, dx2);
    std::cout << "g2 extents: (xmin,xmax,ymin,ymax): " << g2.extents() << std::endl;

    // Load an image
    std::string fn = "../examples/bike256_65.png";

    morph::vvec<float> image_data; // image loaded BLTR by default
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);

    std::cout << "Image dims: " << dims << std::endl;

    if (dims[0] != 256) {
        std::cerr << "Wrong image width!\n";
        return -1;
    }

    // Resample
    // This allows you to expand or contract the image on the Grid.
    morph::vec<float,2> image_scale = {1.0f, 1.0f};
    // You can shift the image with an offset if necessary. Use the units of the target Grid/HexGrid
    morph::vec<float,2> image_offset = {0.0f, 0.0f};

    morph::vvec<float> img_resampled = g2.resample_image (image_data, dims[0], image_scale, image_offset);

    // Resample to HexGrid too, for good measure
    morph::vec<float, 2> g2_dx = g2.get_dx();
    morph::HexGrid hg(g2_dx[0], g2.width()*2.0f, 0.0f);
    hg.setRectangularBoundary (g2.width(), g2.height()); // Make rectangular boundary same size as g2

    // Here's the HexGrid method that will resample the square pixel grid onto the hex
    // grid Because HexGrids are not naturally rectangular, a scaling of {1,1} does not
    // expand the image to fit the HexGrid. Instead, we scale by the width of g2, which
    // scales the interpreted image (which is interpreted as having width 1.0) to the
    // correct dimensions.
    morph::vec<float,2> hex_image_scale = { g2.width(), g2.width() };
    morph::vvec<float> hex_image_data = hg.resampleImage (image_data, dims[0], hex_image_scale, image_offset);

    // Visualise original with a GridVisual
    auto gv1 = std::make_unique<morph::GridVisual<float>>(&g1, morph::vec<float>({0,0,0}));
    v.bindmodel (gv1);
    gv1->gridVisMode = morph::GridVisMode::RectInterp;
    gv1->setScalarData (&image_data);
    gv1->cm.setType (morph::ColourMapType::GreyscaleInv); // inverse greyscale is good for a monochrome image
    gv1->zScale.setParams (0, 0); // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    gv1->addLabel ("Original", {0, -0.2, 0}, morph::TextFeatures(0.1f));
    gv1->finalize();
    v.addVisualModel (gv1);

    // Visualize resampled
    auto gv2 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>{ 0.0f, -2.0f, 0.0f });
    v.bindmodel (gv2);
    gv2->gridVisMode = morph::GridVisMode::RectInterp;
    gv2->setScalarData (&img_resampled);
    gv2->cm.setType (morph::ColourMapType::GreyscaleInv);
    gv2->zScale.setParams (0, 0);
    gv2->addLabel ("Resampled to coarser Grid", {0, -0.2, 0}, morph::TextFeatures(0.1f));
    gv2->finalize();
    v.addVisualModel (gv2);

    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, morph::vec<float>{ g2.width() / 2.0f, g2.height() / 2.0f - 4.0f , 0.0f });
    v.bindmodel (hgv);
    hgv->setScalarData (&hex_image_data);
    hgv->cm.setType (morph::ColourMapType::GreyscaleInv);
    hgv->zScale.setParams (0, 0);
    hgv->addLabel ("Resampled to HexGrid", {-g2.width() / 2.0f, -0.2f - g2.height() / 2.0f, 0}, morph::TextFeatures(0.1f));
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
