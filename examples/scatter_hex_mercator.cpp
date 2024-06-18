/*
 * Apply an inverse Mercator projection to a HexGrid to place it on a sphere. Then visualize.
 */

#include <morph/HexGrid.h>
#include <morph/Visual.h>
#include <morph/ScatterVisual.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <cmath>

int main()
{
    using mc = morph::mathconst<float>;

    int rtn = -1;

    morph::Visual v(1024, 768, "Inverse Mercator HexGrid");
    v.showCoordArrows = true;
    v.lightingEffects();

    morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
    morph::Scale<float> scale;
    scale.setParams (1.0f, 0.0f);

    // Have a Sphere of radius 1
    constexpr float r_sph = 1.0f;

    // Make a HexGrid of width similar to sphere
    constexpr float hex_d = 0.1f;
    constexpr float hex_span = 3.0f * mc::two_pi * r_sph;
    morph::HexGrid hg(hex_d, hex_span, 0.0f);
    hg.setCircularBoundary(0.6f * mc::pi * r_sph);

    // hg has d_x and d_y. Can make up a new container of 3D locations for each hex.
    morph::vvec<morph::vec<float, 3>> sphere_coords (hg.num());
    for (unsigned int i = 0; i < hg.num(); ++i) {
        // This is the inverse Mercator projection.
        // See https://stackoverflow.com/questions/12732590/how-map-2d-grid-points-x-y-onto-sphere-as-3d-points-x-y-z
        float longitude = hg.d_x[i] / r_sph;
        float latitude = 2.0f * std::atan (std::exp(hg.d_y[i]/r_sph)) - mc::pi_over_2;
        float coslat = std::cos (latitude);
        float sinlat = std::sin (latitude);
        float coslong = std::cos (longitude);
        float sinlong = std::sin (longitude);
        sphere_coords[i] = { r_sph * coslat * coslong, r_sph * coslat * sinlong , r_sph * sinlat };
    }

    morph::vvec<float> data;
    data.linspace (0, 1, hg.num());

    auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
    v.bindmodel (sv);
    sv->setDataCoords (&sphere_coords);
    sv->setScalarData (&data);
    sv->radiusFixed = 0.005f;
    sv->colourScale = scale;
    sv->cm.setType (morph::ColourMapType::Jet);
    sv->finalize();
    v.addVisualModel (sv);

    v.keepOpen();

    return rtn;
}
