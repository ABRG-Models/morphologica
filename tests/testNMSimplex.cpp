/*
 * Test Nelder Mead Simplex algorithm on the Rosenbrock banana function.
 */

#include "morph/NM_Simplex.h"
#include "morph/vvec.h"
#include <iostream>

// Here's the Rosenbrock banana function
FLT banana (FLT x, FLT y) {
    FLT a = 1.0;
    FLT b = 100.0;
    FLT rtn = ((a-x)*(a-x)) + (b * (y-(x*x)) * (y-(x*x)));
    return rtn;
}

int main()
{
    // Initialise the vertices
    morph::vvec<morph::vvec<FLT>> i_vertices;
    morph::vvec<FLT> v1 = {{ 0.7, 0.0 }};
    morph::vvec<FLT> v2 = {{ 0.0, 0.6 }};
    morph::vvec<FLT> v3 = {{ -0.6, -1.0 }};
    i_vertices.push_back(v1);
    i_vertices.push_back(v2);
    i_vertices.push_back(v3);

    // Check banana function
    FLT test = banana (1.0, 1.0);
    std::cout << "test point on banana function = " << test << " (should be 0)." << std::endl;

    morph::NM_Simplex<FLT> simp(i_vertices);

    // The smaller you make the threshold, the nearer the algo will get
    simp.termination_threshold = std::numeric_limits<FLT>::epsilon();

    // Temporary variable
    FLT val;

    // Now do the business
    unsigned int lcount = 0;
    while (simp.state != morph::NM_Simplex_State::ReadyToStop) {
        lcount++;
        if (simp.state == morph::NM_Simplex_State::NeedToComputeThenOrder) {
            // 1. apply objective to each vertex
            std::cout << "Recompute (did shrink):";
            for (unsigned int i = 0; i <= simp.n; ++i) {
                simp.values[i] = banana (simp.vertices[i][0], simp.vertices[i][1]);
                if (i > 0) { std::cout << ","; }
                std::cout << "(" << simp.vertices[i][0] << "," << simp.vertices[i][1] << ")";
            }
            std::cout << std::endl;
            simp.order();

        } else if (simp.state == morph::NM_Simplex_State::NeedToOrder) {
#if 0
            for (unsigned int i = 0; i <= simp.n; ++i) {
                std::cout << "Value for vertex (" << simp.vertices[i][0] << "," << simp.vertices[i][1]
                          << ") is: " << simp.values[i] << std::endl;
            }
            std::cout << "(2) Call order()" << std::endl;
#endif
            simp.order();

        } else if (simp.state == morph::NM_Simplex_State::NeedToComputeReflection) {
            val = banana (simp.xr[0], simp.xr[1]);
            std::cout << "Compute reflection, which is (" << simp.xr[0] << "," << simp.xr[1]
                      << ") with value " << val << std::endl;
            simp.apply_reflection (val);

        } else if (simp.state == morph::NM_Simplex_State::NeedToComputeExpansion) {
            val = banana (simp.xe[0], simp.xe[1]);
            std::cout << "Compute expansion, which is (" << simp.xe[0] << "," << simp.xe[1]
                      << ") with value " << val << std::endl;
            simp.apply_expansion (val);

        } else if (simp.state == morph::NM_Simplex_State::NeedToComputeContraction) {
            val = banana (simp.xc[0], simp.xc[1]);
            std::cout << "Compute contraction, which is (" << simp.xc[0] << "," << simp.xc[1]
                      << ") with value " << val << std::endl;
            simp.apply_contraction (val);
        }

#if 0
        // Output in matlab/octave format to plot3() the simplex.
        std::cout << "simp=[";
        for (unsigned int i = 0; i <= simp.n; ++i) {
            std::cout << simp.vertices[i][0] << "," << simp.vertices[i][1] << ","
                      << simp.values[i] << ";";
        }
        std::cout << simp.vertices[0][0] << "," << simp.vertices[0][1] << ","
                  << simp.values[0] << "];" << std::endl;
        std::cout << "order:" << simp.vertex_order[0] << ","<< simp.vertex_order[1]
                  << ","<< simp.vertex_order[2] << std::endl;
#endif
    }
    morph::vvec<FLT> thebest = simp.best_vertex();
    FLT bestval = simp.best_value();
    std::cout << "FINISHED! lcount=" << lcount
              << ". Best approximation: (" << thebest
              << ") has value " << bestval << std::endl;

    int rtn = -1;
    if (abs(thebest[0] - FLT{1}) < FLT{1e-3}
        && abs(thebest[1] - FLT{1}) < FLT{1e-3}) {
        std::cout << "Test success" << std::endl;
        rtn = 0;
    }

    return rtn;
}
