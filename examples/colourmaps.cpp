/*
 * Many 2D colourbars to show all our different ColourMaps.
 */

#include <iostream>
#include <vector>
#include <string>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/ColourBarVisual.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    std::string title_str = "ColourMaps";
    morph::Visual v(1000, 1400, title_str);
    v.setSceneTrans (morph::vec<float,3>{ float{-1.17245}, float{1.24502}, float{-7.7} });

    morph::Scale<float> scale1;
    scale1.set_input_range (0, 1); // Simply maps 0->1 to 0->1!

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<morph::ColourMapType> cmap_types;
    cmap_types.push_back (morph::ColourMapType::Jet);
    cmap_types.push_back (morph::ColourMapType::Rainbow);
    cmap_types.push_back (morph::ColourMapType::Magma);
    cmap_types.push_back (morph::ColourMapType::Inferno);
    cmap_types.push_back (morph::ColourMapType::Plasma);
    cmap_types.push_back (morph::ColourMapType::Viridis);
    cmap_types.push_back (morph::ColourMapType::Cividis);
    cmap_types.push_back (morph::ColourMapType::Twilight);
    cmap_types.push_back (morph::ColourMapType::Greyscale);
    cmap_types.push_back (morph::ColourMapType::GreyscaleInv);
    cmap_types.push_back (morph::ColourMapType::Monochrome);
    cmap_types.push_back (morph::ColourMapType::Monoval);

    morph::ColourMap<float> cm1(morph::ColourMapType::Jet);

    // Display 1D colour maps
    int i = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        cbv->scale = scale1;
        cbv->addLabel (morph::ColourMap<float>::colourMapTypeToStr (cmap_type), {0, -0.1, 0}, morph::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    float hue = 0.0f;
    for (int k = 0; k < 6; ++k) {
        ++i;
        cm1.setType (morph::ColourMapType::HSV1D);
        auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = morph::colourbar_orientation::vertical;
        cbv->tickside = morph::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        // Set the 'hue' angle (range 0 to 1)
        cbv->cm.setHue (hue);
        cbv->scale = scale1;
        cbv->addLabel ("hsv1d " + std::to_string(static_cast<int>(hue * 360)), {0, -0.1, 0}, morph::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Increment hue
        hue += 0.2f;
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    /*
     * Maps that encode 2D data
     */

    // HSV and Duochrome maps can be displayed on a Grid
    std::vector<morph::ColourMapType> cmap_2d_types;
    cmap_2d_types.push_back (morph::ColourMapType::HSV);
    cmap_2d_types.push_back (morph::ColourMapType::Duochrome);

    constexpr float pw = 0.03f; // pixel width
    constexpr int N = 20;
    constexpr morph::vec<float, 2> grid_spacing = { pw, pw };
    morph::Grid grid(N, N, grid_spacing);
    // Dummy data encodes 2D data
    std::vector<morph::vec<float, 3>> data(grid.n);
    for (int j = 0; j < grid.n; ++j) {
        data[j] = (grid[j] / (N * pw)).plus_one_dim();
    }

    auto gv = std::make_unique<morph::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[0]);
    gv->zScale.setParams(0,0);
    gv->addLabel (morph::ColourMap<float>::colourMapTypeToStr (cmap_2d_types[0]), {0, -0.1, 0}, morph::TextFeatures(0.05f));
    gv->twodimensional = true;
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += 0.8f;

    gv = std::make_unique<morph::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[1]);
    gv->zScale.setParams(0,0);
    gv->addLabel ("duochrome red-green", {0, -0.1, 0}, morph::TextFeatures(0.05f));
    gv->twodimensional = true;
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += 0.8f;

    gv = std::make_unique<morph::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[1]);
    gv->cm.setHueRB();
    gv->zScale.setParams(0,0);
    gv->addLabel ("duochrome red-blue", {0, -0.1, 0}, morph::TextFeatures(0.05f));
    gv->twodimensional = true;
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
