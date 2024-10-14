/*
 * Make a colourbar tester using a morph::Grid/GridVisual
 *
 * This shows a min to max gradient of a ColourMap, with a decaying sine wave added to
 * the signal. Poor colour maps like Jet show structure in the features that is not part
 * of the data.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>

struct myvisual final : public morph::Visual<>
{
    myvisual (int width, int height, const std::string& title) : morph::Visual<> (width, height, title) {}
    morph::ColourMapType curr_map_type = morph::ColourMapType::Plasma;
protected:
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == morph::key::d && action == morph::keyaction::press) {
            std::cout << "Next map. Change from " << morph::ColourMap<float>::colourMapTypeToStr (this->curr_map_type) << " to ";
            ++this->curr_map_type;
            std::cout << morph::ColourMap<float>::colourMapTypeToStr (this->curr_map_type) << std::endl;
        }
        if (key == morph::key::a && action == morph::keyaction::press) { --this->curr_map_type; }
        if (key == morph::key::h && action == morph::keyaction::press) { std::cout << "left/right switch maps\n"; }
    }
};

int main()
{
    myvisual v(1600, 1000, "Colourbar perceptual uniformity test");

    // Create a grid for the colourmaps
    constexpr unsigned int Nside_w = 512;
    constexpr unsigned int Nside_h = 256;
    constexpr float barw = 2.56f;
    constexpr float barh = 0.5f;
    constexpr morph::vec<float, 2> grid_spacing = {barw/static_cast<float>(Nside_w), barh/static_cast<float>(Nside_h)};
    morph::Grid grid(Nside_w, Nside_h, grid_spacing);

    // Our data is a ramp with a sine wave embossed on it
    std::vector<float> data(grid.n, 0.0);
    for (unsigned int ri=0; ri<grid.n; ++ri) {
        auto x = grid[ri][0];
        auto y = grid[ri][1];
        data[ri] = x / grid.width() + 0.1f * (y / grid.height()) * (y / grid.height()) * std::sin (120.0f * x);
    }

    morph::vec<float, 3> offset = { -0.5f * grid.width(), -0.5f * grid.height(), 0.0f };
    auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setScalarData (&data);
    gv->cm.setType (v.curr_map_type);
    gv->zScale.setParams (0, 0);
    gv->addLabel (gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish == false) {
        v.render();
        v.waitevents (0.017);
        if (gvp->cm.getType() != v.curr_map_type) {
            // Update
            v.removeVisualModel (gvp);
            gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
            v.bindmodel (gv);
            gv->gridVisMode = morph::GridVisMode::Triangles;
            gv->setScalarData (&data);
            gv->cm.setType (v.curr_map_type);
            gv->zScale.setParams (0, 0);
            gv->addLabel (gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05f));
            gv->finalize();
            gvp = v.addVisualModel (gv);
        }
    }

    return 0;
}
