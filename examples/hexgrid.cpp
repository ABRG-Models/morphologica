/*
 * An example morph::Visual scene, containing a HexGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    // Contructor args are width, height, title
    morph::Visual<morph::gl::version_4_1> v(1600, 1000, "morph::HexGridVisual");
    // You can set a field of view (in degrees)
    v.fov = 15;
    // set the x/y offset. Try pressing 'z' in the app window to see what the current sceneTrans is
    v.setSceneTransXY (0.0f, 0.0f);
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows (true);
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    //v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("This is a\nmorph::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a HexGrid to show in the scene. Hexes outside the circular boundary will
    // all be discarded.
    morph::HexGrid hg(0.01f, 3.0f, 0.0f);
    hg.setCircularBoundary (0.6f);
    std::cout << "Number of pixels in grid:" << hg.num() << std::endl;


    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0f);
    for (unsigned int ri=0; ri<hg.num(); ++ri) {
        data[ri] = 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<morph::HexGridVisual<float,morph::gl::version_4_1>>(&hg, offset);
    v.bindmodel (hgv);
    hgv->cm.setType (morph::ColourMapType::Ice);
    hgv->setScalarData (&data);
    hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
    hgv->finalize();

    if (v.checkContext() == true) {
        std::cout << "I have the context after hgv->finalize()\n";
    } else {
        std::cout << "I don't have the context after hgv->finalize()\n";
    }

    v.addVisualModel (hgv);

    if (v.checkContext() == true) {
        std::cout << "I have the context after addVisualModel\n";
    } else {
        std::cout << "I don't have the context after addVisualModel()\n";
    }

    v.keepOpen();

    if (v.checkContext() == true) {
        std::cout << "I have the context after user requested exit\n";
    } else {
        std::cout << "I don't have the context after user requested exit\n";
    }

    return 0;
}
