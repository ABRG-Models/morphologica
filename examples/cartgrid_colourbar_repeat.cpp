/*
 * An example morph::Visual scene, containing a CartGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/CartGridVisual.h>
#include <morph/CartGrid.h>
#include <morph/ColourBarVisual.h>

int main()
{
    morph::Visual v(1600, 1000, "morph::CartGridVisual");
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked = false;
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
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
    auto modelp = v.addVisualModel (cgv);

    // Add the colour bar
    morph::vec<float, 3> cboffset = {1.0f, 0.0f, 0.0f};
    auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(cboffset);
    v.bindmodel (cbv);
    cbv->orientation = morph::colourbar_orientation::vertical;
    cbv->tickside = morph::colourbar_tickside::right_or_below;
    cbv->cm = modelp->cm;
    cbv->scale = modelp->colourScale;
    cbv->finalize();
    auto cbvp = v.addVisualModel (cbv);

    v.render();

    unsigned long long loop = 0;
    while (!v.readyToFinish) {

        v.poll();

        v.removeVisualModel (modelp);
        v.removeVisualModel (cbvp);

        offset[0] += 0.01f;
        if (offset[0] > 1.0f) { offset[0] = 0.0f; }

        cgv = std::make_unique<morph::CartGridVisual<float>>(&cg, offset);
        v.bindmodel (cgv);
        cgv->cartVisMode = morph::CartVisMode::RectInterp;
        cgv->setScalarData (&data);
        cgv->cm.setType (morph::ColourMapType::Twilight);
        cgv->finalize();
        modelp = v.addVisualModel (cgv);

        cbv = std::make_unique<morph::ColourBarVisual<float>>(cboffset);
        v.bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->cm = modelp->cm;
        cbv->scale = modelp->colourScale;
        cbv->finalize();
        cbvp = v.addVisualModel (cbv);

        v.render();
        ++loop;
    }

    std::cout << "Removed and re-added " << loop << " cartgrids\n";

    return 0;
}
