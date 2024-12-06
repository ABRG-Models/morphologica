/*
 * Visualize a XYz surface from points.
 */
#include <morph/Visual.h>
#include <morph/ArbSurfaceVisual.h>
#include <morph/vec.h>
#include <morph/Random.h>
#include <iostream>

static constexpr int n_points = 1000;

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "ArbSurfaceVisual", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);

    morph::RandUniform<float> rngxy(-1.0f, 2.0f);
    morph::RandUniform<float> rngz(0.8f, 1.0f);

    // make 100 random coordinates
    std::vector<morph::vec<float>> points(n_points);
    std::vector<float> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        data[i] = static_cast<float>(i) / n_points;
    }

    morph::vec<float, 3> offset = { 0.0f };
    auto asv = std::make_unique<morph::ArbSurfaceVisual<float>> (offset);
    v.bindmodel (asv);
    asv->setDataCoords (&points);
    asv->setScalarData (&data);
    asv->finalize();
    v.addVisualModel (asv);

    v.keepOpen();

    return rtn;
}
