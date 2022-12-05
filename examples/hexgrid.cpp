/*
 * An example morph::Visual scene, containing a HexGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    morph::Visual v(1600, 1000, "morph::HexGridVisual", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked = false;
    // Whole-scene offsetting uses two methods - this one to set the depth at which the object is drawn
    v.setZDefault (-5.0f);
    // ...and this one to set the x/y offset. Try pressing 'z' in the app window to see what the current sceneTrans is
    v.setSceneTransXY (0.0f, 0.0f);
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows = true;
    // The title can be hidden
    v.showTitle = false;
    // The coord arrows can be displayed within the scene (rather than in, say, the corner)
    v.coordArrowsInScene = false;
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("This is a\nmorph::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a HexGrid to show in the scene. Hexes outside the circular boundary will
    // all be discarded.
#ifdef __WIN__ // HexGrid performance is bad on Windows
    morph::HexGrid hg(0.03f, 3.0f, 0.0f);
#else
    morph::HexGrid hg(0.01f, 3.0f, 0.0f);
#endif
    hg.setCircularBoundary (0.6f);
    std::cout << "Number of pixels in grid:" << hg.num() << std::endl;


    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0f);
    for (unsigned int ri=0; ri<hg.num(); ++ri) {
        data[ri] = 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset);
    hgv->setScalarData (&data);
    hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
    hgv->finalize();
    unsigned int gridId = v.addVisualModel (hgv);
    std::cout << "Added HexGridVisual with gridId " << gridId << std::endl;

    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
