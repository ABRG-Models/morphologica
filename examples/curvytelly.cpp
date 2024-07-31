/*
 * Demonstrate the CurvyTellyVisual by showing an image
 */
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Grid.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/CurvyTellyVisual.h>

int main()
{
    morph::Visual<> v(1600, 1000, "CurvyTellyVisual showing an image");

    std::string fn = "../examples/horsehead_reduced.png";
    //morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data_tlbr, morph::vec<bool, 2>{false,false});
    morph::vvec<float> image_data;
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);
    std::cout << "Image dims: " << dims << std::endl;

    // Make a Grid to display the stripes.
    // In x, make it as many wide as there will be facets on the tube.
    // In y, make it as many long as you want there to be stripes
    // Choose x/y spacing to suit required circumference and length
    morph::vec<float, 2> grid_spacing = { 0.1f, 0.01f };
    morph::Grid grid(dims[0], dims[1], grid_spacing);
    std::cout << "Number of pixels in grid:" << grid.n << std::endl;

    morph::vec<float> offset = { 0, 0, 0 };
    auto ctv = std::make_unique<morph::CurvyTellyVisual<float>>(&grid, offset);
    v.bindmodel (ctv);
    ctv->setScalarData (&image_data);
    ctv->radius = 10.0f;
    ctv->angle_to_subtend = morph::mathconst<float>::pi_over_3; // 2pi is default
    //ctv->tb_frames = false;
    //ctv->lr_frames = false;
    //ctv->cm.setType (morph::ColourMapType::Greyscale);
    ctv->finalize();
    v.addVisualModel (ctv);

    v.keepOpen();

    return 0;
}
