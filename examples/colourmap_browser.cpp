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

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>
#include <morph/CyclicColourVisual.h>

struct myvisual final : public morph::Visual<>
{
    myvisual (int width, int height, const std::string& title) : morph::Visual<> (width, height, title) {}
    morph::ColourMapType curr_map_type = morph::ColourMapType::Plasma;
    bool forwards = true;
protected:
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == morph::key::right && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            ++this->curr_map_type;
            this->forwards = true;
        }
        if (key == morph::key::left && (action == morph::keyaction::press || action == morph::keyaction::repeat)) {
            --this->curr_map_type;
            this->forwards = false;
        }
        if (key == morph::key::h && action == morph::keyaction::press) { std::cout << "left/right switch maps\n"; }
    }
};

// Parameters for the fineness of the grid/cyclic wheels
static constexpr unsigned int Nside_w = 512;
static constexpr unsigned int Nside_h = 256;

morph::VisualModel<>* addmap (myvisual& v, morph::ColourMapType display_map_type, const morph::Grid<>& grid, const std::vector<float>& data)
{
    morph::VisualModel<>* vmp = nullptr;
    morph::ColourMap<float> nextmap (display_map_type);
    if ((nextmap.flags & morph::ColourMapFlags::cyclic) == true) {
        morph::vec<float, 3> offset = {0,0,0};
        auto cv = std::make_unique<morph::CyclicColourVisual<float>>(offset);
        v.bindmodel (cv);
        cv->outer_radius = 0.6;
        cv->inner_radius = 0.2;
        cv->numsegs = Nside_w;
        cv->numrings = Nside_h;
        cv->cm = nextmap;
        cv->draw_ticks = false;
        cv->addLabel (cv->cm.getTypeStr() + std::string(" (") + cv->cm.getFlagsStr() + std::string(")"),
                      morph::vec<float>({-1.3, -0.4, 0}), morph::TextFeatures(0.05f));
        cv->finalize();
        vmp = v.addVisualModel (cv);
    } else {
        morph::vec<float, 3> offset = { -0.5f * grid.width(), -0.5f * grid.height(), 0.0f };
        auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = morph::GridVisMode::Triangles;
        gv->twodimensional = true;
        gv->setScalarData (&data);
        gv->cm = nextmap;
        gv->zScale.setParams (0, 0);
        gv->addLabel (gv->cm.getTypeStr() + std::string(" (") + gv->cm.getFlagsStr() + std::string(")"),
                      morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05f));
        gv->finalize();
        vmp = v.addVisualModel (gv);
    }
    return vmp;
}

int main()
{
    myvisual v(2100, 1100, "Colourbar perceptual uniformity test");
    v.setSceneTrans (morph::vec<float,3>{ float{-0.00636619}, float{0.0518834}, float{-3} });

    // Create a grid for the colourmaps
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

    morph::ColourMapType display_map_type = v.curr_map_type;
    morph::VisualModel<>* gvp = addmap (v, v.curr_map_type, grid, data);

    while (v.readyToFinish == false) {
        v.render();
        v.waitevents (0.017);
        if (v.curr_map_type != display_map_type) {
            // Change to v.curr_map_type
            morph::ColourMap<float> nextmap(v.curr_map_type);
            if ((nextmap.flags & morph::ColourMapFlags::one_d) == true) {
                // Update the map
                v.removeVisualModel (gvp);
                gvp = addmap (v, v.curr_map_type, grid, data);
                display_map_type = v.curr_map_type;
            } else {
                // The map wasn't 1D, so skip
                if (v.forwards) { ++v.curr_map_type; } else { --v.curr_map_type; }
            }
        }
    }

    return 0;
}
