#include "MathAlgo2.h"
#include <iostream>

#include "Vector3.h"

using namespace morph;
using namespace std;

// Testing MathAlgo with vector/array types.

int main()
{
    int rtn = 0;

    vector<double> vf;
    vf.push_back (0.0);
    vf.push_back (0.1);
    vf.push_back (0.2);
    vf.push_back (1.0);
    vf.push_back (1.1);
    vf.push_back (1.2);
    cout << "double functions" << endl;
    pair<double, double> vfmm = MathAlgo::maxmin (vf);
    cout << "max/min: " << vfmm.first << "/" << vfmm.second << endl;

    // Throws runtime error, as expected; can't pass vector of scalars to the
    // MathAlgo::centroid function (instead, use MathAlgo::centroid3D)
    //
    //double cen = MathAlgo::centroid (vf);
    //cout << "centroid (" << cen << endl;

    array<double, 3> v1 = { 1.0f, 1.0f, 1.0f };
    array<double, 3> v2 = { 0.5f, 2.0f, 1.0f };
    array<double, 3> v3 = { 1.0f, 1.0f, 2.1f };
    vector< array<double, 3> > vv3;
    vv3.push_back (v1);
    vv3.push_back (v2);
    vv3.push_back (v3);
    cout << "array<double,3> functions" << endl;
    pair<array<double,3>, array<double,3> > vv3mm = MathAlgo::maxmin (vv3);
    cout << "max/min: (" << vv3mm.first[0] << ","
         << vv3mm.first[1] << "," << vv3mm.first[2] << ")/(" << vv3mm.second[0] << ","
         << vv3mm.second[1] << "," << vv3mm.second[2] << ")" << endl;

    array<double,3> cen = MathAlgo::centroid (vv3);
    cout << "centroid (" << cen[0] << "," << cen[1] << "," << cen[2] << ")" << endl;

    cout << "Try vector of vectors" << endl;
    vector<float> vf1 = { 1.0f, 1.0f, 1.0f };
    vector<float> vf2 = { 2.0f, 2.0f, 3.0f };
    vector<float> vf3 = { 3.0f, -1.0f, 5.0f };
    vector< vector<float> > vvf;
    vvf.push_back (vf1);
    vvf.push_back (vf2);
    vvf.push_back (vf3);
    cout << "vector<float> functions" << endl;
    pair<vector<float>, vector<float> > vvfmm = MathAlgo::maxmin (vvf);
    cout << "max: (";
    for (auto i : vvfmm.first) {
        cout << i << " ";
    }
    cout << ")\n";
    cout << "min: (";
    for (auto i : vvfmm.second) {
        cout << i << " ";
    }
    cout << ")\n";

    vector<float> cen2 = MathAlgo::centroid (vvf);
    cout << "centroid (" << cen2[0] << "," << cen2[1] << "," << cen2[2] << ")" << endl;

    // Not currently possible:
    //vector<Vector3<float>> vVec3 (4, Vector3<float>(0,0,0));
    //Vector3<float> cen3 = MathAlgo::centroid (vVec3);

    list<int> li;
    li.push_back(2);
    li.push_back(1);
    li.push_back(7);
    pair<int,int> limm = MathAlgo::maxmin (li);
    cout << "max/min: " << limm.first << "," << limm.second << endl;
    return rtn;
}
