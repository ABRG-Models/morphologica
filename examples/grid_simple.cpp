/*
 * An example morph::Visual scene, containing a Gridv, and using GridvVisual
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

int main()
{
    morph::Visual v(1600, 1000, "morph::GridVisual");

#ifdef ORTHOGRAPHIC
    // Here's how to set your Visual to do an orthographic projection rather than perspective
    v.ptype = morph::perspective_type::orthographic;
#endif

    // Create a grid to show in the scene
    constexpr unsigned int Nside = 100;
    constexpr morph::vec<float, 2> grid_spacing = {0.01f, 0.01f};

    // The simplest declaration of Grid is:
    //   morph::Grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    morph::Grid grid(Nside, Nside, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(grid.n, 0.0);
    for (unsigned int ri=0; ri<grid.n; ++ri) {
        auto coord = grid[ri];
        data[ri] = 0.05f + 0.05f * std::sin(20.0f * coord[0]) * std::sin(10.0f*coord[1]) ; // Range 0->1
    }

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the morph::Visual scene
    morph::vec<float, 3> offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->addLabel (std::string("GridVisMode::Triangles, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Buda);
    gv->addLabel (std::string("GridVisMode::RectInterp, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { -step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Columns;
    gv->interpolate_colour_sides = true;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Imola);
    gv->addLabel (std::string("GridVisMode::Columns, interpolated sides, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Columns;
    //gv->interpolate_colour_sides = false; // default
    //gv->clr_east_column = morph::colour::black; // These are defaults but you can change them
    //gv->clr_north_column = morph::colour::black;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Managua);
    gv->addLabel (std::string("GridVisMode::Columns, black sides, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { 3 * step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Navia);
    gv->addLabel (std::string("GridVisMode::Pixels, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
