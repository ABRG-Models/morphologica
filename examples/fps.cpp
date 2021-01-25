/*
 * An example morph::Visual scene, containing a HexGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <sstream>

#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, ?
    morph::Visual v(1600, 1000, "morph::Visual", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
    v.fov = 15;
    v.zFar = 200;
    v.lightingEffects();
    morph::VisualTextModel* fps_tm;
    v.addLabel ("0 FPS", {0.33f, -0.23f, 0.0f}, fps_tm); // With fps_tm can update the VisualTextModel with fps_tm->setupText("new text")

    // Create a HexGrid to show in the scene
    morph::HexGrid hg(0.02, 15, 0, morph::HexDomainShape::Boundary);
    hg.setEllipticalBoundary (4, 4);
    std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
    std::stringstream sss;
    sss << "Surface evaluated at " << hg.num() << " coordinates";
    v.addLabel (sss.str(), {0.0f, 0.0f, 0.0f});

    // Make some dummy data (a radially symmetric Bessel fn) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0);
    std::vector<float> r(hg.num(), 0.0);
    float k = 1.0f;
    for (unsigned int hi=0; hi<hg.num(); ++hi) {
        // x/y: hg.d_x[hi] hg.d_y[hi]
        r[hi] = std::sqrt (hg.d_x[hi]*hg.d_x[hi] + hg.d_y[hi]*hg.d_y[hi]);
        data[hi] = std::sin(k*r[hi])/k*r[hi];
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::Vector<float, 3> offset = { 0.0, -0.05, 0.0 };
    morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset, &data);
    hgv->hexVisMode = morph::HexVisMode::Triangles;
    unsigned int gridId = v.addVisualModel (hgv);
    std::cout << "Added HexGridVisual with gridId " << gridId << std::endl;

    using namespace std::chrono;
    using std::chrono::steady_clock;

    auto tstart = steady_clock::now();
    unsigned int fcount = 0;
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.00001);
        if (k > 8.0f) { k = 1.0f; }
#if 1
#pragma omp parallel for shared(r,k)
        for (unsigned int hi=0; hi<hg.num(); ++hi) {
            data[hi] = std::sin(k*r[hi])/k*r[hi];
        }
#endif
        hgv->updateData (&data);
        k += 0.02f;

        auto tduration = steady_clock::now() - tstart;
        if (duration_cast<milliseconds>(tduration).count() > 500) {
            // Update FPS text
            double tau = duration_cast<milliseconds>(tduration).count();
            std::stringstream ss;
            ss << std::round((((double)fcount/tau))*1000.0) << " FPS";
            fps_tm->setupText (ss.str());
            tstart = steady_clock::now();
            fcount = 0;
        }

        v.render();
        fcount++;
    }


    return 0;
}
