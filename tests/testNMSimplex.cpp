/*
 * Test Nelder Mead Simplex algorithm on the Rosenbrock banana function.
 */

#include "NM_Simplex.h"
#include <iostream>
#include <unistd.h>

using namespace morph;
using namespace std;

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
    vector<vector<FLT>> i_vertices;
    vector<FLT> v1 = {{ 0.7, 0.0 }};
    vector<FLT> v2 = {{ 0.0, 0.6 }};
    vector<FLT> v3 = {{ -0.6, -1.0 }};
    i_vertices.push_back(v1);
    i_vertices.push_back(v2);
    i_vertices.push_back(v3);

    // Check banana function
    FLT test = banana (1.0, 1.0);
    cout << "test point on banana function = " << test << endl;

    NM_Simplex<FLT> simp(i_vertices);

    // The smaller you make the threshold, the nearer the algo will get
    simp.termination_threshold = numeric_limits<FLT>::epsilon();

    // Temporary variable
    FLT val;

    // Now do the business
    unsigned int lcount = 0;
    while (simp.state != NM_Simplex_State::ReadyToStop) {
        lcount++;
        if (simp.state == NM_Simplex_State::NeedToComputeThenOrder) {
            // 1. apply objective to each vertex
            cout << "Recompute (did shrink):";
            for (unsigned int i = 0; i <= simp.n; ++i) {
                simp.values[i] = banana (simp.vertices[i][0], simp.vertices[i][1]);
                if (i > 0) { cout << ","; }
                cout << "(" << simp.vertices[i][0] << "," << simp.vertices[i][1] << ")";
            }
            cout << endl;
            simp.order();

        } else if (simp.state == NM_Simplex_State::NeedToOrder) {
#if 0
            for (unsigned int i = 0; i <= simp.n; ++i) {
                cout << "Value for vertex (" << simp.vertices[i][0] << "," << simp.vertices[i][1]
                     << ") is: " << simp.values[i] << endl;
            }
            cout << "(2) Call order()" << endl;
#endif
            simp.order();

        } else if (simp.state == NM_Simplex_State::NeedToComputeReflection) {
            val = banana (simp.xr[0], simp.xr[1]);
            cout << "Compute reflection, which is (" << simp.xr[0] << "," << simp.xr[1] << ") with value " << val << endl;
            simp.apply_reflection (val);

        } else if (simp.state == NM_Simplex_State::NeedToComputeExpansion) {
            val = banana (simp.xe[0], simp.xe[1]);
            cout << "Compute expansion, which is (" << simp.xe[0] << "," << simp.xe[1] << ") with value " << val << endl;
            simp.apply_expansion (val);

        } else if (simp.state == NM_Simplex_State::NeedToComputeContraction) {
            val = banana (simp.xc[0], simp.xc[1]);
            cout << "Compute contraction, which is (" << simp.xc[0] << "," << simp.xc[1] << ") with value " << val << endl;
            simp.apply_contraction (val);
        }

#if 0
        cout << "simp=[";
        for (unsigned int i = 0; i <= simp.n; ++i) {
            cout << simp.vertices[i][0] << "," << simp.vertices[i][1] << "," << simp.values[i] << ";";
        }
        cout << simp.vertices[0][0] << "," << simp.vertices[0][1] << "," << simp.values[0] << "];" << endl;
        cout << "order:" << simp.vertex_order[0] << ","<< simp.vertex_order[1] << ","<< simp.vertex_order[2] << endl;
#endif
    }
    vector<FLT> thebest = simp.best_vertex();
    FLT bestval = simp.best_value();
    cout << "FINISHED! lcount=" << lcount << ". Best approximation: (" << thebest[0] << "," << thebest[1]<< ") has value " << bestval << endl;

    int rtn = -1;
    if (abs(thebest[0] - 1.0) < 1e-3 // Choose 1e-3 so that this will succeed with floats or doubles
        && abs(thebest[1] - 1.0) < 1e-3) {
        cout << "Test success" << endl;
        rtn = 0;
    }

    return rtn;
}
