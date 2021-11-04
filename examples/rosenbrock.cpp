/*
 * Test Nelder Mead Simplex algorithm on the Rosenbrock banana function.
 */

#include <morph/NM_Simplex.h>
#include <morph/Vector.h>
#include <morph/Visual.h>
#include <morph/TriFrameVisual.h>
#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>
#include <morph/MathAlgo.h>
#include <iostream>
#include <chrono>

// Here's the Rosenbrock banana function
FLT banana (FLT x, FLT y) {
    FLT a = 1.0;
    FLT b = 100.0;
    FLT rtn = ((a-x)*(a-x)) + (b * (y-(x*x)) * (y-(x*x)));
    return rtn;
}

int main()
{
    using std::vector;
    using std::cout;
    using std::endl;

    // Set up a visual environment
    morph::Visual v(2600, 1800, "Rosenbrock bananas", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.01f);
    v.zNear = 0.001;
    v.zFar = 100000;
    v.fov=60;
    v.showCoordArrows = true;
    v.lightingEffects (true);

    // Initialise the vertices
    morph::vVector<morph::vVector<FLT>> i_vertices;
    morph::vVector<FLT> v1 = {{ 0.7, 0.0 }};
    morph::vVector<FLT> v2 = {{ 0.0, 0.6 }};
    morph::vVector<FLT> v3 = {{ -0.6, -1.0 }};
    i_vertices.push_back(v1);
    i_vertices.push_back(v2);
    i_vertices.push_back(v3);

    // Add a 'triangle visual' to be visualised as three rods
    morph::Vector<float> _offset = {0,0,0};
    morph::TriFrameVisual<FLT>* tfv = new morph::TriFrameVisual<FLT>(v.shaderprog, _offset);
    tfv->radius = 0.01f;
    tfv->sradius = 0.01f;
    vector<FLT> tri_values(3, 0);
    vector<morph::Vector<float>> tri_coords(3);
    tri_coords[0] = { v1[0], v1[1], 0.0 };
    tri_coords[1] = { v2[0], v2[1], 0.0 };
    tri_coords[2] = { v3[0], v3[1], 0.0 };
    tfv->setScalarData (&tri_values);
    tfv->setDataCoords (&tri_coords);
    tfv->cm.setType (morph::ColourMapType::Cividis);
    tfv->finalize();
    v.addVisualModel (tfv);

    // Check banana function
    FLT test = banana (1.0, 1.0);
    std::cout << "test point on banana function = " << test << " (should be 0).\n";

    // Evaluate banana function and plot
    morph::HexGrid hg (0.01, 10, 0, morph::HexDomainShape::Boundary);
    hg.setCircularBoundary (2.5);
    vector<FLT> banana_vals(hg.num(), 0.0f);
    for (size_t i = 0; i < hg.num(); ++i) {
        banana_vals[i] = banana (hg.d_x[i], hg.d_y[i]);
    }
    std::pair<FLT, FLT> mm = morph::MathAlgo::maxmin(banana_vals);
    std::cout << "Banana surface max/min: " << mm.first << "," << mm.second << std::endl;
    morph::HexGridVisual<FLT>* hgv = new morph::HexGridVisual<FLT>(v.shaderprog, v.tshaderprog, &hg, _offset);
    hgv->hexVisMode = morph::HexVisMode::Triangles;
    hgv->cm.setType (morph::ColourMapType::Viridis);
    hgv->setScalarData (&banana_vals);
    hgv->zScale.setParams (0.001f, 0.0f);
    hgv->colourScale.compute_autoscale (0.01f, 5.0f);
    hgv->setAlpha (0.4f);
    hgv->finalize();
    v.addVisualModel (hgv);

    morph::NM_Simplex<FLT> simp(i_vertices);

    // The smaller you make the threshold, the nearer the algo will get
    simp.termination_threshold = std::numeric_limits<FLT>::epsilon();

    // Temporary variable
    FLT val;

    // if using plotting, then set up the render clock
    std::chrono::steady_clock::time_point lastrender = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastoptstep = std::chrono::steady_clock::now();

    // Now do the business
    unsigned int lcount = 0;
    while (simp.state != morph::NM_Simplex_State::ReadyToStop) {

        // Perform optimisation steps slowly
        std::chrono::steady_clock::duration sinceoptstep = std::chrono::steady_clock::now() - lastoptstep;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(sinceoptstep).count() > 250) {
            lcount++;
            if (simp.state == morph::NM_Simplex_State::NeedToComputeThenOrder) {
                // 1. apply objective to each vertex
                for (unsigned int i = 0; i <= simp.n; ++i) {
                    simp.values[i] = banana (simp.vertices[i][0], simp.vertices[i][1]);
                }
                simp.order();

            } else if (simp.state == morph::NM_Simplex_State::NeedToOrder) {
                simp.order();

            } else if (simp.state == morph::NM_Simplex_State::NeedToComputeReflection) {
                val = banana (simp.xr[0], simp.xr[1]);
                simp.apply_reflection (val);

            } else if (simp.state == morph::NM_Simplex_State::NeedToComputeExpansion) {
                val = banana (simp.xe[0], simp.xe[1]);
                simp.apply_expansion (val);

            } else if (simp.state == morph::NM_Simplex_State::NeedToComputeContraction) {
                val = banana (simp.xc[0], simp.xc[1]);
                simp.apply_contraction (val);
            }

#if 0
            // Output in matlab/octave format to plot3() the simplex.
            cout << "simp=[";
            for (unsigned int i = 0; i <= simp.n; ++i) {
                cout << simp.vertices[i][0] << "," << simp.vertices[i][1] << ",val" << simp.values[i] << ";";
            }
            cout << simp.vertices[0][0] << "," << simp.vertices[0][1] << ",val" << simp.values[0] << "];" << endl;
            //cout << "order:" << simp.vertex_order[0] << ","<< simp.vertex_order[1] << ","<< simp.vertex_order[2] << endl;
#endif

            // Visualise the triangle defined by simp.vertices

            // Copy data out from NM_Simplex
            for (unsigned int i = 0; i <= simp.n; ++i) {
                tri_coords[i] = { simp.vertices[i][0], simp.vertices[i][1], 0.0 };
                tri_values[i] = simp.values[i];
            }
            tfv->reinit();

            lastoptstep = std::chrono::steady_clock::now();
        }

        std::chrono::steady_clock::duration sincerender = std::chrono::steady_clock::now() - lastrender;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(sincerender).count() > 17) { // 17 is about 60 Hz
            glfwPollEvents();
            v.render();
            lastrender = std::chrono::steady_clock::now();
        }
    }
    vector<FLT> thebest = simp.best_vertex();
    FLT bestval = simp.best_value();
    cout << "FINISHED! lcount=" << lcount
         << ". Best approximation: (" << thebest[0] << "," << thebest[1]
         << ") has value " << bestval << endl;

    int rtn = -1;
    if (abs(thebest[0] - 1.0) < 1e-3 // Choose 1e-3 so that this will succeed with floats or doubles
        && abs(thebest[1] - 1.0) < 1e-3) {
        cout << "Test success" << endl;
        rtn = 0;
    }

    v.keepOpen();

    return rtn;
}
