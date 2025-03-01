/*
 * Linear regression, with visualisation
 */

#include <morph/vvec.h>
#include <morph/Random.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/MathAlgo.h>
#include <iostream>

int main()
{
    // Data
    morph::vvec<float> absc = { 1, 2, 3, 4, 5 }; // x
    morph::vvec<float> ord = { 1, 3, 2, 3, 5 }; // y

    // Fit y = mx + c
    morph::vec<float, 2> mc = morph::MathAlgo::linregr (absc, ord);
    std::cout << "Linear regression coefficients: gradient=" << mc[0] << ", offset=" << mc[1] << std::endl;
    // Create fit data points for visualisation:
    morph::vvec<float> fit = (absc * mc[0]) + mc[1];

    // Visualise data and linear fit
    morph::Visual v(1024, 768, "Linear regression");
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);

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
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.render();
    while (v.readyToFinish == false) {
        v.waitevents (0.018);
        v.render();
    }

    return 0;
}
