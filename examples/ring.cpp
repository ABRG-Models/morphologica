/*
 * Visualize a ring with VisualModel::computeRing
 */
#include <morph/Visual.h>
#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/ColourMap.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

// A simple ring class
template <int glver = morph::gl::version_4_1>
class RingVisual : public morph::VisualModel<glver>
{
public:
    RingVisual(const morph::vec<float> _offset) : morph::VisualModel<glver>::VisualModel (_offset) {}

    void initializeVertices()
    {
        this->computeRing (this->locn, this->clr, this->radius, this->thickness, this->segments);
    }

    std::array<float, 3> clr = morph::colour::goldenrod;
    morph::vec<float, 3> locn = { 0.0f };
    float radius = 1.0f;
    float thickness = 0.2f;
    int segments = 80;
};

int main()
{
    morph::Visual v(1024, 768, "A ring");
    v.lightingEffects(true);
    morph::ColourMap<float> cmap;
    morph::vec<int, 6> segs = {3, 4, 6, 8, 12, 24};
    morph::vec<float, 3> offset = { -6.0f, 0.0f, 0.0f };
    for (int i = 0; i < 6; ++i) {
        auto rvm = std::make_unique<RingVisual<>> (offset);
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
