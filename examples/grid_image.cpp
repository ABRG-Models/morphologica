#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>

int main()
{
    morph::Visual v(1600, 1000, "Demo of Grid showing an image");

    morph::vec<float, 2> dx = { 0.02f, 0.02f };
    morph::vec<float, 2> nul = { 0.0f, 0.0f };
    // Top left to bottom right order matches image loaded by loadpng and avoids the need for a
    // vec<bool, 2> flip arg to morph::loadpng.
    morph::Grid g(256, 65, dx, nul, morph::GridDomainWrap::Horizontal,
                  morph::GridOrder::topleft_to_bottomright);


    // Load an image with the help of OpenCV.
    std::string fn = "../examples/bike256_65.png";

    morph::vvec<float> image_data;
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);
    std::cout << "Image dims: " << dims << std::endl;

    // Now visualise with a HexGridVisual
    auto gv = std::make_unique<morph::GridVisual<float>>(&g, morph::vec<float>({0,0,0}));
    v.bindmodel (gv);
    //gv->gridVisMode = morph::GridVisMode::Triangles; // Test
    // Set the image data as the scalar data for the HexGridVisual
    gv->setScalarData (&image_data);
    // The inverse greyscale map is appropriate for a monochrome image
    gv->cm.setType (morph::ColourMapType::GreyscaleInv);
    // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    gv->zScale.setParams (0, 1);

    gv->finalize();
    auto gvp = v.addVisualModel (gv);

    v.render();
    gvp->reinit();

    v.keepOpen();

    return 0;
}
