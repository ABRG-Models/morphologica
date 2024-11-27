/*
 * Many 2D colourbars to show all our different ColourMaps.
 */

#include <iostream>
#include <vector>
#include <string>
#include <morph/scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/ColourBarVisual.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    std::string title_str = "ColourMaps from William Lenthe";
    morph::Visual v(1600, 750, title_str);
    v.setSceneTrans (morph::vec<float,3>{ float{-1.88699}, float{0.239456}, float{-3.6} });

    morph::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<morph::ColourMapType> cmap_types;
    cmap_types.push_back (morph::ColourMapType::Fire);
    cmap_types.push_back (morph::ColourMapType::Ocean);
    cmap_types.push_back (morph::ColourMapType::Ice);
    cmap_types.push_back (morph::ColourMapType::DivBlueRed);
    cmap_types.push_back (morph::ColourMapType::CyclicGrey);
    cmap_types.push_back (morph::ColourMapType::CyclicFour);
    cmap_types.push_back (morph::ColourMapType::CyclicSix);
    cmap_types.push_back (morph::ColourMapType::CyclicDivBlueRed);
    cmap_types.push_back (morph::ColourMapType::Greyscale);
    cmap_types.push_back (morph::ColourMapType::GreyscaleInv);

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

    /*
     * Maps that encode 2D data
     */

    // HSV and Duochrome maps can be displayed on a Grid
    std::vector<morph::ColourMapType> cmap_2d_types;
    cmap_2d_types.push_back (morph::ColourMapType::DiscSixWhite);
    cmap_2d_types.push_back (morph::ColourMapType::DiscSixBlack);
    cmap_2d_types.push_back (morph::ColourMapType::DiscFourWhite);
    cmap_2d_types.push_back (morph::ColourMapType::DiscFourBlack);

    constexpr float pw = 0.03f; // pixel width
    constexpr int N = 20;
    constexpr morph::vec<float, 2> grid_spacing = { pw, pw };
    morph::Grid grid(N, N, grid_spacing);
    // Dummy data encodes 2D data
    std::vector<morph::vec<float, 3>> data(grid.n());
    for (int j = 0; j < grid.n(); ++j) {
        data[j] = (grid[j] / (N * pw)).plus_one_dim();
    }

    for (auto cmap_type : cmap_2d_types) {

        auto gv = std::make_unique<morph::GridVisual<float, int>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = morph::GridVisMode::Triangles;
        gv->setVectorData (&data);
        gv->cm.setType (cmap_type);
        gv->zScale.setParams(0,0);
        gv->addLabel (morph::ColourMap<float>::colourMapTypeToStr (cmap_type), {0, -0.1, 0}, morph::TextFeatures(0.05f));
        gv->twodimensional = true;
        gv->finalize();
        v.addVisualModel (gv);

        offset[0] += 0.8f;
    }

    v.keepOpen();

    return 0;
}
