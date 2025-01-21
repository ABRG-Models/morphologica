/*
 * Bargraph example
 */

#include <morph/vvec.h>
#include <morph/Random.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <iostream>

int main()
{
    // Data
    morph::vvec<float> absc = {1, 2, 3, 4};
    morph::vvec<float> familarities = {1, 1, 4, 2};

    morph::Visual v(1024, 768, "Bar graph", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);

    std::cout << "1 ord2_scale is " << (gv->ord2_scale.ready() ? "ready" : "unready") << std::endl;

    gv->setsize (2, 2);
    std::cout << "2 ord2_scale is " << (gv->ord2_scale.ready() ? "ready" : "unready") << std::endl;

    gv->twodimensional = true;
    gv->scalingpolicy_y = morph::scalingpolicy::manual;
    gv->datarange_y.set(0, 1.0f);
    std::cout << "3 ord2_scale is " << (gv->ord2_scale.ready() ? "ready" : "unready") << std::endl;
    gv->xlabel = "";
    gv->legend = false;
    gv->omit_x_tick_labels = true;

    morph::DatasetStyle ds(morph::stylepolicy::bar); // Draw a bar graph by creating a bar policy DatasetStyle
    ds.markercolour = morph::colour::aquamarine; // bar colour
    ds.datalabel = "bar";
    ds.markersize = 0.1f;
    ds.linewidth = ds.markersize/8.0f;
    ds.axisside = morph::axisside::right;
    //  generate x axis values, must have same type as GraphVisual Template
    std::vector<float> xaxis(familarities.size());
    std::iota (xaxis.begin(), xaxis.end(), 0.f);
    std::cout << "4 ord2_scale is " << (gv->ord2_scale.ready() ? "ready" : "unready") << std::endl;
    gv->setdata (xaxis, familarities, ds); // sets ord2_scale
    std::cout << "5 ord2_scale is " << (gv->ord2_scale.ready() ? "ready" : "unready") << std::endl;

    // Render a red vertical line at the maximum familiarity offset
    morph::DatasetStyle ds_line(morph::stylepolicy::lines);
    ds_line.markercolour = morph::colour::red; // line colour
    float max_offset = 3;
    morph::vvec<float> annotx = {max_offset, max_offset};
    morph::vvec<float> annoty = {0.f, 1.f};
    ds_line.axisside = morph::axisside::left; // Unfix
    gv->setdata (annotx, annoty, ds_line); // Fails, but SHOULD work ok as ord1_scale has not been scaled?

    gv->finalize();

    v.addVisualModel (gv);

    // Render the graph until user exits
    v.render();
    while (v.readyToFinish == false) {
        v.waitevents (0.018);
        v.render();
    }

    return 0;
}
