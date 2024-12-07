/*
 * This example generates a number (n_points) of random xy positions. zcoords are some
 * function. The value of z is used also as the scalarData input, so that the colourmap
 * represents the height of the surface.c
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

    morph::RandUniform<float> rngxy(-2.0f, 2.0f, 1000);

    // make n_points random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<float> data(n_points);
    std::vector<float> r(n_points);

    float k = 1.0f;

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), 0.0f };
        r[i] = points[i].length();
        data[i] = std::sin (k * r[i]) / k * r[i]; // colour function
        points[i][2] = data[i];
    }

    morph::ColourMapType cmap_t = morph::ColourMapType::Plasma;

    morph::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = false; // true to show the 2D voronoi edges
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

        if (k > 8.0f) { k = 1.0f; }
        for (unsigned int i = 0; i < n_points; ++i) {
            data[i] = std::sin (k * r[i]) / k * r[i];
            points[i][2] = data[i];
        }

        if (fcount++% 600 == 0) {
            vorvp->cm.setType (++cmap_t);
        }

        vorvp->reinit(); // slow, but map has to be rebuilt

        v.waitevents(0.001);
        v.render();
        k += 0.01f;
    }

    return rtn;
}
