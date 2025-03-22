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
    morph::vvec<float> ord = {1, 1, 4, 2};

    morph::Visual v(1024, 768, "Bar graph");
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);

    morph::DatasetStyle ds(morph::stylepolicy::bar); // Draw a bar graph by creating a bar policy DatasetStyle
    ds.markercolour = morph::colour::aquamarine;     // markercolour sets the bar 'fill' colour
    ds.datalabel = "bar";
    ds.markersize = 0.2f;                     // The width of each bar
    ds.showlines = true;                      // Whether or not to draw the lines around the bar
    ds.linecolour = morph::colour::royalblue; // linecolour sets the 'lines around the bar' colour
    ds.linewidth = ds.markersize/20.0f;       // The linewidth of the lines around the bar
    // Bar graphs usually need to extend up from 0, so set scaling policy for the y axis accordingly:
    gv->scalingpolicy_y = morph::scalingpolicy::manual_min;
    gv->datarange_y.min = 0;
    // Set the data-to-axis distance based on the markersize.
    gv->setdataaxisdist (0.04f + ds.markersize/2.0f);
    gv->num_ticks_range_x = { 5, 5 };
    gv->setdata (absc, ord, ds);

    // It may be nice to condense the above into a convenience function:
    //    gv->add_bargraph (absc, ord, 0.2, morph::colour::aquamarine, morph::colour::royalblue);
    // with similar convenience functions:
    //    gv->add_linegraph (absc, ord, 0.03, morph::markerstyle::square, morph::colour::aquamarine, morph::colour::royalblue);

    // Add a line graph (default look)
    ord += 0.5;
    gv->setdata (absc, ord, "line");

    gv->xlabel = "Condition";
    gv->ylabel = "Value";
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
