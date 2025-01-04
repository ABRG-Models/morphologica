/*
 * An example morph::Visual scene, containing a Grid, and using GridVisual. This is for
 * debugging/demonstrating grid borders. see aso grid_border.cpp
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>

int main()
{
    morph::Visual v(1600, 1000, "Flat GridVisual grids with borders");
    v.lightingEffects();

    // Create a grid to show in the scene
    constexpr unsigned int Nside = 4; // You can change this
    constexpr morph::vec<float, 2> grid_spacing = {0.5f, 0.5f};
    morph::Grid grid(Nside, Nside, grid_spacing);
    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(grid.n(), 0.0);

    // Set data
    constexpr float length = morph::mathconst<float>::pi_over_4;
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        auto coord = grid[ri];
        data[ri] = std::sin(length * coord[0]) * std::sin(0.5f * length * coord[1]) ; // Range 0->1
    }

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the morph::Visual scene
    morph::vec<float, 3> offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    // A label position offset for use below
    auto dx = grid.get_dx();
    auto lblpos = -dx.plus_one_dim() + morph::vec<float>{dx[0]/2.0f,0,0};

    // Grid with border
    auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    // Border specific parameters
    gv->showborder (true);
    gv->border_thickness = 0.15f; // of a pixel
    gv->border_z_offset = 0.0f;
    gv->border_colour = morph::colour::aquamarine3;
    gv->addLabel ("Pixels, border", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);

    // Grid with no border
    offset[0] += grid.width_of_pixels() * 1.2f;
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    gv->showborder (false);
    gv->implygrid (true);
    gv->addLabel ("Rectinterp, No border, implied grid", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += grid.width_of_pixels() * 1.2f;
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::RectInterp;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    gv->showborder (false);
    gv->showgrid (true);
    gv->grid_colour = morph::colour::grey48;
    gv->addLabel ("Rectinterp, No border, filled grid", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);

    // Selected pix
    offset[0] += grid.width_of_pixels() * 1.2f;
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    // Border specific parameters
    gv->showborder (true);
    gv->border_thickness = 0.15f; // of a pixel
    gv->border_z_offset = 0.0f;
    gv->border_colour = morph::colour::black;
    // selected pix
    gv->selected_pix[5] = morph::colour::orangered2;
    gv->selected_pix[6] = morph::colour::crimson;
    gv->selected_pix[9] = morph::colour::crimson;
    gv->selected_pix[10] = morph::colour::crimson;
    gv->showselectedpixborder (false);
    gv->showselectedpixborder_enclosing (true);
    gv->addLabel ("Pixels, border, selected pix with border", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    offset[1] -= grid.height_of_pixels() * 1.2f;
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    // Border specific parameters
    gv->showborder (true);
    gv->border_thickness = 0.15f; // of a pixel
    gv->border_z_offset = 0.0f;
    gv->border_colour = morph::colour::grey10;
    gv->border_tubular (false);
    // selected pix
    gv->selected_pix[0] = morph::colour::crimson;
    gv->selected_pix[1+4] = morph::colour::blue2;
    gv->selected_pix[2+8] = morph::colour::goldenrod2;
    gv->selected_pix[3+12] = morph::colour::royalblue2;
    gv->showselectedpixborder (true);
    gv->showselectedpixborder_enclosing (false);
    gv->addLabel ("Pixels, flat border, selected pix coloured", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += grid.width_of_pixels() * 1.2f;
    gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    // Border specific parameters
    gv->showborder (true);
    gv->border_thickness = 0.15f; // of a pixel
    gv->border_z_offset = 0.0f;
    gv->border_colour = morph::colour::grey32;
    gv->addLabel ("Triangles, border (smaller is as expected)", lblpos, morph::TextFeatures(0.08f));
    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
