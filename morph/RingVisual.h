#pragma once

#include <morph/VisualModel.h>

namespace morph {

    // A simple ring class that uses the primitive VisualModel::computeRing
    template <int glver = morph::gl::version_4_1>
    struct RingVisual : public morph::VisualModel<glver>
    {
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
} // namespace
