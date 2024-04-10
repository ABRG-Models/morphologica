/*
 * Test use of vec<F, N> objects in a Scale class.
 */

#include <vector>
using std::vector;
#include <list>
using std::list;
#include <array>
using std::array;
#include <iostream>
using std::cout;
using std::endl;
#include "morph/vec.h"
using morph::vec;
#include "morph/Scale.h"
using morph::Scale;

int main () {

    int rtn = 0;

    Scale<vec<float,4>> s2;
    s2.do_autoscale = true;
    vector<vec<float,4>> vaf;
    vaf.push_back ({1,1,2,1});
    vaf.push_back ({2,2,2,3});
    vaf.push_back ({3,3,4,1});
    vaf.push_back ({4,4,4,4});
    vector<vec<float,4>> result2(vaf);
    s2.transform (vaf, result2);

    cout << "vector<vec<float,4>> unscaled/scaled vectors:\n";
    for (unsigned int i = 0; i < result2.size(); ++i) {

        cout << "(";
        for (auto v : vaf[i]) {
            cout << v << ",";
        }
        cout << ")   ";

        cout << "(";
        for (auto v : result2[i]) {
            cout << v << ",";
        }
        cout << ")\n";
    }

    cout << "Stream Scale<vec<float,4>>: " << s2 << endl;

    // Test this scaling:
    vector<vec<float, 4>>::const_iterator r2i = result2.end();
    r2i--; // To get to last element in vector
    float r2ilen = std::sqrt ((*r2i)[0] * (*r2i)[0] + (*r2i)[1] * (*r2i)[1] + (*r2i)[2] * (*r2i)[2] + (*r2i)[3] * (*r2i)[3]);
    if (abs(r2ilen - 1) > 0.0001) {
        cout << "Error" << endl;
        rtn--;
    }

    return rtn;
}
