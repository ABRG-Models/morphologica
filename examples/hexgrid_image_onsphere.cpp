/*
 * An example morph::Visual scene, containing a HexGrid, onto which is sampled an image. Then
 * projected onto a sphere.
 *
 * \author Seb James
 * \date June 2024
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

enum class spherical_projection
{
    mercator,
    equirectangular,
    cassini,
    splodge
};

int main()
{
    using mc = morph::mathconst<float>;
    constexpr spherical_projection proj = spherical_projection::mercator;

    morph::Visual v(1600, 1000, "Spherically transformed HexGrid");

    // radius of sphere
    constexpr float r_sph = 1.0f;

    constexpr float hex_d = 0.02;
    constexpr float hex_span = mc::two_pi * r_sph;
    morph::HexGrid hg(hex_d, 2.0f*hex_span, 0.0f);
    if constexpr (proj == spherical_projection::splodge) {
        hg.setCircularBoundary (0.95f * r_sph);
    } else {
        hg.setCircularBoundary (0.5f * mc::pi * r_sph);
        // You can set a rectangular boundary too:
        //hg.setRectangularBoundary (0.5f * mc::pi * r_sph, 0.5f * mc::pi * r_sph);
    }

    // Load an image with morph::loadpng()
    std::string fn = "../examples/bike256.png";
    morph::vvec<float> image_data;
    morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);

    // This controls how large the photo will be on the HexGrid
    morph::vec<float,2> image_scale = {3.2f, 3.2f};
    if constexpr (proj == spherical_projection::splodge) {
        image_scale = {2.8f, 2.8f};
    }

    // You can shift the photo with an offset if necessary
    morph::vec<float,2> image_offset = {0.0f, 0.0f};

    // Here's the HexGrid method that will resample the square pixel grid onto the hex grid
    morph::vvec<float> hex_image_data = hg.resampleImage (image_data, dims[1], image_scale, image_offset);

    // hg has d_x and d_y. Can make up a new container of 3D locations for each hex.
    morph::vvec<morph::vec<float, 3>> sphere_coords(hg.num());
    for (unsigned int i = 0u; i < hg.num(); ++i) {
        // This is the inverse Mercator projection.
        // See https://stackoverflow.com/questions/12732590/how-map-2d-grid-points-x-y-onto-sphere-as-3d-points-x-y-z
        morph::vec<float, 2> xy = { hg.d_x[i], hg.d_y[i] };
        float longitude = 0.0f; // or lambda
        float latitude = 0.0f;  // or phi
        if constexpr (proj == spherical_projection::equirectangular) {
            float phi0 = 0.0f;
            float phi1 = 0.0f;
            float lambda0 = 0.0f;
            longitude = xy[0] / (r_sph * std::cos(phi1)) + lambda0;
            latitude = xy[1] / r_sph + phi0;
        } else if constexpr (proj == spherical_projection::cassini) {
            // Spherical Cassini
            float phi0 = 0.0f;
            float lambda0 = 0.0f;
            float D = xy[1] / r_sph + phi0;
            longitude = lambda0 + std::atan2 (std::tan(xy[0]/r_sph), std::cos(D));
            latitude = std::asin (std::sin(D) * std::cos(xy[0]/r_sph));

        } else if constexpr (proj == spherical_projection::splodge) {
            // It's all below
        } else { // Default Mercator projection
            longitude = xy[0] / r_sph;
            latitude = 2.0f * std::atan (std::exp(xy[1]/r_sph)) - mc::pi_over_2;
        }

        if constexpr (proj == spherical_projection::splodge) {
            // In the splodge projection we just 'throw' the 2D plane onto a sphere
            float z_sq = r_sph * r_sph - xy.sq().sum();
            float z_sph = 0.0f;
            if (z_sq >= 0.0f)  {
                z_sph = std::sqrt (z_sq);
            } else {
                z_sph = -std::sqrt (-z_sq); // Anything beyond the edge of r_sph will make a nice bell shape
            }
            sphere_coords[i] = xy.plus_one_dim (z_sph);
        } else { // it's a serious projection
            float coslat = std::cos (latitude);
            float sinlat = std::sin (latitude);
            float coslong = std::cos (longitude);
            float sinlong = std::sin (longitude);
            sphere_coords[i] =  { r_sph * coslat * coslong, r_sph * coslat * sinlong , r_sph * sinlat };
        }
    }

    std::string label = "Mercator";
    if constexpr (proj == spherical_projection::equirectangular) {
        label = "Equirectangular";
    } else if constexpr (proj == spherical_projection::cassini) {
        label = "Spherical Cassini";
    } else if constexpr (proj == spherical_projection::splodge) {
        label = "'Raised Sphere'";
    }

    // Now visualise with a HexGridVisual
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, morph::vec<float>{1.5,0,0});
    v.bindmodel (hgv);
    // Set the image data as the scalar data for the HexGridVisual
    hgv->setScalarData (&hex_image_data);
    // This will make it a spherical projection (these coords will override the 2D coords in the HexGrid)
    hgv->setDataCoords (&sphere_coords);
    // The inverse greyscale map is appropriate for a monochrome image
    hgv->cm.setType (morph::ColourMapType::Inferno);
    hgv->addLabel (label, morph::vec<float>{ 0, -1.1f * r_sph, 0 }, morph::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Let's have a flat one alongside for comparison
    hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, morph::vec<float>{-1.5,0,-1});
    v.bindmodel (hgv);
    hgv->setScalarData (&hex_image_data);
    // The only real difference is that this has no hgv->setDataCoords(&sphere_coords) call.
    hgv->cm.setType (morph::ColourMapType::Inferno);
    hgv->zScale.setParams (0, 1);
    hgv->addLabel (std::string("2D HexGrid"),
                   morph::vec<float>{1.2*r_sph, -1.2*r_sph, 1},
                   morph::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
