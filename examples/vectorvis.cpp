/*
 * Visualize a single vector
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/VectorVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

int main()
{
    morph::Visual v(1024, 768, "morph::VectorVisual");
    v.lightingEffects();

    morph::vec<float> offset = {0,0,0};
    auto vvm = std::make_unique<morph::VectorVisual<float, 2>>(offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1};
    vvm->finalize();
    auto ptr = v.addVisualModel (vvm);

    float f = 0.0f;
    while (!v.readyToFinish) {
        v.render();
        v.wait (0.05);
        f += 0.05f;
        ptr->thevec[0] = 2.5 * std::sin(f);
        ptr->thevec[1] = 2.5 * std::cos(f);
        ptr->reinit();
    }

    return 0;
}
