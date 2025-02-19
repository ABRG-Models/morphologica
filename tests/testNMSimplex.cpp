/*
 * Test Nelder Mead Simplex algorithm on the Rosenbrock banana function
 */

#include "morph/NM_Simplex.h"
#include "morph/vvec.h"
#include <iostream>
#include <cmath>

int main()
{
    // Initialise the vertices
    morph::vvec<morph::vvec<FLT>> i_vertices = {
        { 0.7, 0.0 },
        { 0.0, 0.6 },
        { -0.6, -1.0 }
    };
    // Create an optimiser object
    morph::NM_Simplex<FLT> simp(i_vertices);
    // Define your Rosenbrock Banana objective function
    auto banana = [](const morph::vvec<FLT>& point) {
        FLT x = point[0];
        FLT y = point[1];
        constexpr FLT a = FLT{1};
        constexpr FLT b = FLT{100};
        FLT rtn = ((a-x)*(a-x)) + (b * (y-(x*x)) * (y-(x*x)));
        return rtn;
    };
    simp.objective = banana;
    // Set an optimization parameter
    simp.termination_threshold = std::numeric_limits<FLT>::epsilon();
    // Run the optimization
    if (!simp.run()) { std::cerr << "Objective was not set\n"; return -1; }
    // Check the final state which we expect to be 'TerminationThreshold':
    if (simp.stopreason != morph::NM_Simplex_Stop_Reason::TerminationThreshold) {
        if (simp.stopreason == morph::NM_Simplex_Stop_Reason::TooManyOperations) {
            std::cerr << "The optimization stopped after TooManyOperations (" << simp.too_many_operations << ")\n";
        } else {
            std::cerr << "The optimization stopped for an unknown reason\n";
        }
        return -1;
    }
    // Test our optimization end point
    morph::vvec<FLT> thebest = simp.best_vertex();
    std::cout << "FINISHED! Best approximation: (" << thebest << ") has value " << simp.best_value() << std::endl;
    if (std::abs(thebest[0] - FLT{1}) < FLT{1e-3} && std::abs(thebest[1] - FLT{1}) < FLT{1e-3}) {
        std::cout << "Test success" << std::endl;
        return 0;
    }
    std::cout << "Test failure" << std::endl;
    return -1;
}
