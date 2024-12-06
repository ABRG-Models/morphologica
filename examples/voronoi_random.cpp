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

    morph::Visual v(1024, 768, "VoronoiVisual", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);

    morph::RandUniform<float> rngxy(-1.0f, 2.0f);
    morph::RandUniform<float> rngz(0.8f, 1.0f);

    // make n_points random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<float> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        data[i] = static_cast<float>(i) / n_points;
    }

    morph::vec<float, 3> offset = { 0.0f };
    auto asv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (asv);
    asv->show_voronoi2d = true; // true to show the 2D voronoi edges
    asv->debug_dataCoords = false; // true to show coordinate spheres
    asv->cm.setType (morph::ColourMapType::Plasma);
    asv->setDataCoords (&points);
    asv->setScalarData (&data);
    asv->finalize();
    v.addVisualModel (asv);

    v.keepOpen();

    return rtn;
}
