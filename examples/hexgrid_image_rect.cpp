/*
 * An example morph::Visual scene, containing a HexGrid, onto which is sampled an image. In this
 * case the image is rectangular.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    morph::Visual v(1600, 1000, "Demo of HexGrid::resampleImage");

    morph::HexGrid hg(0.01f, 3.0f, 0.0f);
    hg.setRectangularBoundary (2.0f, 0.5f);

    // Load a rectangular image with the help of morph::loadpng().
    std::string fn = "../examples/bike256_65.png";
    morph::vvec<float> image_data;
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);
    std::cout << "Loaded image with dims: " << dims << std::endl;

    // This controls how large the photo will be on the HexGrid
    morph::vec<float,2> image_scale = {2.0f, 2.0f};
    // You can shift the photo with an offset if necessary
    morph::vec<float,2> image_offset = {0.0f, 0.0f};

    // Here's the HexGrid method that will resample the square pixel grid onto the hex grid
    morph::vvec<float> hex_image_data = hg.resampleImage (image_data, dims[0], image_scale, image_offset);

    // Now visualise with a HexGridVisual
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, morph::vec<float>({0,0,0}));
    v.bindmodel (hgv);

    // Set the image data as the scalar data for the HexGridVisual
    hgv->setScalarData (&hex_image_data);
    // The inverse greyscale map is appropriate for a monochrome image
    hgv->cm.setType (morph::ColourMapType::GreyscaleInv);
    // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    hgv->zScale.setParams (0, 1);

    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
