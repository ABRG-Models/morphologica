/*
 * An example morph::Visual scene, containing a HexGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, ?
    morph::Visual v(1600, 1000, "morph::Visual", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
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
    v.showTitle = true;
    // The coord arrows can be displayed within the scene (rather than in, say, the corner)
    v.coordArrowsInScene = false;
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("Each object is derived from morph::VisualModel", {0.005f, -0.02f, 0.0f});
    v.addLabel ("This is a morph::CoordArrows object", {0.03f, -0.23f, 0.0f});
    v.addLabel ("This is a\nmorph::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a HexGrid to show in the scene
    morph::HexGrid hg(0.01, 3, 0, morph::HexDomainShape::Boundary);
    hg.setCircularBoundary (0.3);
    std::cout << "Number of hexes in grid:" << hg.num() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0);
    for (unsigned int hi=0; hi<hg.num(); ++hi) {
        data[hi] = 0.05 + 0.05*std::sin(10*hg.d_x[hi]); // Range 0->1
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::Vector<float, 3> offset = { 0.0, -0.05, 0.0 };
    unsigned int gridId = v.addVisualModel (new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset, &data));
    std::cout << "Added HexGridVisual with gridId " << gridId << std::endl;

    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
