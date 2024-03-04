/*
 * An example morph::Visual scene, containing a Grid<>, and using GridVisual
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridctVisual.h>
#include <morph/Gridct.h>

int main()
{
    morph::Visual v(1600, 1000, "morph::GridctVisual");
    v.lightingEffects();
    v.addLabel ("This is a\nmorph::GridctVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a Gridct to show in the scene
    constexpr size_t Nside = 100;
    constexpr morph::vec<float, 2> grid_spacing = {0.01f, 0.01f};
    constexpr morph::vec<float, 2> grid_zero = {0.0f, 0.0f};
    constexpr morph::GridDomainWrap d_wrap = morph::GridDomainWrap::None;
    constexpr morph::GridOrder g_order = morph::GridOrder::bottomleft_to_topright;

    morph::Gridct<Nside, Nside, grid_spacing, grid_zero, true, d_wrap, g_order> grid;
    std::cout << "Number of pixels in grid:" << grid.n << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(grid.n, 0.0);
    for (unsigned int ri=0; ri<grid.n; ++ri) {
        auto coord = grid[ri];
        data[ri] = 0.05f + 0.05f * std::sin(20.0f * coord[0]) * std::sin(10.0f*coord[1]) ; // Range 0->1
    }

    // Add a CartGridVisual to display the CartGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto gv = std::make_unique<morph::GridctVisual<float, Nside, Nside, grid_spacing, grid_zero, true, d_wrap, g_order>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridctVisMode::RectInterp; // RectInterp or Triangles
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Twilight);
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
