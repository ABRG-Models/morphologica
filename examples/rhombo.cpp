#include <morph/Visual.h>
#include <morph/RhomboVisual.h>
#include <morph/vec.h>

int main()
{
    // Create a scene
    morph::Visual v(1024, 768, "A rhombohedron");
    v.showCoordArrows (true); // Please show the coord arrows by default
    v.lightingEffects();

    // Parameters of the model
    morph::vec<float, 3> offset = { 0,  0,  0 };   // a within-scene offset
    morph::vec<float, 3> e1 = { 0.25,  0,  0 };
    morph::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    morph::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    morph::vec<float, 3> colour1 = { 0.35,  0.76,  0.98 };  // RGB colour triplet

    auto rv = std::make_unique<morph::RhomboVisual<>> (offset, e1, e2, e3, colour1);
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);
    v.keepOpen();

    return 0;
}
