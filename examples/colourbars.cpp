/*
 * A 3D surface with an associated 2D colourbar.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>
#include <morph/ColourBarVisual.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    morph::Visual v(1600, 1000, "ColourBar");

    // A HexGrid to show in the scene.
    morph::HexGrid hg(0.01f, 3.0f, 0.0f);
    hg.setCircularBoundary (0.6f);
    std::cout << "Number of pixels in grid:" << hg.num() << std::endl;

    // Make some data for the surface
    morph::vvec<float> data(hg.num(), 0.0f);
    for (unsigned int ri=0; ri<hg.num(); ++ri) {
        data[ri] = 0.00001f + 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, offset);
    v.bindmodel (hgv);
    hgv->setScalarData (&data);
    hgv->hexVisMode = morph::HexVisMode::Triangles;
    hgv->finalize();
    auto hgvp = v.addVisualModel (hgv);

    // Add the colour bar
    offset = {1.0f, 0.0f, 0.0f};
    auto cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
    v.bindmodel (cbv);
    cbv->orientation = morph::colourbar_orientation::vertical;
    cbv->tickside = morph::colourbar_tickside::right_or_below;
    // Copy colourmap and scale to colourbar visual
    cbv->cm = hgvp->cm;
    cbv->scale = hgvp->colourScale;
    // Now build it
    cbv->finalize();
    v.addVisualModel (cbv);

    // Add a horizontal colourbar, too
    offset = {1.5f, 0.0f, 0.0f};
    cbv =  std::make_unique<morph::ColourBarVisual<float>>(offset);
    v.bindmodel (cbv);
    cbv->orientation = morph::colourbar_orientation::horizontal;
    cbv->tickside = morph::colourbar_tickside::left_or_above;
    cbv->cm = hgvp->cm;
    cbv->scale = hgvp->colourScale;
    cbv->finalize();
    v.addVisualModel (cbv);

    v.keepOpen();

    return 0;
}
