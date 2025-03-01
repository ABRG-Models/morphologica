/*
 * This example generates a number (n_points) of random (but bounded) coordinates and
 * uses the VoronoiVisual to display the coordinates as a map, with the order of
 * random-choice being used to colourize the Voronoi cells.
 *
 * This shows you how to use VoronoiVisual to visualize a surface from a non regular
 * grid (i.e. non morph::Grid or morph::HexGrid or morph::HealpixVisual ordered values)
 *
 * Author Seb James
 * Date 2024
 */
#include <morph/Visual.h>
#include <morph/VoronoiVisual.h>
#include <morph/vec.h>
#include <morph/Random.h>
#include <iostream>

static constexpr int n_points = 1000;

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "VoronoiVisual");

    morph::RandUniform<float> rngxy(-2.0f, 2.0f, 1000);
    morph::RandUniform<float> rngz(0.8f, 1.0f, 1000);

    // make n_points random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<float> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        data[i] = static_cast<float>(i) / n_points;
    }

    morph::ColourMapType cmap_t = morph::ColourMapType::Plasma;

    morph::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width  = length_scale;
    vorv->cm.setType (cmap_t);
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto vorvp = v.addVisualModel (vorv);

    int fcount = 0;
    while (!v.readyToFinish) {
        if (fcount++% 600 == 0) {
            vorvp->cm.setType (++cmap_t);
            vorvp->reinitColours(); // Not quite working when I change the colourmap
        }
        v.waitevents(0.018);
        v.render();
    }

    return rtn;
}
