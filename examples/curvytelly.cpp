/*
 * Demonstrate the CurvyTellyVisual by showing an image
 */
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Grid.h>
#include <morph/quaternion.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/CurvyTellyVisual.h>

int main()
{
    morph::Visual<> v(1600, 1000, "CurvyTellyVisual showing an image");

    // Load an image (you have to run the program from ./build/
    morph::vvec<float> image_data;
    morph::vec<unsigned int, 2> dims = morph::loadpng ("../examples/horsehead_reduced.png", image_data);

    // CurvyTellyVisual needs a Grid as an underlying data structure
    morph::vec<float, 2> grid_spacing = { 0.1f, 0.01f };
    morph::Grid grid(dims[0], dims[1], grid_spacing);

    morph::vec<float> offset = { 0, 0, 0 };
    auto ctv = std::make_unique<morph::CurvyTellyVisual<float>>(&grid, offset);
    v.bindmodel (ctv);
    ctv->setScalarData (&image_data);
    ctv->cm.setType (morph::ColourMapType::Magma);
    ctv->radius = 10.0f;     // The radius of curvature of the telly
    ctv->centroidize = true; // Ensures the centre of the VisualModel is the 'middle of the screen' (it's centroid)
    ctv->angle_to_subtend = morph::mathconst<float>::pi_over_3; // 2pi is default
    ctv->frame_width = 0.1f; // Show a frame around the image
    ctv->frame_clr = morph::colour::navy;
    ctv->finalize();
    v.addVisualModel (ctv);

    // To make this view in the correct orientation as if it were a TV, we have to rotate & translate the scene.
    v.setSceneTrans (morph::vec<float,3>{ float{0}, float{0}, float{-14} });
    v.setSceneRotation (morph::quaternion<float>{ float{-0.5}, float{0.5}, float{-0.5}, float{-0.5} });

    v.keepOpen();

    return 0;
}
