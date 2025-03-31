/*
 * An example morph::Visual scene, containing a HexGrid. This one shows you how to place
 * unicode characters on your coordinate arrows.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

#include <morph/unicode.h>
namespace uc =  morph::unicode;

// Derive Visual to modify the coordinate arrows object in the constructor.
class MyVisual : public morph::Visual<>
{
public:
    MyVisual (int width, int height, const std::string& title)
        : morph::Visual<> (width, height, title)
    {
        this->backgroundWhite();
        this->coordArrows->clear();
        this->coordArrows->x_label = uc::toUtf8 (uc::theta);
        this->coordArrows->y_label = std::string("d") + uc::toUtf8 (uc::beta);
        this->coordArrows->z_label = "F";
        this->coordArrows->initAxisLabels();
        this->coordArrows->reinit();
    }
};

int main()
{
    // Contructor args are width, height, title
    MyVisual v(1600, 1000, "morph::HexGridVisual");
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked = false;
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows (true);
    // The title can be hidden
    v.showTitle = false;
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
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
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, offset);
    v.bindmodel (hgv);
    hgv->setScalarData (&data);
    hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
    hgv->finalize();
    v.addVisualModel (hgv);

    while (v.readyToFinish == false) {
        v.waitevents (0.018);
        v.render();
    }

    return 0;
}
