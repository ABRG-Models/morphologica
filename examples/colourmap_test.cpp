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

int main()
{
    morph::Visual v(1600, 1000, "Colourbar perceptual uniformity test");

    // Create a grid for the colourmaps
    constexpr unsigned int Nside_w = 256;
    constexpr unsigned int Nside_h = 32;
    constexpr float barw = 2.56f;
    constexpr float barh = 0.5f;
    constexpr morph::vec<float, 2> grid_spacing = {barw/static_cast<float>(Nside_w), barh/static_cast<float>(Nside_h)};
    morph::Grid grid(Nside_w, Nside_h, grid_spacing);

    // Our data is a ramp with a sine wave embossed on it
    std::vector<float> data(grid.n(), 0.0);
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        auto x = grid[ri][0];
        auto y = grid[ri][1];
        data[ri] = x / grid.width() + 0.1f * (y / grid.height()) * (y / grid.height()) * std::sin (120.0f * x);
    }

    std::vector<morph::ColourMapType> cmtypes;
    cmtypes.push_back (morph::ColourMapType::Plasma);
    cmtypes.push_back (morph::ColourMapType::Jet);
    cmtypes.push_back (morph::ColourMapType::Petrov);
    cmtypes.push_back (morph::ColourMapType::Inferno);
    cmtypes.push_back (morph::ColourMapType::Rainbow);
    cmtypes.push_back (morph::ColourMapType::HSV1D);
    cmtypes.push_back (morph::ColourMapType::Viridis);
    cmtypes.push_back (morph::ColourMapType::Cividis);
    cmtypes.push_back (morph::ColourMapType::Twilight);
    cmtypes.push_back (morph::ColourMapType::Greyscale);
    cmtypes.push_back (morph::ColourMapType::MonochromeRed);
    cmtypes.push_back (morph::ColourMapType::MonochromeGreen);
    cmtypes.push_back (morph::ColourMapType::MonochromeBlue);
    cmtypes.push_back (morph::ColourMapType::MonovalRed);
    cmtypes.push_back (morph::ColourMapType::MonovalGreen);
    cmtypes.push_back (morph::ColourMapType::MonovalBlue);

    float step = 0.6f;
    morph::vec<float, 3> offset = { -step * grid.width(), -step * grid.height(), 0.0f };
    for (auto cmtype : cmtypes) {
        auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = morph::GridVisMode::Triangles;
        gv->setScalarData (&data);
        gv->cm.setType (cmtype);
        gv->zScale.setParams (0, 0);
        gv->addLabel (gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05f));
        gv->finalize();
        v.addVisualModel (gv);
        offset[1] -= grid.height() * 1.35;
    }

    v.keepOpen();

    return 0;
}
