/*
 * An example morph::Visual scene, containing a CartGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/CartGridVisual.h>
#include <morph/CartGrid.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, ?
    morph::Visual v(1600, 1000, "morph::CartGridVisual", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked = false;
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows = false;
    // The title can be hidden
    v.showTitle = false;
    // The coord arrows can be displayed within the scene (rather than in, say, the corner)
    v.coordArrowsInScene = false;
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("This is a\nmorph::CartGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a HexGrid to show in the scene
    morph::CartGrid cg(0.01, 0.01, 1, 1);
    std::cout << "Number of pixels in grid:" << cg.num() << std::endl;

    // *NB* This call (or any other 'set boundary' call) is essential, as it sets up the
    // d_ vectors in the CartGrid. Without it, the CartGrid will be unusable!
    cg.setBoundaryOnOuterEdge();

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(cg.num(), 0.0);
    for (unsigned int ri=0; ri<cg.num(); ++ri) {
        //std::cout << cg.d_x.size() << std::endl;
        //std::cout << "cg.d_x["<<ri<<"]=" << cg.d_x[ri] << std::endl;
        data[ri] = 0.05f + 0.05f*std::sin(20.0f*cg.d_x[ri]) * std::sin(10.0f*cg.d_y[ri]) ; // Range 0->1
    }

    // Add a CartGridVisual to display the CartGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto cgv = std::make_unique<morph::CartGridVisual<float>>(&cg, offset);
    v.bindmodel (cgv);
    cgv->cartVisMode = morph::CartVisMode::RectInterp;
    cgv->setScalarData (&data);
    cgv->cm.setType (morph::ColourMapType::Twilight);
    cgv->finalize();
    v.addVisualModel (cgv);

    v.keepOpen();

    return 0;
}
