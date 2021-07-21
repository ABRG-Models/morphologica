/*
 * Linear regression, with visualisation
 */

#include <morph/Vector.h>
#include <morph/vVector.h>
#include <morph/Random.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/MathAlgo.h>
#include <iostream>

int main()
{
    // Data
    morph::vVector<float> absc = { 1, 2, 3, 4, 5 }; // x
    morph::vVector<float> ord = { 1, 3, 2, 3, 5 }; // y

    // Fit y = mx + c
    std::pair<float, float> mc = morph::MathAlgo::linregr (absc, ord);
    std::cout << "Linear regression coefficients: gradient=" << mc.first << ", offset=" << mc.second << std::endl;
    // Create fit data points for visualisation:
    morph::vVector<float> fit = (absc * mc.first) + mc.second;

    // Visualise data and linear fit
    morph::Visual v(1024, 768, "Linear regression", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);
    morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});

    // The first dataset shows the data points
    morph::DatasetStyle ds(morph::stylepolicy::markers);
    ds.markercolour = morph::colour::blue3;
    ds.markersize = 0.05;
    ds.datalabel = "data";
    gv->setdata (absc, ord, ds);

    // The second dataset is for the fit
    morph::DatasetStyle ds2(morph::stylepolicy::lines);
    ds2.linecolour = morph::colour::lightsteelblue2;
    ds2.datalabel = "fit";
    gv->setdata (absc, fit, ds2);

    // Add axis labels, finalize and add to our morph::Visual:
    gv->xlabel = "x";
    gv->ylabel = "y";
    gv->finalize();
    v.addVisualModel (static_cast<morph::VisualModel*>(gv));

    // Render the graph until user exits
    v.render();
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
