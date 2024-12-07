/*
 * An example morph::Visual scene, containing a Grid, and using GridVisual. The function shown
 * changes and is updated with reinitColours() and reinit() and each is profiled.
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/GridVisual.h>
#include <morph/Grid.h>

int main()
{
    morph::Visual v(1600, 1000, "morph::GridVisual");

    morph::VisualTextModel<>* fps_tm;
    v.addLabel ("0 FPS", {0.53f, -0.23f, 0.0f}, fps_tm); // With fps_tm can update the VisualTextModel with fps_tm->setupText("new text")
    morph::VisualTextModel<>* mode_tm;
    v.addLabel ("Unknown", {0.23f, -0.03f, 0.0f}, mode_tm); // With fps_tm can update the VisualTextModel with fps_tm->setupText("new text")

    // Create a grid to show in the scene
    constexpr unsigned int Nside = 400; // You can change this
    constexpr morph::vec<float, 2> grid_spacing = {0.01f, 0.01f};
    morph::Grid grid(Nside, Nside, grid_spacing);
    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(grid.n(), 0.0);

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the morph::Visual scene
    morph::vec<float, 3> offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = morph::GridVisMode::Triangles; // Choose [fastest-->] Triangles, Pixels, RectInterp or Columns [-->slowest]
    gv->setScalarData (&data);
    gv->cm.setType (morph::ColourMapType::Cork);
    gv->zScale.do_autoscale = false;
    gv->zScale.setParams (0, 0);
    gv->colourScale.do_autoscale = false;
    gv->colourScale.compute_scaling (-1, 1);
    gv->addLabel (std::string("GridVisMode::Triangles, cm: ") + gv->cm.getTypeStr(), morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.03f));
    gv->finalize();
    auto gvp = v.addVisualModel (gv);

    v.render();

    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    sc::duration ddata = sc::duration{0};
    sc::duration dreinit = sc::duration{0};
    sc::duration dreinitjc = sc::duration{0};

    unsigned int incrementer = 0;
    unsigned int fcount = 0;

    bool reinitJustColours = true;
    double jc_fps = 0.0;
    double full_fps = 0.0;
    while (!v.readyToFinish) {

        v.poll();

        if (incrementer %500 == 0) { // change colourmap
            if (gvp->cm.getType() == morph::ColourMapType::Cork) {
                gvp->cm.setType (morph::ColourMapType::Jet);
            } else {
                gvp->cm.setType (morph::ColourMapType::Cork);
            }
        }

        if (incrementer % 1000 == 0) { // switch modes
            reinitJustColours = reinitJustColours ? false : true;
            if (reinitJustColours) {
                dreinitjc = sc::duration{0};
            } else {
                dreinit = sc::duration{0};
            }
            ddata = sc::duration{0};
            fcount = 0;
            std::stringstream ss;
            std::string gvmode = "Pixels";
            if (gvp->gridVisMode == morph::GridVisMode::RectInterp) {
                gvmode = "RectInterp";
            } else if (gvp->gridVisMode == morph::GridVisMode::Triangles) {
                gvmode = "Triangles";
            } else if (gvp->gridVisMode == morph::GridVisMode::Columns) {
                gvmode = "Columns";
            }
            ss << "Calling " << (reinitJustColours ? "reinitColours()" : "full reinit()" ) << " for " << grid.n()
               << " Grid pixels in " << gvmode << " mode";
            mode_tm->setupText (ss.str());
        }

        // Change data.
        auto tsd = steady_clock::now();
        for (unsigned int ri=0; ri<grid.n(); ++ri) {
            auto coord = grid[ri];
            float length = (incrementer % 1000) * 0.01f;
            data[ri] = std::sin(length * coord[0]) * std::sin(0.5f * length * coord[1]) ; // Range 0->1
        }
        ddata += sc::now() - tsd;

        incrementer++;
        fcount++;

        tsd = sc::now();
        if (reinitJustColours) {
            gvp->reinitColours(); // Colour reinit is faster (3-4 times for a larger grid)
            dreinitjc += sc::now() - tsd;
        } else {
            gvp->reinit();          // A full rebuild runs slower
            dreinit += sc::now() - tsd;
        }

        if (duration_cast<milliseconds>(ddata).count() > 500 || incrementer % 1000 == 0) {
            // Update FPS text
            double data_tau = duration_cast<milliseconds>(ddata).count();
            double data_fps = std::round((((double)fcount/data_tau))*1000.0);
            if (reinitJustColours) {
                double reinit_tau_jc = duration_cast<milliseconds>(dreinitjc).count();
                jc_fps = std::round((((double)fcount/reinit_tau_jc))*1000.0);
            } else {
                double reinit_tau_full = duration_cast<milliseconds>(dreinit).count();
                full_fps = std::round((((double)fcount/reinit_tau_full))*1000.0);
            }
            std::stringstream ss;
            ss << "FPS: " << data_fps << " data : " << full_fps << " full reinit : " << jc_fps << " colour reinit";

            fps_tm->setupText (ss.str());
            ddata = sc::duration{0};

            if (reinitJustColours) {
                dreinitjc = sc::duration{0};
            } else {
                dreinit = sc::duration{0};
            }

            fcount = 0;
            v.waitevents (0.0001);
        }

        v.render();
    }

    return 0;
}
