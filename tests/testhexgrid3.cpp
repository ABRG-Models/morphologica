#include <morph/tools.h>
#include <morph/ColourMap.h>
#include <utility>
#include <iostream>
#include <unistd.h>
#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/Visual.h>
#include <morph/HexGridVisual.h>

int main()
{
    int rtn = 0;
    try {
        std::string pwd = morph::Tools::getPwd();
        std::string curvepath = "../../tests/trialmod.svg";
        morph::ReadCurves r(curvepath);

        morph::HexGrid hg(0.02, 7, 0, morph::HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        std::cout << hg.extent() << std::endl;
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 2088 && hg.num() != 2087) {
            std::cerr << "hg num (" << hg.num() << ") not equal to 2087/2088..." << std::endl;
            rtn = -1;
        }

        // Create a HexGrid
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
                colours[h.vi] = cl_boundary_and_in;
            } else if (h.boundaryHex()) {
                // orange is boundary ONLY
                colours[h.vi] = cl_bndryonly;
            } else if (h.insideBoundary()) {
                // Inside boundary -  blue
                colours[h.vi] = cl_inside;
            } else {
                // The domain - greenish
                colours[h.vi] = cl_domain;
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

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << morph::Tools::getPwd() << std::endl;
        rtn = -1;
    }

    return rtn;
}
