/*
 * This example generates a number (n_points) of random xy positions. The z coordinate
 * is always 0. A function gives scalarData input, so that the colourmap represents the
 * value of the function.
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

    // make n_points random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<float> data(n_points);
    std::vector<float> r(n_points);

    float k = 1.0f;

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), 0.0f };
        r[i] = points[i].length();
        data[i] = std::sin (k * r[i]) / k * r[i]; // colour function
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
    while (!v.readyToFinish()) {

        if (k > 8.0f) { k = 1.0f; }
        for (unsigned int i = 0; i < n_points; ++i) {
            data[i] = std::sin (k * r[i]) / k * r[i];
        }

        if (fcount++% 600 == 0) {
            vorvp->cm.setType (++cmap_t);
        }
        vorvp->reinitColours(); // Not quite working when I change the colourmap
        v.waitevents(0.018);
        v.render();
        k += 0.01f;
    }

    return rtn;
}
