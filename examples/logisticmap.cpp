/*
 * Visualize a graph of the logistic map
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "The Logistic Map");
    v.zNear = 0.001;
    v.backgroundWhite();
    v.lightingEffects();

    try {
        morph::vvec<double> absc;
        morph::vvec<double> ord;
        auto gv = std::make_unique<morph::GraphVisual<double>>(morph::vec<float>({0,0,0}));
        v.bindmodel (gv);

        double x = 0.5;
        double x1 = 0.0;

        // This is a good opportunity to have a graph which updates with time
        for (double r = 1.0; r<4.0; r+=0.0001) {
            x = 0.5;
            bool converged = false;
            std::set<double> values;
            // Just run for a while, to allow settling
            for (unsigned int i = 0; i < 100000; ++i) {
                x1 = r*x*(1.0-x);
                x = x1;
            }
            size_t sz = values.size();
            while (!converged) {
                x1 = r*x*(1.0-x);
                //std::cout << "insert x1=" << x1 << std::endl;
                values.insert(x1);
                x = x1;
                if (sz == values.size()) { converged = true; }
                sz = values.size();
                if (sz > 100) { converged = true; }
            }
            for (auto v : values) {
                absc.push_back (r);
                ord.push_back (v);
            }
            if (r<3.0) {
                r+=0.0001;
            } else {
                r+=0.00001;
            }
        }
        std::cout << "absc size: " << absc.size() << ", ord size " <<  ord.size() << "absc.max():" << absc.max() << " ord max " << ord.max() << std::endl;

        gv->setsize (1.33, 1);
        gv->setlimits (1,4,0,1);

        morph::DatasetStyle ds;
        ds.markerstyle = morph::markerstyle::diamond;
        ds.markercolour = morph::colour::blue4;
        ds.markersize = 0.001;
        ds.policy =  morph::stylepolicy::markers; // markers, lines, both, allcolour
        ds.showlines = false;

        gv->policy = morph::stylepolicy::markers; // markers, lines, both, allcolour
        gv->xlabel = "r";
        gv->ylabel = "x";
        gv->setdata (absc, ord, ds);
        gv->twodimensional = false;
        gv->finalize();

        // Add the GraphVisual to the morph::Visual scene
        v.addVisualModel (gv);

        // You can render the scene manually like this
        v.render();

        // v.keepOpen() is equivalent to this:
        while (v.readyToFinish() == false) {
            v.waitevents (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
