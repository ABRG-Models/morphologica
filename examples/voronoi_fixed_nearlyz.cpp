/*
 * Make a very small Voronoi surface. Used to debug VoronoiVisual
 */
#include <morph/Visual.h>
#include <morph/VoronoiVisual.h>
#include <morph/VectorVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <limits>

// Use key presses to change border_width, so extend morph::Visual in the usual way
struct myvisual final : public morph::Visual<>
{
    myvisual (int width, int height, const std::string& title) : morph::Visual<> (width, height, title) {}
    float border_width = std::numeric_limits<float>::epsilon();
protected:
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == morph::key::up && action == morph::keyaction::press) {
            this->border_width += 0.01f;
        }
        if (key == morph::key::down && action == morph::keyaction::press) {
            this->border_width -= 0.01f;
            if (this->border_width <= 0.0f) { this->border_width = std::numeric_limits<float>::epsilon(); }
        }
    }
};

int main()
{
    int rtn = -1;

    myvisual v(1024, 768, "VoronoiVisual");
    v.coordArrowsInScene (true);

    std::vector<morph::vec<float>> points = {
        {0,0,1},
        {1,0,1},
        {0,1,1},
        {1,1,1},
        {.5,.5,0.5},
    };
    std::vector<float> data = {1,2,3,4,5};

    morph::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = true;
    vorv->debug_edges = true;
    vorv->debug_dataCoords = true;
    vorv->data_z_direction = morph::vec<float>{1,0,1};
    vorv->data_z_direction.renormalize();
    vorv->border_width  = v.border_width;
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto p_vorv = v.addVisualModel (vorv);

    offset[0] -= 0.5f;
    auto vvm = std::make_unique<morph::VectorVisual<float, 3>>(offset);
    v.bindmodel (vvm);
    vvm->thevec = p_vorv->data_z_direction;
    vvm->fixed_colour = true;
    vvm->thickness = 0.03f;
    vvm->single_colour = morph::colour::dodgerblue2;
    vvm->addLabel ("Arrow gives data direction", {-0.8, -0.3, 0}, morph::TextFeatures(0.1f));
    vvm->finalize();
    v.addVisualModel (vvm);

    while (!v.readyToFinish) {
        if (p_vorv->border_width != v.border_width) {
            p_vorv->border_width = v.border_width;
            p_vorv->reinit();
        }
        v.render();
        v.waitevents (0.018);
    }

    return rtn;
}
