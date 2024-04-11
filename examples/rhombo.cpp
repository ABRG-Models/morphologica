#include <morph/Visual.h>
#include <morph/RhomboVisual.h>
#include <morph/vec.h>

int main()
{
    // Create a scene
    morph::Visual v(1024, 768, "A rhombohedron");
    v.showCoordArrows = true; // Please show the coord arrows by default
    v.coordArrowsInScene = true;
    v.lightingEffects();

    // Parameters of the model
    morph::vec<float, 3> offset = { -1,  0,  0 };   // a within-scene offset
    morph::vec<float, 3> e1 = { 0.25,  0,  0 };
    morph::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    morph::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    morph::vec<float, 3> colour1 = { 0.35,  0.76,  0.98 };  // RGB colour triplet
    morph::vec<float, 3> colour2 = { 0.75,  0.16,  0.0 };
    morph::vec<float, 3> colour3 = { 0.1,  0.86,  0.2 };

    auto rv = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour1);
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 1, 0, 0 };
    auto rv2 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour2);
    v.bindmodel (rv2);
    rv2->finalize();
    v.addVisualModel (rv2);

    offset = { 0, 1, 0 };
    auto rv3 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, (colour1+colour2)/2);
    v.bindmodel (rv3);
    rv3->finalize();
    v.addVisualModel (rv3);

    offset = { 0, 0, 1 };
    auto rv4 = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour3);
    v.bindmodel (rv4);
    rv4->finalize();
    v.addVisualModel (rv4);


    v.keepOpen();

    return 0;
}
