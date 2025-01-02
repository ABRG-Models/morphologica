/*
 * An example morph::Visual scene, containing a Gridv, and using GridvVisual
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
    morph::Visual v(1600, 1000, "morph::GridVisual");

#ifdef ORTHOGRAPHIC
    // Here's how to set your Visual to do an orthographic projection rather than perspective
    v.ptype = morph::perspective_type::orthographic;
#endif

    // Create a grid to show in the scene
    constexpr unsigned int N_pix_w = 25;
    constexpr unsigned int N_pix_h = 8;
    constexpr morph::vec<float, 2> grid_spacing = {0.2f, 0.2f};

    // The simplest declaration of Grid is:
    //   morph::Grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    morph::Grid<unsigned int, float> grid(N_pix_w, N_pix_h, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    morph::vvec<morph::vec<float, 3>> data(grid.n(), morph::vec<float, 3>({0.0f, 0.0f, 0.0f}));
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        // auto coord = grid[ri];
        data[ri][0] =  static_cast<double>(std::rand()) / RAND_MAX ; // Range 0->1
    }

    float step = 0.64f;
    // Add a GridVisual to display the Grid within the morph::Visual scene
    morph::vec<float, 3> offset = { -step * grid.width(), step * grid.height(), 0.0f };

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
    offset = { step * grid.width(), step * grid.height(), 0.0f };
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
    offset = { 3 * step * grid.width(), step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showborder = true;
    gv->border_thickness = 0.25f;
    gv->border_colour = morph::colour::cyan;
    gv->addLabel ("3) 2 + border", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 4) same as 2 with border ON and border colour set to cyan
    offset = { 5 * step * grid.width(), step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->showborder = true;
    gv->border_thickness = 0.25f;
    gv->border_colour = morph::colour::cyan;
    gv->addLabel ("4) 1 + border", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 5) same as 2 + grid
    offset = { step * grid.width(), -step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showgrid = true;
    gv->grid_colour = morph::colour::black;
    gv->grid_thickness = 0.1f;
    gv->addLabel ("5) 2 + grid ", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 6) show both border and grid
    offset = { 3 * step * grid.width(), -step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);
    gv->showgrid = true;
    gv->grid_colour = morph::colour::black;
    gv->grid_thickness = 0.1f;
    gv->showborder = true;
    gv->border_thickness = 0.25f;
    gv->border_colour = morph::colour::magenta;
    gv->addLabel ("6) 5 + border ", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 7) show how to use the selected pixel option
    offset = { step * grid.width(), -3 * step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);

    gv->showselectedpixborder = true;
    gv->selected_pix[6] = morph::colour::forestgreen;
    gv->selected_pix[0] = morph::colour::yellow3;
    gv->selected_pix[9] = {1,0.2431372549,0.5882352941};
    gv->selected_pix[10] = morph::colour::skyblue;
    gv->selected_pix[124] = morph::colour::tomato2;
    gv->selected_pix[125] = morph::colour::gray55;
    gv->selected_pix[49] = morph::colour::red2;
    gv->selected_pix[90] = morph::colour::tan1;
    gv->selected_pix[99] = morph::colour::gold;

    gv->grid_thickness = 0.1f;

    gv->addLabel ("7) 2 + selected pixel borders", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 8) show how to use the selected pixel option with grid
    offset = { 3 * step * grid.width(), -3 * step * grid.height(), 0.0f };
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->zScale.setParams(0.0f, 0.0f);

    gv->showgrid = true;
    gv->grid_colour = morph::colour::black;
    gv->grid_thickness = 0.05f;

    gv->showselectedpixborder = true;
    gv->selected_pix_thickness = 0.1f;

    gv->selected_pix[6] = morph::colour::forestgreen;
    gv->selected_pix[0] = morph::colour::yellow3;
    gv->selected_pix[9] = {1,0.2431372549,0.5882352941};
    gv->selected_pix[10] = morph::colour::skyblue;
    gv->selected_pix[124] = morph::colour::tomato2;
    gv->selected_pix[125] = morph::colour::gray55;
    gv->selected_pix[49] = morph::colour::red2;
    gv->selected_pix[90] = morph::colour::tan1;
    gv->selected_pix[99] = morph::colour::gold;

    gv->addLabel ("8) 7 + grid", morph::vec<float>({0,-0.2,0}), morph::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
