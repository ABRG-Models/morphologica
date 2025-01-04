/*
 * Draw a line (made of a few segments)
 */

#include <morph/Visual.h>
#include <morph/VisualModel.h>

namespace morph {

    // A test VisualModel which draws some lines
    template<int glver = morph::gl::version_4_1>
    struct LinestestVisual : public VisualModel<glver>
    {
        LinestestVisual() : morph::VisualModel<glver>() {}

        void initializeVertices()
        {
            float lth = 0.1f; // line thickness
            morph::vvec<vec<float>> p(5);
#ifdef LINE_Z
            // Draw 3 lines with a different angle and one extra line
            p[0] = {-0.5, -0.5, 0};
            p[1] = {0,    0,   0};
            p[2] = {1,    0,   0};
            p[3] = {1.5,  0.5, 0};
            p[4] = {2,    0,   0};

            this->computeFlatLine (p[0],        p[1],
                                   p[0],        p[2],
                                   this->uz, morph::colour::black, lth);
            this->computeFlatLine (p[1],        p[2],
                                   p[0],        p[3],
                                   this->uz, morph::colour::crimson, lth);
            this->computeFlatLine (p[2],        p[3],
                                   p[1],        p[4],
                                   this->uz, morph::colour::goldenrod1, lth);
            this->computeFlatLine (p[3],        p[4],
                                   p[2],        p[4],
                                   this->uz, morph::colour::dodgerblue2, lth);
#else
            p[0] = {0,    0,   0};
            p[1] = {1,    0,   1};
            p[2] = {2,    0,   0};

            this->computeFlatLine (p[0],        p[1],
                                   p[0],        p[2],
                                   this->uy, morph::colour::black, lth);
            this->computeFlatLine (p[1],        p[2],
                                   p[0],        p[2],
                                   this->uy, morph::colour::crimson, lth);

#endif
        }
    };

} // morph

// Main program simply creates a Visual and places a LinestestVisual object in it.
int main()
{
    morph::Visual v(1024, 768, "Lines");
    v.coordArrowsInScene = true;
    auto vm = std::make_unique<morph::LinestestVisual<>>();
    v.bindmodel (vm);
    vm->finalize();
    v.addVisualModel (vm);
    v.keepOpen();
    return 0;
}
