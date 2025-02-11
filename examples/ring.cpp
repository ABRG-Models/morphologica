/*
 * Visualize a ring with morph::RingVisual
 */
#include <morph/Visual.h>
#include <morph/RingVisual.h>
#include <morph/vec.h>
#include <morph/ColourMap.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

int main()
{
    morph::Visual v(1024, 768, "A ring");
    v.lightingEffects(true);
    morph::ColourMap<float> cmap;
    morph::vec<int, 6> segs = {3, 4, 6, 8, 12, 24};
    morph::vec<float, 3> offset = { -6.0f, 0.0f, 0.0f };
    for (int i = 0; i < 6; ++i) {
        auto rvm = std::make_unique<morph::RingVisual<>> (offset);
        v.bindmodel (rvm);
        rvm->clr = cmap.convert (i/6.0f);
        rvm->segments = segs[i];
        rvm->finalize();
        v.addVisualModel (rvm);
        offset[0] += 2.3f;
    }
    v.keepOpen();

    return 0;
}
