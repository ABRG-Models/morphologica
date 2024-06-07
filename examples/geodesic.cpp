/*
 * Visualize an Icosahedron
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GeodesicVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Geodesic Polyhedron");
    v.showCoordArrows = true;

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::vec<float, 3> step = { 2.2, 0.0, 0.0 };

        int imax = 4;
        for (int i = 0; i < imax; ++i) {
            auto gv1 = std::make_unique<morph::GeodesicVisual<float>> (offset + step * i, 0.9f);
            v.bindmodel (gv1);
            gv1->iterations = i;
            std::string lbl = std::string("iterations = ") + std::to_string(i);
            gv1->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
            gv1->cm.setType (morph::ColourMapType::Jet);
            //gv1->colourScale.do_autoscale = false;
            //gv1->colourScale.compute_autoscale (0.0f, 2.0f);
            gv1->finalize();
#if 0       // no re-colouring
            v.addVisualModel (gv1);
#else       // re-colour after construction
            auto gv1p = v.addVisualModel (gv1);

            float imax_mult = 1.0f / static_cast<float>(imax);
# if 0      // Funky pattern colouring
            for (unsigned int j = 0; j < gv1p->data.size(); ++j) {
                gv1p->data[j] = j%3 ? 0 : (1+i) * imax_mult;
            }
# else      // sequential colouring
            size_t sz1 = gv1p->data.size();
            gv1p->data.linspace (0.0f, 1+i * imax_mult, sz1);
# endif
            gv1p->updateColours();
#endif
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
