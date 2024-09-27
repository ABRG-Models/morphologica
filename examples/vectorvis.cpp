/*
 * Visualize a single vector
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/VectorVisual.h>
#include <morph/vec.h>
#include <morph/Quaternion.h>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

int main()
{
    morph::Visual v(1024, 768, "morph::VectorVisual");
    v.lightingEffects();

    morph::vec<float> offset = {0,0,0};
    auto vvm = std::make_unique<morph::VectorVisual<float, 3>>(offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1,1};
    vvm->finalize();
    auto ptr = v.addVisualModel (vvm);

    // Also demo quaternion rotation.
    float angle_per_frame = 0.05f;
    // Set up a rotation about the z axis
    morph::Quaternion<float> qr (morph::vec<float>{0,0,1}, angle_per_frame);

    while (!v.readyToFinish) {
        v.render();
        v.wait (0.05);
        ptr->thevec = (qr * ptr->thevec);
        ptr->reinit();
    }

    return 0;
}
