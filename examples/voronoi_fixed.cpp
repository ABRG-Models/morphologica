/*
 * Make a very small Voronoi surface. Used to debug VoronoiVisual
 */
#include <morph/Visual.h>
#include <morph/VoronoiVisual.h>
#include <morph/vec.h>
#include <iostream>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "VoronoiVisual");

    std::vector<morph::vec<float>> points = {
        {0,0,1},
        {1,0,1},
        {0,1,1},
        {1,1,1},
        {.5,.5,0.8},
    };
    std::vector<float> data = {1,2,3,4,5};

    morph::vec<float, 3> offset = { 0.0f };
    auto asv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (asv);
    asv->show_voronoi2d = true;
    asv->debug_edges = false;
    asv->debug_dataCoords = true;
    asv->setDataCoords (&points);
    asv->setScalarData (&data);
    asv->finalize();
    v.addVisualModel (asv);

    v.keepOpen();

    return rtn;
}
