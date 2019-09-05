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
    FLT rtn = (a-x)*(a-x) + b * (y-(x*x)) * (y-(x*x));
    return rtn;
}

int main()
{
    int rtn = 0;

    // Initialise the vertices
    vector<vector<FLT>> i_vertices;
    vector<FLT> v1 = {{ 0.0, 0.0 }};
    vector<FLT> v2 = {{ 0.0, 1.0 }};
    vector<FLT> v3 = {{ -1.0, 1.0 }};
    i_vertices.push_back(v1);
    i_vertices.push_back(v2);
    i_vertices.push_back(v3);

    NM_Simplex<FLT> simp(i_vertices);

    // Temporary variable
    FLT val;

    // Now do the business
    unsigned int lcount = 0;
    while (simp.state != NM_Simplex_State::ReadyToStop) {
        lcount++;
        if (simp.state == NM_Simplex_State::NeedToComputeThenOrder) {
            // 1. apply objective to each vertex
            cout << "Compute vertices" << endl;
            for (unsigned int i = 0; i <= simp.n; ++i) {
                simp.values[i] = banana (simp.vertices[i][0], simp.vertices[i][1]);
                cout << "Value for vertex (" << simp.vertices[i][0] << "," << simp.vertices[i][1]
                     << ") is: " << simp.values[i] << endl;
            }
            cout << "(1) Call order()" << endl;
            simp.order();

        } else if (simp.state == NM_Simplex_State::NeedToOrder) {
            for (unsigned int i = 0; i <= simp.n; ++i) {
                cout << "Value for vertex (" << simp.vertices[i][0] << "," << simp.vertices[i][1]
                     << " is: " << simp.values[i] << endl;
            }
            cout << "(2) Call order()" << endl;
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
            //usleep (1000000);
    }
    cout << "FINISHED! lcount=" << lcount << endl;

    return rtn;
}
