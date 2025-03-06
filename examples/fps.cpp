/*
 * An example morph::Visual scene, containing a HexGrid.
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <sstream>

#include <morph/vec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/VisualTextModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

int main()
{
    morph::Visual v(1600, 1000, "morph::Visual");
    v.fov = 15.0f;
    v.zFar = 200.0f;
    v.lightingEffects();
    morph::VisualTextModel<>* fps_tm;
    v.addLabel ("0 FPS", {0.13f, -0.23f, 0.0f}, fps_tm); // With fps_tm can update the VisualTextModel with fps_tm->setupText("new text")

    // Create a HexGrid to show in the scene
    constexpr float hex_to_hex = 0.02f;

    morph::HexGrid hg(hex_to_hex, 15.0f, 0.0f);
    hg.setEllipticalBoundary (4.0f, 4.0f);
    std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
    std::stringstream sss;
    sss << "Surface evaluated at " << hg.num() << " coordinates";
    v.addLabel (sss.str(), {0.0f, 0.0f, 0.0f});

    // Make some dummy data (a radially symmetric Bessel fn) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0f);
    std::vector<float> r(hg.num(), 0.0f);
    float k = 1.0f;
    for (unsigned int hi = 0; hi < hg.num(); ++hi) {
        // x/y: hg.d_x[hi] hg.d_y[hi]
        r[hi] = std::sqrt (hg.d_x[hi] * hg.d_x[hi] + hg.d_y[hi] * hg.d_y[hi]);
        data[hi] = std::sin (k * r[hi]) / k * r[hi];
    }

    // Add a HexGridVisual to display the HexGrid within the morph::Visual scene
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(&hg, offset);
    v.bindmodel (hgv);
    hgv->setScalarData (&data);
    hgv->hexVisMode = morph::HexVisMode::Triangles;
    hgv->finalize();
    auto hgvp = v.addVisualModel (hgv);

    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    auto t00 = sc::now();
    auto t0 = sc::now();
    auto t1 = sc::now();
    auto t2 = sc::now();
    auto data_dur = sc::duration{0};
    auto update_dur = sc::duration{0};
    auto all_dur = sc::duration{0};
    double data_fps = 0.0;
    double update_fps = 0.0;
    double rest_fps = 0.0;
    double all_fps = 0.0;
    unsigned int fcount = 0u;

    while (v.readyToFinish == false) {
        all_dur += sc::now() - t00;
        t00 = sc::now();

        v.waitevents (0.00001);
        if (k > 8.0f) { k = 1.0f; }

        t0 = sc::now();
#pragma omp parallel for shared(r,k,data)
        for (unsigned int hi = 0; hi < hg.num(); ++hi) {
            data[hi] = std::sin (k * r[hi]) / k * r[hi];
        }
        t1 = sc::now();
        data_dur += (t1 - t0);

        if (v.validVisualModel (hgvp) != nullptr) { // Test hgvp is still valid
            hgvp->updateData (&data);
        }
        t2 = sc::now();
        update_dur += (t2 - t1);
        k += 0.02f;

        if (fcount == 500) {
            // Update FPS text
            double data_tau = duration_cast<milliseconds>(data_dur).count();
            double update_tau = duration_cast<milliseconds>(update_dur).count();
            double all_tau = duration_cast<milliseconds>(all_dur).count();
            double rest_tau = all_tau - data_tau - update_tau;
            data_fps = std::max (data_fps, std::round((((double)fcount/data_tau))*1000.0));
            update_fps = std::max (update_fps, std::round((((double)fcount/update_tau))*1000.0));
            all_fps = std::max (all_fps, std::round((((double)fcount/all_tau))*1000.0));
            rest_fps = std::max (rest_fps, std::round((((double)fcount/rest_tau))*1000.0));
            std::stringstream ss;
            ss << "FPS: " << data_fps << " [dat] " << update_fps << " [upd] " << rest_fps << " [rest] " << all_fps << " [all]\n";
            fps_tm->setupText (ss.str());
            data_dur = sc::duration{0};
            update_dur = sc::duration{0};
            all_dur = sc::duration{0};
            fcount = 0;
        }

        v.render();
        fcount++;
    }

    return 0;
}
