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
    constexpr unsigned int Nside = 10;
    constexpr morph::vec<float, 2> grid_spacing = {0.2f, 0.2f};

    // The simplest declaration of Grid is:
    //   morph::Grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    morph::Grid grid(Nside, Nside, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    morph::vvec<morph::vec<float, 3>> data(grid.n, morph::vec<float, 3>({0.0f, 0.0f, 0.0f}));
    for (unsigned int ri=0; ri<grid.n; ++ri) {
        // auto coord = grid[ri];
        data[ri][0] =  static_cast<double>(std::rand()) / RAND_MAX ; // Range 0->1
    }

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the morph::Visual scene
    morph::vec<float, 3> offset = { -step * grid.width(), step * grid.width(), 0.0f };

    // 1) visualizing vector with GridVisMode = RectInterp
    auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->addLabel ("1) Base GridVisMode::RectInterp", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 2) same as 1 with zScale set to 0
    offset = { step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->addLabel ("2) 1 + no zScale", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 3) same as 2 with border ON and border colour set to cyan
    offset = { 3 * step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showborder = true;
    gv->border_thickness = 0.5f;
    gv->border_colour = morph::colour::cyan;
    gv->addLabel ("3) 2 + border", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 4) same as 2 with border ON and border colour set to cyan
    offset = { 5 * step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->showborder = true;
    gv->border_thickness = 0.5f;
    gv->border_colour = morph::colour::cyan;
    gv->addLabel ("4) 1 + border", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 5) same as 2 + grid
    offset = { step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showgrid = true;
    gv->grid_colour = morph::colour::red;
    gv->grid_thickness = 0.2f;
    gv->addLabel ("5) 2 + grid ", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 6) show both border and grid
    offset = { 3 * step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showgrid = true;
    gv->grid_colour = morph::colour::red;
    gv->grid_thickness = 0.2f;
    gv->showborder = true;
    gv->border_thickness = 0.5f;
    gv->border_colour = morph::colour::magenta;
    gv->addLabel ("6) 5 + border ", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 7) show how to use the selected pixel option
    offset = { step * grid.width(), -3 * step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);

    gv->showselectedpixborder = true;
    gv->selected_pix_indexes.reserve(9);
    gv->selected_pix_indexes.push_back(6);
    gv->selected_pix_indexes.push_back(0);
    gv->selected_pix_indexes.push_back(9);
    gv->selected_pix_indexes.push_back(10);
    gv->selected_pix_indexes.push_back(45);
    gv->selected_pix_indexes.push_back(46);
    gv->selected_pix_indexes.push_back(49);
    gv->selected_pix_indexes.push_back(90);
    gv->selected_pix_indexes.push_back(99);

    gv->grid_thickness = 0.2f;

    gv->selected_pix_border_colour.push_back(morph::colour::forestgreen);
    gv->selected_pix_border_colour.push_back(morph::colour::yellow3);
    gv->selected_pix_border_colour.push_back({1,0.2431372549,0.5882352941});
    gv->selected_pix_border_colour.push_back(morph::colour::skyblue);
    gv->selected_pix_border_colour.push_back(morph::colour::tomato2);
    gv->selected_pix_border_colour.push_back(morph::colour::gray55);
    gv->selected_pix_border_colour.push_back(morph::colour::red2);
    gv->selected_pix_border_colour.push_back(morph::colour::tan1);
    gv->selected_pix_border_colour.push_back(morph::colour::gold);

    gv->addLabel ("7) 2 + selected pixel borders", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 8) show how to use the selected pixel option with grid
    offset = { 3 * step * grid.width(), -3 * step * grid.width(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    
    gv->showgrid = true;
    gv->grid_colour = morph::colour::red;
    gv->grid_thickness = 0.2f;

    gv->showselectedpixborder = true;
    gv->selected_pix_indexes.reserve(9);
    gv->selected_pix_indexes.push_back(6);
    gv->selected_pix_indexes.push_back(0);
    gv->selected_pix_indexes.push_back(9);
    gv->selected_pix_indexes.push_back(10);
    gv->selected_pix_indexes.push_back(45)
    gv->selected_pix_indexes.push_back(46);
    gv->selected_pix_indexes.push_back(49);
    gv->selected_pix_indexes.push_back(90);
    gv->selected_pix_indexes.push_back(99);

    gv->grid_thickness = 0.2f;

    gv->selected_pix_border_colour.push_back(morph::colour::forestgreen);
    gv->selected_pix_border_colour.push_back(morph::colour::yellow3);
    gv->selected_pix_border_colour.push_back({1,0.2431372549,0.5882352941});
    gv->selected_pix_border_colour.push_back(morph::colour::skyblue);
    gv->selected_pix_border_colour.push_back(morph::colour::tomato2);
    gv->selected_pix_border_colour.push_back(morph::colour::gray55);
    gv->selected_pix_border_colour.push_back(morph::colour::red2);
    gv->selected_pix_border_colour.push_back(morph::colour::tan1);
    gv->selected_pix_border_colour.push_back(morph::colour::gold);

    gv->addLabel ("8) 7 + grid", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
