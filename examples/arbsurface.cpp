/*
 * Visualize a XYz surface from points.
 */
#include <morph/Visual.h>
#include <morph/ArbSurfaceVisual.h>
#include <morph/vec.h>
#include <iostream>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "ArbSurfaceVisual", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);

    std::vector<morph::vec<float>> points = {
        {0,0,1},
        {1,0,1.1},
        {2,0,1},
        {1,1,1.3},
        {0,2,1},
        {1,2,0.9},
        {2,2,1},
    };
    std::vector<float> data = { .1, .2, .3, .4, .4, .4, .9 };

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
