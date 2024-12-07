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
    auto vorv = std::make_unique<morph::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = true;
    vorv->debug_edges = false;
    vorv->debug_dataCoords = true;
    // There's an issue with this if border_width is left at 0.0f
    //float length_scale = 4.0f / std::sqrt (points.size());
    //vorv->border_width  = length_scale;
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    v.addVisualModel (vorv);

    v.keepOpen();

    return rtn;
}
