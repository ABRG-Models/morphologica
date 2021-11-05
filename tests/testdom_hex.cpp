#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <utility>
#include <iostream>

#include "morph/HexGrid.h"
#include "morph/ReadCurves.h"
#include "morph/Visual.h"
#include "morph/HexGridVisual.h"

using namespace std;

int main()
{
    int rtn = 0;
    try {
        string pwd = morph::Tools::getPwd();
        string curvepath = "../../tests/trialmod.svg";

        morph::ReadCurves r(curvepath);

        morph::HexGrid hg(0.01, 1.2, 0, morph::HexDomainShape::Hexagon);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 11347) {
            rtn = -1;
        }

        // Create a HexGrid Visual
        morph::Visual v(1600, 1000, "HexGrid");
        v.lightingEffects();
        morph::Vector<float, 3> offset = { 0.0f, -0.0f, 0.0f };
        morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset);
        // Set up data for the HexGridVisual and colour hexes according to their state as being boundary/inside/domain, etc
        std::vector<float> colours (hg.num(), 0.0f);
        static constexpr float cl_boundary_and_in = 0.9f;
        static constexpr float cl_bndryonly = 0.8f;
        static constexpr float cl_domain = 0.5f;
        static constexpr float cl_inside = 0.15f;
        // Note, HexGridVisual uses d_x and d_y vectors, so set colours according to d_flags vector
        for (unsigned int i = 0; i < hg.num(); ++i) {
            if (hg.d_flags[i] & HEX_IS_BOUNDARY ? true : false
                && hg.d_flags[i] & HEX_INSIDE_BOUNDARY ? true : false) {
                // red is boundary hex AND inside boundary
                colours[i] = cl_boundary_and_in;
            } else if (hg.d_flags[i] & HEX_IS_BOUNDARY ? true : false) {
                // orange is boundary ONLY
                colours[i] = cl_bndryonly;
            } else if (hg.d_flags[i] & HEX_INSIDE_BOUNDARY ? true : false) {
                // Inside boundary -  blue
                colours[i] = cl_inside;
            } else {
                // The domain - greenish
                colours[i] = cl_domain;
            }
        }
        hgv->cm.setType (morph::ColourMapType::Jet);
        hgv->zScale.setParams (0,0); // makes the output flat in z direction, but you still get the colours
        hgv->setScalarData (&colours);
        hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
        hgv->finalize();
        v.addVisualModel (hgv);

        // Would be nice to:
        // Draw small hex at boundary centroid.
        // red hex at zero

        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading svg: " << e.what() << endl;
        cerr << "Current working directory: " << morph::Tools::getPwd() << endl;
        rtn = -1;
    }

    return rtn;
}
