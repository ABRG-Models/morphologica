#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <utility>
#include <iostream>
#include <unistd.h>
#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/Visual.h>
#include <morph/HexGridVisual.h>

using namespace std;

int main()
{
    int rtn = 0;
    try {
        string pwd = morph::Tools::getPwd();
        string curvepath = "../../tests/trialmod.svg";

        morph::ReadCurves r(curvepath);

        morph::HexGrid hg(0.01, 3, 0, morph::HexDomainShape::Parallelogram);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;
        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 14535) {
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
        for (auto h : hg.hexen) {
            if (h.boundaryHex() && h.insideBoundary()) {
                // red is boundary hex AND inside boundary
                std::cout << "red hex (bndry) at position " << h.ri << "," << h.gi << " with vi=" << h.vi << std::endl;
                if (colours[h.vi] == 0) colours[h.vi] = cl_boundary_and_in;
            } else if (h.boundaryHex()) {
                // orange is boundary ONLY
                std::cout << "orange hex (bndry) at position " << h.ri << "," << h.gi << " with vi=" << h.vi << std::endl;
                if (colours[h.vi] == 0) colours[h.vi] = cl_bndryonly;
            } else if (h.insideBoundary()) {
                // Inside boundary -  blue
                std::cout << "blue hex (inside) at position " << h.ri << "," << h.gi << " with vi=" << h.vi << std::endl;
                if (colours[h.vi] == 0) colours[h.vi] = cl_inside;
            } else {
                // The domain - greenish
                std::cout << "green hex (domain) at position " << h.ri << "," << h.gi << " with vi=" << h.vi << std::endl;
                if (colours[h.vi] == 0) colours[h.vi] = cl_domain;
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
