/*
 * Voronoi example using vector data and ColourMapType::RGB
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

    morph::Visual v(1024, 768, "VoronoiVisual vectorData");

    morph::RandUniform<float> rngxy(-2.0f, 2.0f, 1000);
    morph::RandUniform<float> rngz(0.8f, 1.0f, 1000);

    // make n_points random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<morph::vec<float>> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        // Set data RGB values from xyx position of points[i]
        data[i][0] = 0.5f + points[i][0] / 4.0f;
        data[i][1] = 0.5f + points[i][1] / 4.0f;
        data[i][2] = (-0.8f + points[i][2]) * 5.0f;
    }

    morph::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width  = length_scale;
#if 1
    // With RGB, the input is passed in as RGB channels with each channel being in range [0, 1]
    vorv->cm.setType (morph::ColourMapType::RGB);
#else
# if 0
    // You can alternatively specify a 2D map like DiscFourBlack...
    vorv->cm.setType (morph::ColourMapType::DiscFourBlack);
# else
    // ...or a 1D map with 'act as if 2d' set true (which will desaturate the map with one dimension of the data):
    vorv->cm.setType (morph::ColourMapType::Plasma);
    vorv->cm.set_act_2d (true);
# endif
#endif

    vorv->setDataCoords (&points);
    vorv->setVectorData (&data);
    vorv->finalize();
    v.addVisualModel (vorv);

    v.keepOpen();

    return rtn;
}
